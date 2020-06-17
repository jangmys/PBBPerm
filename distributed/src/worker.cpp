#include <sys/sysinfo.h>

#include <pthread.h>
#include <sched.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>

#include "../../common/headers/macros.h"
#include "../../common/headers/pbab.h"
#include "../../common/headers/solution.h"
#include "../../common/headers/ttime.h"
#include "../../common/headers/log.h"

#include "../../multicoreLL/headers/tree.h"
#include "../../multicoreLL/headers/treeheuristic.h"

#include "../headers/worker.h"
#include "../headers/fact_work.h"
#include "../headers/work.h"
#include "../headers/communicator.h"

worker::worker(pbab * _pbb)
{
    pbb  = _pbb;
    size = pbb->size;

    dwrk = std::make_shared<work>(pbb);

    pthread_barrier_init(&barrier, NULL, 2);// sync worker and helper thread

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);

    pthread_mutex_init(&mutex_wunit, &attr);
    pthread_mutex_init(&mutex_inst, &attr);
    pthread_mutex_init(&mutex_best, &attr);
    pthread_mutex_init(&mutex_end, &attr);
    pthread_mutex_init(&mutex_updateAvail, &attr);
    pthread_mutex_init(&mutex_trigger, &attr);

    pthread_mutex_init(&mutex_solutions, &attr);
    sol_ind_begin=0;
    sol_ind_end=0;
    max_sol_ind=2*arguments::heuristic_threads;
    solutions=(int*)malloc(max_sol_ind*size*sizeof(int));

    pthread_cond_init(&cond_updateApplied, NULL);
    pthread_cond_init(&cond_trigger, NULL);

    local_sol=new solution(pbb);
    // for(int i=0;i<size;i++)local_sol->bestpermut[i]=i;

    reset();
}

worker::~worker()
{
    pthread_barrier_destroy(&barrier);

    pthread_mutex_destroy(&mutex_wunit);
    pthread_mutex_destroy(&mutex_inst);
    pthread_mutex_destroy(&mutex_best);
    pthread_mutex_destroy(&mutex_end);
    pthread_mutex_destroy(&mutex_updateAvail);
    pthread_mutex_destroy(&mutex_trigger);

    pthread_mutex_destroy(&mutex_solutions);

    free(solutions);

    delete comm;
}

void
worker::reset()
{
    end     = false;
    newBest = false;
    shareWithMaster = true;
    updateAvailable = false;

    pbb->stats.totDecomposed = 0;
    pbb->stats.johnsonBounds = 0;
    pbb->stats.simpleBounds  = 0;
    pbb->stats.leaves = 0;
}

void
worker::wait_for_trigger(bool& check, bool& best)
{
    // wait for an event to trigger communication
    pthread_mutex_lock_check(&mutex_trigger);
    while (!sendRequest && !newBest) {
        pthread_cond_wait(&cond_trigger, &mutex_trigger);
    }
    check = sendRequest;
    best  = newBest;
    pthread_mutex_unlock(&mutex_trigger);
}

void
worker::wait_for_update_complete()
{
    // printf("wait for upd \t");
    pthread_mutex_lock_check(&mutex_updateAvail);
    // signal update
    updateAvailable = true;
    // wait until done
    while (updateAvailable) {
        pthread_cond_wait(&cond_updateApplied, &mutex_updateAvail);
    }
    pthread_mutex_unlock(&mutex_updateAvail);
    // printf("... complete %d \n",comm->rank);
}

