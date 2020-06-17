#include "macros.h"

#include "../include/bbthread.h"

bool
bbthread::has_request()
{
    pthread_mutex_lock(&workRequested_mutex);
    bool ok = (requestQueue.empty()) ? false : true;
    pthread_mutex_unlock(&workRequested_mutex);
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

/*
not needed by sequential
*/
subproblem *
bbthread::top_share()
{
    switch (strategy) {
        case DEQUE:
            return deq.back();
        case STACK:
            return pile.top();
        case PRIOQ:
            return pque.top();
        default:
            std::cout << "Undeended strategy";
            exit(1);
    }
}
/*
not needed by sequential
*/
void
bbthread::pop_share()
{
    switch (strategy) {
        case DEQUE:
            deq.pop_back();
            break;
        case STACK:
            pile.pop();
            break;
        case PRIOQ:
            pque.pop();
        default:
            std::cout << "Undefined strategy";
            exit(1);
    }
}
