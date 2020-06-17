#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/sysinfo.h>

#include "macros.h"
#include "arguments.h"
#include "pbab.h"
#include "solution.h"
#include "log.h"

#include "tree_controller.h"
#include "tree.h"

#include "bbthread.h"


TreeController::TreeController(pbab * _pbb)
{
    pbb = _pbb;

    M = (arguments::nbivms_mc < 1) ? get_nprocs_conf() : arguments::nbivms_mc;

    pthread_barrier_init(&barrier, NULL, M);

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);

    pthread_mutex_init(&mutex_list, &attr);
    for (int i = 0; i < arguments::nbivms_mc; i++) {
        victim_list.push_back(i);
    }

    // explorer_last = 0;
    // compteur = 0;
    pthread_mutex_init(&compteur_mutex, &attr);

    srand(time(NULL));

    for (unsigned int i = 0; i < M; i++) {
        sbb[i] = new bbthread(i, pbb);
        if (i == 0) sbb[i]->setRoot(pbb->sltn->perm);
    }

    atom_nb_explorers.store(0);// id_generator
    end_counter.store(0);// termination_counter
    allEnd.store(false);// termination_counter
}

void
TreeController::explore()
{
    bool continuer = true;
    int id         = explorer_get_new_id();

    int res = pthread_barrier_wait(&barrier);

    if (res == PTHREAD_BARRIER_SERIAL_THREAD) std::cout << " === all threads start exploration...\n";
    else if (res != 0) std::cout << "error.\n";

    while (1) {
        if (!continuer && !allEnd.load()) {
            demander(id);
            // request_work(id);
            // if (allEnd.load()) break;
        } else  {
            sbb[id]->set_workState(continuer);
            try_repondre(id);
            // try_answer_request(id);
        }

        if (allEnd.load()) { // interrupted from outside or no more work reached
            break;
        }
        continuer = next(id);
    }

    allEnd.store(true);

    stop(id);
}

void *
tree_thread(void * _tc)
{
    TreeController * tc = (TreeController *) _tc;

    tc->explore();

    return NULL;
}

void
TreeController::next()
{
    pthread_t * threads = new pthread_t[M];

    for (unsigned int i = 0; i < M; i++)
        pthread_create(&threads[i], NULL, tree_thread, (void *) this);

    for (unsigned int i = 0; i < M; i++)
        pthread_join(threads[i], NULL);

    delete[]threads;
}

int
TreeController::explorer_get_new_id()
{
    if (atom_nb_explorers.load() >= MAX_EXPLORERS) {
        printf("Erreur : Nombre maximal de threads\n");
        exit(2);
    }
    return (atom_nb_explorers++);
}

bool
TreeController::next(const int id)
{
    bool res = sbb[id]->next();

    return res;
}

// ============================================================================================
// ================================================= gestion terminaison========================
// =============================================================================================
void
TreeController::unlockWaiting(int id)
{
    FILE_LOG(logDEBUG1) << "Unlock all waiting";

    // unlock threads waiting for work
    for (int i = 0; i < M; i++) {
        if (i == id) continue;
        pthread_mutex_lock_check(&sbb[i]->mutex_sharedWrk);
        sbb[i]->waiting=false;
        pthread_cond_signal(&sbb[i]->cond_sharedWrk);
        pthread_mutex_unlock(&sbb[i]->mutex_sharedWrk);
        // sem_post(&sbb[i]->sema_wait);
    }
}

void
TreeController::stop(int id)
{
    unlockWaiting(id);

    int ret=pthread_barrier_wait(&barrier);
    if(ret==PTHREAD_BARRIER_SERIAL_THREAD)
        FILE_LOG(logDEBUG1) << "=== stop "<<M<<" ===";

    // sem_destroy(&sbb[id]->sema_wait);
}


bool
TreeController::counter_increment(int id)
{
    pthread_mutex_lock_check(&compteur_mutex);
    if ((++end_counter) == M) {
        allEnd.store(true);
        pthread_mutex_unlock(&compteur_mutex);
        return true;
    }
    pthread_mutex_unlock(&compteur_mutex);
    return false;
}

// _____________________________________________________________________________________________
void
TreeController::counter_decrement()
{
    pthread_mutex_lock(&compteur_mutex);
    end_counter--;
    pthread_mutex_unlock(&compteur_mutex);
}

// _____________________________________________________________________________________________
int
TreeController::choisir_thread(int id)
{
    int retour;

    switch (arguments::mc_ws_select) {
        case 'r': retour = chooseNeighbour(id);
            break;
        case 'a': retour = chooseRandom(id);
            break;
        // case 'l': retour = chooseLargest(id);
        //     break;
        case 'o': retour = chooseOldest(id);
            break;
        default: std::cout << "wrong strategy\n";
            exit(0);
            break;
    }

    return retour;
}

