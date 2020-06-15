/*
 * "global" part of multi-core b&b
 * - work stealing
 * - termination detection (local)
 * - best (in pbb->sltn)
 */
#include <sys/sysinfo.h>
#include <unistd.h>

#include <memory>

#include "../../common/include/arguments.h"
#include "../../common/include/pbab.h"
#include "../../common/include/solution.h"
#include "../../common/include/ttime.h"
#include "../../common/include/macros.h"
#include "../../common/include/log.h"

#include "../include/sequentialbb.h"
#include "../include/bbthread.h"
#include "../include/matrix_controller.h"


matrix_controller::matrix_controller(pbab * _pbb)
{
    pbb  = _pbb;
    size = pbb->size;

    // printf("nbivms_mc %d\n",nbivm_mc);
    M = (arguments::nbivms_mc < 1) ? get_nprocs_conf() : arguments::nbivms_mc;

    pthread_barrier_init(&barrier, NULL, M);

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);

    pthread_mutex_init(&mutex_steal_list, &attr);
    pthread_mutex_init(&mutex_end, &attr);

    for (int i = 0; i < (int) M; i++)
        victim_list.push_back(i);

    allocate();
    resetExplorationState();
}

matrix_controller::~matrix_controller()
{
    pthread_mutex_destroy(&mutex_steal_list);
    pthread_mutex_destroy(&mutex_end);

    for (int i = 0; i < (int) M; i++) {
        delete sbb[i];
    }
}

void
matrix_controller::allocate()
{
    if (M < 1 || M > MAX_EXPLORERS) exit(-1);

    for (int i = 0; i < (int) M; i++) {
        sbb[i] = new bbthread(pbb);
    }
}


void
matrix_controller::initFullInterval()
{
    ivm_bound::first=true;
    for (int k = 0; k < M; k++) {
        sbb[k]->clear();
        sbb[k]->set_workState(false);
        // sbb[k]->setRoot();
    }

    int * zeroFact = (int *) malloc(size * sizeof(int));
    int * endFact  = (int *) malloc(size * sizeof(int));

    for (int i = 0; i < size; i++) {
        zeroFact[i] = 0;
        endFact[i]  = size - i - 1;
    }

    sbb[0]->setRoot(pbb->root_sltn->bestpermut);
    sbb[0]->initAtInterval(zeroFact, endFact);
    sbb[0]->set_workState(true);

    victim_list.remove(0);
    victim_list.push_front(0);// put in front

    FILE_LOG(logINFO) << "WS list ";
    for(auto i:victim_list)
    {
        FILE_LOG(logINFO) << i;
    }
    FILE_LOG(logINFO) << "MC initialized";

    free(zeroFact);
    free(endFact);
}

bool
matrix_controller::solvedAtRoot()
{
    return sbb[0]->solvedAtRoot();
}

//nbint := number received intervals
void
matrix_controller::initFromFac(const int nbint, const int * ids, int * pos, int * end)
{
    if (nbint > M) {
        printf("cannot handle more than %d intervals\n", M);
        exit(-1);
    }
    // printf("init from fac %d\n",nbint);fflush(stdout);

    for (int k = 0; k < M; k++) {
        pthread_mutex_lock_check(&sbb[k]->mutex_ivm);
        sbb[k]->clear();//zero IVM, intreval..
        pthread_mutex_unlock(&sbb[k]->mutex_ivm);
    }

    for (int k = 0; k < nbint; k++) {
        pthread_mutex_lock_check(&sbb[k]->mutex_ivm);
        int id = ids[k];

        if (id >= M) {
            printf("ID > nbIVMs!");
            exit(-1);
        }

        victim_list.remove(id);
        victim_list.push_front(id);// put in front

        // initialize at interval [begin,end]
        sbb[id]->setRoot(pbb->root_sltn->bestpermut);
        sbb[id]->initAtInterval(pos + k * size, end + k * size);
        pthread_mutex_unlock(&sbb[k]->mutex_ivm);
    }
}

