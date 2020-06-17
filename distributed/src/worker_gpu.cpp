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

#include "../headers/worker.h"
#include "../headers/fact_work.h"
#include "../headers/work.h"
#include "../headers/communicator.h"

#ifdef USE_GPU
#include "../headers/worker_gpu.h"
#include "../../gpu/headers/gpubb.h"
#endif

void
worker_gpu::interrupt()
{
    gbb->interruptExploration();
}

bool
worker_gpu::doWork()
{
    pbb->ttm->on(pbb->ttm->workerExploretime);

    //    printf("dowork///\n");
    bool allEnd = false;

    #ifdef USE_GPU
    allEnd = gbb->next();
    #endif

    pbb->ttm->off(pbb->ttm->workerExploretime);

    return allEnd; //triggerComm;// comm condition met
}

void
worker_gpu::updateWorkUnit()
{
    pthread_mutex_lock_check(&mutex_wunit);
    #ifdef USE_GPU
    gbb->initFromFac(
        work_buf->nb_intervals,
        work_buf->ids,
        work_buf->pos,
        work_buf->end
    );
    #endif
    pthread_mutex_unlock(&mutex_wunit);

    pthread_mutex_lock_check(&mutex_updateAvail);
    updateAvailable = false;
    pthread_mutex_unlock(&mutex_updateAvail);
    pthread_cond_signal(&cond_updateApplied);
}

// copies work units from GPU (resp. thread-private IVMs) to communicator-buffer
// --> prepare SEND
void
worker_gpu::getIntervals()
{
    #ifdef USE_GPU
    gbb->getIntervals(
        work_buf->pos,
        work_buf->end,
        work_buf->ids,
        work_buf->nb_intervals,
        work_buf->max_intervals
    );


    dwrk->exploredNodes      = pbb->stats.totDecomposed;
    dwrk->nbLeaves           = pbb->stats.leaves;
    pbb->stats.totDecomposed = 0;
    pbb->stats.leaves        = 0;
    #endif
}

void
worker_gpu::getSolutions()
{
    //still room in buffer
    if(sol_ind_begin<sol_ind_end)return;

    pthread_mutex_lock_check(&mutex_solutions);
    int nb=gbb->getDeepSubproblem(solutions,max_sol_ind);

    if(nb){
        sol_ind_begin=0;
        sol_ind_end=nb;
    }
    pthread_mutex_unlock(&mutex_solutions);
}