// use main thread....
void *
comm_thread(void * arg)
{
    worker * w = (worker *) arg;

    MPI_Status status;

    w->sendRequestReady = true;
    w->sendRequest      = false;
    w->newBest = false;

    pthread_barrier_wait(&w->barrier);

    int nbiter = 0;
    int dummy  = 11;

    solution* mastersol=new solution(w->pbb);

    int masterbest;

    while (1) {
        if (w->checkEnd()) break;

        // printf("--- COMM READY.....\n");fflush(stdout);
        // wait unitl update applied
        bool doCheckpoint;
        bool doBest;
        w->wait_for_trigger(doCheckpoint, doBest);

        // printf("---\n");
        fflush(stdout);
        nbiter++;
        // checkpoint triggered
        if (doCheckpoint) {
            // printf("%d === CHECKPOINT\n",w->comm->rank);fflush(stdout);
            pthread_mutex_lock_check(&w->mutex_trigger);
            w->sendRequest = false;
            pthread_mutex_unlock(&w->mutex_trigger);

            // CONVERT TO MPZ INTEGER INTERVALS AND SORT...
            w->work_buf->fact2dec(w->dwrk);
            //w->dwrk->displayUinterval();

            w->comm->send_work(w->dwrk, 0, WORK);
            //            w->comm->send_fwork(0, WORK);
            //            printf("send work unit...\n");fflush(stdout);//DEBUG
        } else if (doBest) {
            // printf("=== BEST\n");fflush(stdout);
            w->newBest = false;
            // get sol
            w->pbb->sltn->getBestSolution(w->comm->best_buf->bestpermut,
                w->comm->best_buf->bestcost);// lock on pbb->sltn
            w->comm->send_sol(w->comm->best_buf, 0, BEST);
            // printf("SEND BEST %d ...\n",w->comm->rank   );fflush(stdout);//DEBUG
        }

        /*
         *  ==============================================
         *  RECEIVE
         *  ==============================================
         */
        // printf("%d === wait\n",w->comm->rank);fflush(stdout);
        MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        switch(status.MPI_TAG)
        {
            case WORK:
            {
                // receive buffer...
                std::shared_ptr<work> rwrk(new work(w->pbb));

                FILE_LOG(logDEBUG4)<<"worker receives";
                w->comm->recv_work(rwrk, 0, MPI_ANY_TAG, &status);
                w->work_buf->dec2fact(rwrk);

                // wait unitl update applied
                w->wait_for_update_complete();
                break;
            }
            case BEST:
            {
                // printf("worker receive best\n");fflush(stdout);
                MPI_Recv(&masterbest, 1, MPI_INT, status.MPI_SOURCE, BEST, MPI_COMM_WORLD, &status);
                w->pbb->sltn->updateCost(masterbest);
                break;
            }
            case END:
            {
                MPI_Recv(&dummy, 1, MPI_INT, 0, END, MPI_COMM_WORLD, &status);
                FILE_LOG(logINFO) << "Rank " << w->comm->rank << " terminates.";
                w->end = true;
                break;
            }
            case NIL:
            {
                w->comm->recv_sol(mastersol, 0, NIL, &status);
                // printf("receive NIL=== %d ===\n",w->pbb->sltn->bestcost );fflush(stdout);
                // MPI_Recv(&masterbest, 1, MPI_INT, 0, NIL, MPI_COMM_WORLD, &status);
                w->pbb->sltn->update(mastersol->bestpermut,mastersol->bestcost);
                break;
            }
            case SLEEP:
            {
                MPI_Recv(&dummy, 1, MPI_INT, 0, SLEEP, MPI_COMM_WORLD, &status);
                usleep(10);
                w->shareWithMaster=false;
                break;
            }
            default:
            {
                FILE_LOG(logERROR) << "unknown message";
                exit(-1);
            }
        }

        // can handle new request now... set flag to true
        // work buffer can be reused!!!
        // if work units were received: the update was taken into account (see cond_updateApplied)
        // else: buffer was used only for send (operation completed)
        pthread_mutex_lock_check(&w->mutex_trigger);
        w->sendRequestReady = true;
        pthread_mutex_unlock(&w->mutex_trigger);
    }

    // // confirm that termination signal received... comm thread will be joined
    // MPI_Send(&dummy, 1, MPI_INT, 0, END, MPI_COMM_WORLD);
    // printf("end-comm / iter:\t %d\n",nbiter);fflush(stdout);

    FILE_LOG(logDEBUG1) << "comm thread return";

    pthread_exit(0);
    // return NULL;
} // comm_thread

bool
worker::commIsReady()
{
    bool isReady = false;
    pthread_mutex_lock_check(&mutex_trigger);
    isReady = sendRequestReady;
    pthread_mutex_unlock(&mutex_trigger);
    return isReady;
}

void
worker::tryLaunchCommBest()
{
    if (commIsReady()) {
        pthread_mutex_lock_check(&mutex_trigger);
        sendRequestReady = false;
        newBest = true;
        pthread_cond_signal(&cond_trigger);
        pthread_mutex_unlock(&mutex_trigger);
        pbb->sltn->newBest = false;
    }
}

