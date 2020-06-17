#ifndef BBTHREAD_H_
#define BBTHREAD_H_

#include "tree.h"

class bbthread : public Tree
{
public:
    bbthread(unsigned int i, pbab* pbb) : Tree(i,pbb)
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);

        workRequested = -1;  // false;
        pthread_mutex_init(&workRequested_mutex, &attr);
        pthread_mutex_init(&mutex_workRequested, &attr);
        pthread_mutex_init(&mutex_workState, &attr);
        // sem_init(&sema_wait, 0, 0);
        // pthread_mutex_init(&size_mutex, NULL);

        if(i==0)hasWork = true;

        pthread_mutex_init(&mutex_sharedWrk, &attr);
        pthread_cond_init(&cond_sharedWrk, NULL);
    };
    ~bbthread(){
        pthread_mutex_destroy(&mutex_workRequested);
        pthread_mutex_destroy(&mutex_workState);
    };

    bool waiting;
    bool got_work;


    bool has_request();
    bool set_workState(const bool newstate);

    pthread_mutex_t mutex_sharedWrk;
    pthread_cond_t cond_sharedWrk;

    std::deque<int>requestQueue;
    pthread_mutex_t workRequested_mutex;
    pthread_mutex_t mutex_workRequested;
    pthread_mutex_t mutex_workState;
    int workRequested;

    // sem_t sema_wait;
    std::atomic<bool> rqst;

    // std::atomic<bool> gotWork;

    bool hasWork;

    subproblem *top_share();
    void pop_share();

    // bbthread(int i, pbab * pbb) : Tree(i,pbb)
    // {
    //     thread = (pthread_t *) malloc(sizeof(pthread_t));
    //
    //     pthread_mutexattr_t attr;
    //     pthread_mutexattr_init(&attr);
    //     pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    //
    //     //only multithreading..
    //     workRequested = -1;  // false;
    //     pthread_mutex_init(&workRequested_mutex, &attr);
    //     pthread_mutex_init(&mutex_workRequested, &attr);
    //     pthread_mutex_init(&size_mutex, &attr);
    //     sem_init(&sema_wait, 0, 0);
    //
    //     // reset();//semaphore
    // };
    //
    // ~bbthread()
    // {
    //     free(thread);
    //     pthread_mutex_destroy(&workRequested_mutex);
    //     pthread_mutex_destroy(&mutex_workRequested);
    //     pthread_mutex_destroy(&size_mutex);
    // }
};


#endif