void
matrix_controller::getIntervals(int * pos, int * end, int * ids, int &nb_intervals, const int  max_intervals)
{
    memset(pos, 0, max_intervals * size * sizeof(int));
    memset(end, 0, max_intervals * size * sizeof(int));

    if (max_intervals < M) {
        FILE_LOG(logERROR)<<"MC:buffer too small";
        exit(-1);
    }

    int nbActive = 0;
    for (int k = 0; k < M; k++) {
        pthread_mutex_lock_check(&sbb[k]->mutex_ivm);//don't need it...
        if (sbb[k]->IVM->beforeEnd()) {
            ids[nbActive] = k;
            memcpy(&pos[nbActive * size],sbb[k]->IVM->posVect, size*sizeof(int));
            memcpy(&end[nbActive * size],sbb[k]->IVM->endVect, size*sizeof(int));
            nbActive++;
        }
        pthread_mutex_unlock(&sbb[k]->mutex_ivm);
    }

    nb_intervals = nbActive;
}

int
matrix_controller::getNbIVM()
{
    return M;
}

int
matrix_controller::explorer_get_new_id()
{
    if (atom_nb_explorers.load() >= MAX_EXPLORERS) {
        printf("Erreur : Nombre maximal de threads\n");
        exit(2);
    }
    return (atom_nb_explorers++);
}


// ===================Termination
bool
matrix_controller::counter_increment(int id)
{
    pthread_mutex_lock_check(&mutex_end);
    if ((++end_counter) == M) {
        allEnd.store(true);
        FILE_LOG(logDEBUG4) << "++END COUNTER " << id << " " <<end_counter<<" "<<M<<std::flush;

        pthread_mutex_unlock(&mutex_end);
        return true;
    }
    FILE_LOG(logDEBUG4) << "+END COUNTER " << id << " " <<end_counter<<" "<<M<<std::flush;

    pthread_mutex_unlock(&mutex_end);
    return false;
}

void
matrix_controller::counter_decrement()
{
    pthread_mutex_lock_check(&mutex_end);
    end_counter--;
    pthread_mutex_unlock(&mutex_end);
}

// push id to matrix victim's request queue (FIFO)
void
matrix_controller::push_request(int victim, int id)
{
    pthread_mutex_lock_check(&sbb[victim]->mutex_workRequested);
    (sbb[victim]->requestQueue).push_back(id); // push ID into requestQueue of victim thread
    pthread_mutex_unlock(&sbb[victim]->mutex_workRequested);
}

// get request from request queue (FIFO)
int
matrix_controller::pull_request(int id)
{
    int thief;

    pthread_mutex_lock_check(&sbb[id]->mutex_workRequested);
    //error check
    if(sbb[id]->requestQueue.size()>=(unsigned)M){
        FILE_LOG(logERROR) << "Received too many requests" << sbb[id]->requestQueue.size();
        for(auto i: sbb[id]->requestQueue) FILE_LOG(logERROR) << i;
        exit(-1);
    }

    thief = (sbb[id]->requestQueue).front();
    (sbb[id]->requestQueue).pop_front();
    pthread_mutex_unlock(&sbb[id]->mutex_workRequested);
    return thief;
}

void
matrix_controller::request_work(int id)
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

        counter_decrement();
        FILE_LOG(logDEBUG4) << id << " cancel " << thief << " count: "<<end_counter;

//Note: For dependable use of condition variables, and to ensure that you do not lose wake-up operations on condition variables, your application should always use a Boolean predicate and a mutex with the condition variable.
//https://www.ibm.com/support/knowledgecenter/en/ssw_ibm_i_74/apis/users_76.htm
        pthread_mutex_lock_check(&sbb[thief]->mutex_shared);
        sbb[thief]->receivedWork=true;
        pthread_cond_signal(&sbb[thief]->cond_shared);
        pthread_mutex_unlock(&sbb[thief]->mutex_shared);
    }

    int victim = select_victim(id); // select victim

    FILE_LOG(logDEBUG4) << id << " select " << victim;

    if (victim != id) {
        pthread_mutex_lock_check(&sbb[id]->mutex_shared);
        sbb[id]->receivedWork=false;
        pthread_mutex_unlock(&sbb[id]->mutex_shared);

        push_request(victim, id);

        pthread_mutex_lock_check(&sbb[id]->mutex_shared);
        while (!sbb[id]->receivedWork && !allEnd.load()) {
            pthread_cond_wait(&sbb[id]->cond_shared, &sbb[id]->mutex_shared);
        }
        pthread_mutex_unlock(&sbb[id]->mutex_shared);
    } else {
        FILE_LOG(logDEBUG4) << id << " selected myself " << victim;
    //     // cancel...
        counter_decrement();
    }
} // matrix_controller::request_work