void
worker::tryLaunchCommWork()
{
    // printf("mmmm\n");
    // printf("get %d intervals\n",work_buf->nb_intervals); fflush(stdout);

    if (commIsReady()) {
        pthread_mutex_lock_check(&mutex_wunit);
        getIntervals();// fill buffer (prepare SEND)
        pthread_mutex_unlock(&mutex_wunit);

        pthread_mutex_lock_check(&mutex_trigger);
        sendRequestReady = false;
        sendRequest      = true;// comm thread uses this to distinguish comm tasks  (best/checkpoint)
        pthread_cond_signal(&cond_trigger);
        pthread_mutex_unlock(&mutex_trigger);
        trigger = false;// reset...
    }
}

bool
worker::checkEnd()
{
    bool stop = false;
    pthread_mutex_lock_check(&mutex_end);
    stop = end;
    pthread_mutex_unlock(&mutex_end);
    return stop;
}

bool
worker::checkUpdate()
{
    bool doUpdate = false;
    pthread_mutex_lock_check(&mutex_updateAvail);
    doUpdate = updateAvailable;
    pthread_mutex_unlock(&mutex_updateAvail);
    return doUpdate;
}

//performs heuristic in parallel to exploration process
void *
heu_thread2(void * arg)
{
    pthread_detach(pthread_self());

    worker * w = (worker *) arg;

    pthread_mutex_lock_check(&w->pbb->mutex_instance);
    treeheuristic *th=new treeheuristic(0,w->pbb);
    IG* ils=new IG(w->pbb->instance);
    th->strategy=PRIOQ;
    pthread_mutex_unlock(&w->pbb->mutex_instance);

    int N=w->pbb->size;
    subproblem *s=new subproblem(N);
    int cost;

    while(!w->checkEnd()){
        w->pbb->sltn->getBestSolution(s->schedule,cost);// lock on pbb->sltn
        int r=helper::intRand(0,100);

        pthread_mutex_lock_check(&w->mutex_solutions);
        if(w->sol_ind_begin<w->sol_ind_end && r<70){
            if(w->sol_ind_begin >= w->max_sol_ind){
                FILE_LOG(logERROR) << "Index out of bounds";
                exit(-1);
            }

            for(int i=0;i<N;i++){
                s->schedule[i]=w->solutions[w->sol_ind_begin*N+i];
            }
            w->sol_ind_begin++;
        }
        pthread_mutex_unlock(&w->mutex_solutions);

        int c;

        ils->igiter=arguments::heuristic_iters;
        s->limit1=-1;
        s->limit2=N;

        c=ils->runIG(s);
        c=th->run(s,cost);

        pthread_mutex_lock_check(&w->mutex_solutions);
        FILE_LOG(logINFO)<<"heu "<<*s<<" "<<c<<std::endl;
        pthread_mutex_unlock(&w->mutex_solutions);

        if (c<cost){
            w->pbb->sltn->update(s->schedule,c);
            // printf("hhh %d\t",c);
            // s->print();
            FILE_LOG(logINFO)<<"HeuristicBest "<<c<<"\t"<<*(w->pbb->sltn);
            w->newBest=true;
        }
        if(c<w->local_sol->bestcost){
            w->local_sol->update(s->schedule,c);
            FILE_LOG(logINFO)<<"LocalBest "<<c<<"\t"<<*(w->local_sol);
        }
    }

    // delete ils;
    delete th;

    pthread_exit(0);
    // return NULL;
}

