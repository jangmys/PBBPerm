#include <pthread.h>
#include <atomic>
#include <memory>
#include <vector>
#include <list>
#include <deque>

#ifndef MATRIX_CONTROLLER_H
#define MATRIX_CONTROLLER_H

#define MAX_EXPLORERS 1024

class pbab;
class bbthread;

class matrix_controller{
private:
    std::atomic<int> atom_nb_explorers;

    std::atomic<bool> allEnd;
    std::atomic<int> atom_nb_steals;

    pthread_mutex_t mutex_requests;
    std::deque<int>requests;

    pthread_mutex_t mutex_steal_list;
    std::list<int> victim_list;

    int size;
    pthread_barrier_t barrier;
    pthread_mutex_t mutex_end;
    int end_counter;

    int M;

    bbthread *sbb[MAX_EXPLORERS];
    pbab* pbb;

public:
    matrix_controller(pbab* _pbb);
    ~matrix_controller();

    void initFullInterval();

    void initFromFac(const int nbint, const int* ids, int*pos, int* end);
    void getIntervals(int *pos, int* end, int *ids, int &nb_intervals, const int max_intervals);
    int getNbIVM();
    bool solvedAtRoot();

    void allocate();
    int select_victim(int id);

    // void set_request(const int i, const bool req);
    void push_request(int victim, int id);
    int pull_request(int id);

    // bool has_work(int id);
    int explorer_get_new_id();
    bool counter_increment(int id);
    void counter_decrement();

    void request_work(int id);
    bool try_answer_request(int id);

    void stop(int id);
    int work_share(int id, int thief);

    void unlockWaiting(int id);
    void resetExplorationState();
    void interruptExploration();

    int getSubproblem(int *ret, const int N);

    bool next();
    void printStats();

    void explore_multicore();

    void *bb_thread(void * _mc);
};

#endif