bool
matrix_controller::try_answer_request(int id)
{
    bool ret = false;

    //check if request queue empty ... called very often! some performance gain is possible by maintaining a separate boolean variable, but complicates code
    if(!sbb[id]->has_request())return false;

    //
    // if(thief!=-1)
    // {
    //     if(thief==id){
    //         printf("fatal error : thief == id"); fflush(stdout);
    //         exit(0);
    //     }
    //
    //     if(sbb[thief]->got_work){
    //         printf("fatal error : thief %d got work\n",thief); fflush(stdout);
    //         for(auto i: sbb[id]->requestQueue)
    //             printf("%d\t",i);
    //         printf("\n");
    //         exit(-1);
    //     }
    //     atom_nb_steals += work_share(id, thief);
    //
    //     atom_counter--;
    //     int ret=sem_post(&sbb[thief]->sema_wait);
    //     if (ret == -1) {
    //         int errno_saved = errno;
    //         fprintf(stderr, "An error occurred!");
    //         fprintf(stderr, "The error value is %d\n", errno_saved);
    //     }
    // }

    int thief = pull_request(id);
    //
    //     //        std::cout << id << " answer " << thief << std::endl << std::flush;
    //

    if(sbb[thief]->got_work){
        FILE_LOG(logERROR) << "id "<<id<<" FATAL error : thief "<<thief<<" got work";
        FILE_LOG(logERROR) << "and "<<sbb[id]->requestQueue.size()<<" pending requests";
        exit(-1);
    }

    pthread_mutex_lock_check(&sbb[thief]->mutex_ivm);
    atom_nb_steals = work_share(id, thief);
    pthread_mutex_unlock(&sbb[thief]->mutex_ivm);

    counter_decrement();
    FILE_LOG(logDEBUG4) << id << " answer " << thief << " counter: " << end_counter;

    pthread_mutex_lock_check(&sbb[thief]->mutex_shared);
    sbb[thief]->receivedWork=true;
    pthread_cond_signal(&sbb[thief]->cond_shared);
    pthread_mutex_unlock(&sbb[thief]->mutex_shared);

    // sem_post(&sbb[thief]->sema_wait);


    //
    //     if (!stolen) break;
    // }
    //
    // if (!sbb[id]->has_request()) sbb[id]->rqst = false;
    // // set_request(id, false);

    return ret;
} // matrix_controller::try_answer_request

int
matrix_controller::work_share(int id, int thief)
{
    int den = 2; // arguments::division;
    int num = 1;

    if(id==thief){perror("can't share with myself (mc)\n"); exit(-1);}
    if(id > M){
        perror("invalid victim ID (mc)\n"); exit(-1);
    }
    if(thief > M){
        perror("invalid thief ID (mc)\n"); exit(-1);
    }

    int ret = sbb[id]->shareWork(num, den, sbb[thief]);// ->IVM);//, matrices[thief]->state, matrices[thief]->bestcost);

    return ret;
}


int
matrix_controller::select_victim(int id)
{
    // default: select left neighbor
    int victim = (id == 0) ? (M - 1) : (id - 1);

    switch (arguments::mc_ws_select) {
        case 'r': {
            // randomly select active thread (at most nbIVM attempts)
            int attempts = 0;
            do {
                victim = rand() % M;
                if (++attempts > M) break;
            } while (victim == id || !sbb[victim]->got_work);
            break;
        }
        case 'o': {
            // select thread which has not made request for longest time
            pthread_mutex_lock(&mutex_steal_list);
            victim_list.remove(id);// remove id from list
            victim_list.push_back(id);// put at end
            victim = victim_list.front();// take first in list (oldest)

            if(!sbb[victim]->got_work)
                victim=(id == 0) ? (M - 1) : (id - 1);

            // FILE_LOG(logDEBUG4) << id << " list ...";
            // for(auto i:victim_list)
            // {
            //     FILE_LOG(logDEBUG4) << i;
            // }

            pthread_mutex_unlock(&mutex_steal_list);
            break;
        }
        default:
        {
            break;
        }
    }

    return victim;
}

void
matrix_controller::unlockWaiting(int id)
{
    FILE_LOG(logDEBUG1) << "Unlock all waiting";

    // unlock threads waiting for work
    for (int i = 0; i < M; i++) {
        if (i == id) continue;
        pthread_mutex_lock_check(&sbb[i]->mutex_shared);
        sbb[i]->receivedWork=true;
        pthread_cond_signal(&sbb[i]->cond_shared);
        pthread_mutex_unlock(&sbb[i]->mutex_shared);
        // sem_post(&sbb[i]->sema_wait);
    }

    // if(!requests.empty())requests.clear();
}