//performs heuristic in parallel to exploration process
void *
heu_thread(void * arg)
{
    pthread_detach(pthread_self());

    worker * w = (worker *) arg;

    pthread_mutex_lock_check(&w->pbb->mutex_instance);
    IG* ils=new IG(w->pbb->instance);
    pthread_mutex_unlock(&w->pbb->mutex_instance);

    //how many iterations in ILS heuristic:
    ils->igiter=arguments::heuristic_iters;

    int N=w->pbb->size;
    subproblem *s=new subproblem(N);
    subproblem *s2=new subproblem(N);

    s->limit1=-1;
    s->limit2=N;
    int cost;
    // int cost2;

    while(!w->checkEnd()){
        // w->local_sol->getBestSolution(s->schedule,cost);// lock on pbb->sltn
        w->pbb->sltn->getBestSolution(s->schedule,cost);// lock on pbb->sltn

        int r=helper::intRand(0,10);

        pthread_mutex_lock_check(&w->mutex_solutions);
        if(w->sol_ind_begin<w->sol_ind_end && r<8){
            if(w->sol_ind_begin >= w->max_sol_ind){
                FILE_LOG(logDEBUG1) << "Index out of bounds";
                exit(-1);
            }

            for(int i=0;i<N;i++){
                s->schedule[i]=w->solutions[w->sol_ind_begin*N+i];
            }
            w->sol_ind_begin++;

        }
        pthread_mutex_unlock(&w->mutex_solutions);

        int c=999999999;
        // w->local_sol->getBestSolution(s2->schedule,cost2);
        // c=ils->vbih(s,s2);
        c=ils->runIG(s); //ils->makespan(s);

        pthread_mutex_lock_check(&w->mutex_solutions);
        FILE_LOG(logINFO)<<"Heuristic Sol "<<c<<" Best: "<<w->pbb->sltn->bestcost;
        // for(int i=0;i<N;i++)
            // FILE_LOG(logINFO)<<s->schedule[i];

        // printf("%d ",s->schedule[i]);
        // // printf("\t %d %d\n",c,w->local_sol->bestcost);
        // printf("\t %d %d\n",c,w->pbb->sltn->bestcost);
        pthread_mutex_unlock(&w->mutex_solutions);

        if (c<w->pbb->sltn->bestcost){
            w->pbb->sltn->update(s->schedule,c);
            // printf("hhh %d\t",c);
            // s->print();
            FILE_LOG(logINFO)<<"HeuristicBest "<<c<<"\t"<<*(w->pbb->sltn);
            w->newBest=true;
        }
        if(c<w->local_sol->bestcost){
            w->local_sol->update(s->schedule,c);
            FILE_LOG(logINFO)<<"LocalBest "<<c<<"\t"<<*(w->local_sol);
        }
    }

    delete s;
    delete s2;
    delete ils;

    pthread_exit(0);
    // return NULL;
}


// worker main thread : spawn communicator
void
worker::run()
{
    // create comm thread
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_t * comm_thd = (pthread_t *) malloc(sizeof(pthread_t));
    pthread_create(comm_thd, &attr, comm_thread, (void *) this);

    bool allEnd = false;
    // trigger = true;// true : got no work
    pthread_barrier_wait(&barrier);// synchronize with communication thread

    //heuristic threads
    int nbHeuThds=arguments::heuristic_threads;
    pthread_t heur_thd[100];
    for(int i=0;i<nbHeuThds;i++)
    {
        pthread_create(&heur_thd[i], NULL, heu_thread2, (void *) this);
    }
    FILE_LOG(logDEBUG) << "Created " << nbHeuThds << " heuristic threads.";
    int workeriter = 0;

    // printf("RUN\n");fflush(stdout);

    // ==========================================
    // worker main-loop :
    // do work or try acquire new work unit until END signal received
    while (1) {
        workeriter++;

        // if comm thread has set END flag, exit
        if (checkEnd()) {
            FILE_LOG(logINFO) << "End detected";
            break;
        }

        // if UPDATE flag set (by comm thread), apply update and signal
        // (comm thread is waiting until update applied)
        if (checkUpdate()) {
            FILE_LOG(logDEBUG) << "Update work unit";
            updateWorkUnit();// read buffer (RECV)
        }


        // work is done here... explore intervals(s)
        //        pbb->ttm->on(pbb->ttm->workerExploretime);

        // printf("iter : %d\n",comm->rank);
        allEnd = doWork();

        if(arguments::heuristic_threads)
            getSolutions();

        //        pbb->ttm->off(pbb->ttm->workerExploretime);

        //        bool isReady=false;

        // printf("rrrrrrrr : %d\t",comm->rank);
        if (pbb->sltn->newBest) {
            FILE_LOG(logDEBUG) << "Try launch best-communication";
            tryLaunchCommBest();
        }
        if(shareWithMaster || allEnd){
            FILE_LOG(logDEBUG) << "Try launch work-communicaion";
            tryLaunchCommWork();
        }
    }

    int err;
    // printf("waintng for coomths\n"); fflush(stdout);
    // pthread_join(*thd, NULL);

    err = pthread_join(*comm_thd, NULL);
    if (err)
    {
        FILE_LOG(logERROR) << "Failed to join comm thread " << strerror(err);
    }

    pbb->ttm->logElapsed(pbb->ttm->workerExploretime, "Worker exploration time\t");

    // confirm that termination signal received... comm thread will be joined
    int dummy = 42;
    MPI_Send(&dummy, 1, MPI_INT, 0, END, MPI_COMM_WORLD);
    // printf("end-comm / iter:\t %d\n",comm->rank);fflush(stdout);

    pthread_attr_destroy(&attr);
    free(comm_thd);
} // worker::run
