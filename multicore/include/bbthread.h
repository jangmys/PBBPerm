#ifndef BBTHREAD_H_
#define BBTHREAD_H_

#include <deque>

#include "sequentialbb.h"

class bbthread : public sequentialbb
{
public:
    pthread_t * thread;

    bbthread(pbab * pbb) : sequentialbb(pbb)
    {
        thread = (pthread_t *) malloc(sizeof(pthread_t));

        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
        pthread_mutex_init(&mutex_ivm, &attr);
        pthread_mutex_init(&mutex_workRequested, &attr);
        pthread_mutex_init(&mutex_workState, &attr);

        requestQueue.clear();

        pthread_mutex_init(&mutex_shared, &attr);
        pthread_cond_init(&cond_shared, NULL);
    };

    ~bbthread()
    {
        free(thread);
        pthread_mutex_destroy(&mutex_ivm);
        pthread_mutex_destroy(&mutex_workRequested);
        pthread_mutex_destroy(&mutex_workState);
    }

    bool receivedWork;
    bool got_work;

    std::deque<int> requestQueue;
    // sem_t sema_wait; //replaced with condition variable

    pthread_mutex_t mutex_ivm;
    pthread_mutex_t mutex_workState;
    pthread_mutex_t mutex_workRequested;

    pthread_mutex_t mutex_shared;
    pthread_cond_t cond_shared;

    // std::atomic<bool> gotWork;

    bool has_request();
    // void reset();
    void reset_requestQueue();

    bool set_workState(const bool newstate);
    int shareWork(int,int,bbthread*);

    void getSchedule(int *sch);
};


#endif // ifndef BBTHREAD_H_