// _____________________________________________________________________________________________
int
TreeController::chooseNeighbour(int id)
{
    return (id) ? (id - 1) : (M - 1);
}

// _____________________________________________________________________________________________
int
TreeController::chooseRandom(int id)
{
    int victim = id;

    do {
        victim = rand() % M;
    } while (victim == id || !sbb[victim]->hasWork);

    return victim;
}

// _____________________________________________________________________________________________
int
TreeController::chooseOldest(int id)
{
    int victim = 0;

    pthread_mutex_lock(&mutex_list);
    victim_list.remove(id);
    victim_list.push_back(id);
    victim = victim_list.front();
    pthread_mutex_unlock(&mutex_list);
    return victim;
}

// =============================================================================================

void
TreeController::demander(int id)
{
    sbb[id]->set_workState(false);
    if (counter_increment(id)) return;

    // if any pending requests, release waiting threads
    while (sbb[id]->has_request()) {
        int thief = pull_request(id);

        if(thief==id){
            FILE_LOG(logERROR) << id << " try cancel myself " << thief;
            exit(-1);
        }
        // std::cout << id << " cancel" << std::endl << std::flush;
        counter_decrement();
        FILE_LOG(logDEBUG) << id << " cancel " << thief << " count: "<<end_counter;

        pthread_mutex_lock_check(&sbb[thief]->mutex_sharedWrk);
        sbb[thief]->waiting=false;
        pthread_cond_signal(&sbb[thief]->cond_sharedWrk);
        pthread_mutex_unlock(&sbb[thief]->mutex_sharedWrk);
    }

    int victim = choisir_thread(id);// select victim

    if (victim != id) {
        push_request(victim, id);

        //wait until condition signalled
        pthread_mutex_lock_check(&sbb[id]->mutex_sharedWrk);
        sbb[id]->waiting=true;
        pthread_mutex_unlock(&sbb[id]->mutex_sharedWrk);

        pthread_mutex_lock_check(&sbb[id]->mutex_sharedWrk);
        while (sbb[id]->waiting && !allEnd.load()) {
            pthread_cond_wait(&sbb[id]->cond_sharedWrk, &sbb[id]->mutex_sharedWrk);
        }
        pthread_mutex_unlock(&sbb[id]->mutex_sharedWrk);
    } else  {
             // std::cout << "autoselect " << id << " wait " << victim << std::endl << std::flush;
        counter_decrement();
    }

    //  push_request(victim, id);

    sbb[id]->hasWork = !sbb[id]->empty();
    //  hasWork[id]=true;
}

// _____________________________________________________________________________________________
void
TreeController::try_repondre(int id)
{
    if (!sbb[id]->has_request()) return;  // if no request return

    // while (!(sbb[id]->requestQueue).empty()) {// try answer all requests
        int thief = pull_request(id);

        int stolen = work_share(id, thief);

        counter_decrement();

        pthread_mutex_lock_check(&sbb[thief]->mutex_sharedWrk);
        sbb[thief]->waiting = false;
        pthread_cond_signal(&sbb[thief]->cond_sharedWrk);
        pthread_mutex_unlock(&sbb[thief]->mutex_sharedWrk);
    // }
}

int
TreeController::work_share(int id, int dest)
{
    if (sbb[id]->size() < 2) return 0;

    int howMany = 0;

    // howMany=1; //steal-one
    howMany = (sbb[id]->size()) / 2; // steal-half
    for (int i = 0; i < howMany; i++) {
        // std::cout<<id<<" Give "<<*sbb[id]->top_share()<<std::endl;
        sbb[dest]->push(sbb[id]->top_share()); // steal from tail, push at head
        sbb[id]->pop_share();
    }
//
    // std::cout<<"poolsizes "<<sbb[id]->size()<<" "<<sbb[id]->size()<<std::endl;


    return howMany;
}

// requests...
void
TreeController::set_request(int i, bool req)
{
    sbb[i]->rqst = req; // rqst is std::atomic_bool
}

void
TreeController::push_request(int victim, int id)
{
    set_request(victim, true);

    pthread_mutex_lock_check(&sbb[victim]->workRequested_mutex);
    (sbb[victim]->requestQueue).push_back(id); // push ID into requestQueue of victim thread
    pthread_mutex_unlock(&sbb[victim]->workRequested_mutex);
}

int
TreeController::pull_request(int id)
{
    int thief;

    pthread_mutex_lock_check(&sbb[id]->workRequested_mutex);
    thief = (sbb[id]->requestQueue).front();
    (sbb[id]->requestQueue).pop_front();
    pthread_mutex_unlock(&sbb[id]->workRequested_mutex);
    return thief;
}