void
matrix_controller::stop(int id)
{
    unlockWaiting(id);

    int ret=pthread_barrier_wait(&barrier);
    if(ret==PTHREAD_BARRIER_SERIAL_THREAD){
        FILE_LOG(logDEBUG1) << "=== stop "<<M<<" ===";
    }

    // sem_destroy(&sbb[id]->sema_wait);
}

void
matrix_controller::interruptExploration()
{
    allEnd.store(true);
}

// run by multiple threads!!!
void
matrix_controller::explore_multicore()
{
    // get unique ID
    int id = explorer_get_new_id();

    //reset thread local
    // sbb[id]->reset();
    sbb[id]->reset_requestQueue();

    FILE_LOG(logDEBUG1) << std::flush;

    int ret = pthread_barrier_wait(&barrier);
    if(ret==PTHREAD_BARRIER_SERIAL_THREAD)
    {
        FILE_LOG(logDEBUG1) << "=== start "<<M<<" exploration threads ===";
    }

    // FILE_LOG(logDEBUG1) << std::flush;
    // pthread_barrier_wait(&barrier);

    // bool continuer = false;

    while (1) {
        pthread_mutex_lock_check(&sbb[id]->mutex_ivm);
        bool continuer = sbb[id]->next();
        pthread_mutex_unlock(&sbb[id]->mutex_ivm);

        if (!continuer && !allEnd.load()) {
            request_work(id);
            if(allEnd.load())break;
        }else{
            sbb[id]->set_workState(continuer);
            try_answer_request(id);
        }

        if (allEnd.load()) { // interrupted from outside or no more work reached
            break;
        }

        if(!arguments::singleNode)
        {
            // printf("chk\n");

            bool passed=pbb->ttm->period_passed(WORKER_BALANCING);
            if(atom_nb_steals>M || passed)
            {
                break;
            }

            if(pbb->sltn->newBest){
                break;
            }
        }
    }

    allEnd.store(true);
    // ret = pthread_barrier_wait(&barrier);
    stop(id);
} // matrix_controller::explore_multicore

void *
mcbb_thread(void * _mc)
{
    matrix_controller * mc = (matrix_controller *) _mc;

    mc->explore_multicore();

    return NULL;
}

void *
matrix_controller::bb_thread(void * _mc)
{
    return NULL;
}




void
matrix_controller::resetExplorationState()
{
    //reset global variables
    end_counter=0;// termination counter

    allEnd.store(false);
    atom_nb_explorers.store(0);// id_generator
    atom_nb_steals.store(0);// termination counter
}

bool
matrix_controller::next()
{
    resetExplorationState();

	FILE_LOG(logDEBUG1) << "start next " << end_counter;

    for (int i = 0; i < M; i++)
        pthread_create(sbb[i]->thread, NULL, mcbb_thread, (void *) this);

    for (int i = 0; i < M; i++)
    {
        // int err = pthread_join(threadId, &ptr);
        int err = pthread_join(*sbb[i]->thread, NULL);
        if (err)
        {
            std::cout << "Failed to join Thread : " << strerror(err) << std::endl;
            return err;
        }
    }

    return allEnd.load();// || periodPassed);
}


int
matrix_controller::getSubproblem(int *ret, const int N)
{
    // printf("get\n");

    // gpuErrchk(cudaMemcpy(depth_histo_h,depth_histo_d,size*sizeof(int int),cudaMemcpyDeviceToHost));

    int count=0;

    for (int i = 0; i < M; i++){
        if( sbb[i]->IVM->beforeEnd() ){
            count++;
        }
    }

    if(count==0)return 0;


    int nb=std::min(count,N);
    count=0;

    for(int i=0;i<M;i++)
    {
        if(count>=nb)break;

        if(sbb[i]->IVM->beforeEnd())
        {
            for(int k=0;k<size;k++){
                sbb[i]->getSchedule(&ret[count*size]);
                // ret[count*size+k]=sbb[i]->bd->node->schedule[k];
            }
            count++;
        }
    }

    // printf(";;; %d %d %d\n",count,N,nb);
    return nb;
}
