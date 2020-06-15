#include "../../common/include/macros.h"

#include "../../common/include/pbab.h"

#include "../include/bbthread.h"

void bbthread::reset_requestQueue()
{
    requestQueue.clear();
}

bool
bbthread::has_request()
{
    pthread_mutex_lock_check(&mutex_workRequested);
    bool ok = (requestQueue.empty()) ? false : true;
    pthread_mutex_unlock(&mutex_workRequested);
    return ok;
}

bool
bbthread::set_workState(const bool newstate)
{
    int oldstate=got_work;
    pthread_mutex_lock_check(&mutex_workState);
    got_work=newstate;
    pthread_mutex_unlock(&mutex_workState);
    return oldstate;
}

void
bbthread::getSchedule(int *sch)
{
    for (int i = 0; i < size; i++) {
        sch[i]=bd->node->schedule[i];
    }
}

int
bbthread::shareWork(int numerator, int denominator, bbthread *thief_thread)
{
    int numShared = 0;
    int l         = 0;

    ivm* thief = thief_thread->IVM;

    while (IVM->posVect[l] == IVM->endVect[l] && l < IVM->line && l < size - 10) l++;

    if (IVM->posVect[l] < IVM->endVect[l])
    {
        numShared++;
        for (int i = 0; i < l; i++) {
            thief->posVect[i] = IVM->posVect[i];
            for (int j = 0; j < size; j++) thief->jobMat[i * size + j] = IVM->jobMat[i * size + j];
            thief->dirVect[i] = IVM->dirVect[i];
        }
        for (int i = 0; i < size; i++) thief->endVect[i] = IVM->endVect[i];
        for (int i = 0; i < size; i++) thief->jobMat[l * size + i] = IVM->jobMat[l * size + i];
        thief->dirVect[l] = IVM->dirVect[l];

        thief->posVect[l] = IVM->cuttingPosition(l, 2);
        IVM->endVect[l]   = thief->posVect[l] - 1;

        // remaining levels : align thief left, victim right
        for (int i = l + 1; i < size; i++) thief->posVect[i] = 0;
        for (int i = l + 1; i < size; i++) IVM->endVect[i] = size - i - 1;

        thief->line = l;
    }

    return numShared;
}
