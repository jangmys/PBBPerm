#ifndef TREE_H
#define TREE_H

#define DEQUE       'd'
#define STACK       's'
#define PRIOQ       'p'

#define BEGIN_ORDER 0
#define END_ORDER   1

#define SIMPLE      0
#define JOHNSON     1

#include "../../common/inih/INIReader.h"

#include "subproblem.h"

#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>

#include <stack>
#include <queue>
#include <atomic>

class bound_abstract;
class subproblem;
class pbab;

struct lb_compare {
    bool
    operator () (subproblem const * p1, subproblem const * p2) const
    {
        if(p1->cost > p2->cost)return true;
        if(p1->cost < p2->cost)return false;
        return (p1->depth < p2->depth);
        // return p1->cost < p2->cost;
    }
};

//p1 "greater" p2 --> smallest on top
struct ub_compare {
    bool
    operator () (subproblem const * p1, subproblem const * p2) const
    {
        // float w=0.5;//0.9;
        // if((w*p1->ub+(1.0-w)*p1->cost) > (w*p2->ub+(1.0-w)*p2->cost))return true;
        // return false;

        if(p1->ub > p2->ub)return true;
        if(p1->ub < p2->ub)return false;
        return (p1->depth > p2->depth); //prefer breadth
        // return (p1->depth < p2->depth); //prefer deep
        // return (p1->cost > p2->cost);
    }
};

class Tree
{
private:
    // int size;
public:
    pbab * pbb;

    Tree(unsigned int id, pbab * pbb);
    ~Tree(){ };

    // different...priority_queue < problem *, vector < problem * >
    int strategy;

    // gestion pool
    std::deque<subproblem *> deq;
    std::stack<subproblem *> pile;
    std::priority_queue<subproblem *, std::vector<subproblem *>, ub_compare> pque;

    int id;
    int local_best;
    int initialUB;

    int * prioFwd;
    int * prioBwd;

    bound_abstract * lb[2];

    // int poolsizes;

    // statistique sur la taille du pool
    // long unsigned int explored_problems;
    // long unsigned int total_sizes;
    // long unsigned int nbBranch;
    // long unsigned int nbStealIntern;

    // struct timespec bnb_time;

    // gestion taille
    // pthread_mutex_t size_mutex;
    // int cumulSize;
    // int maxSize;

    // unsigned long long int iter;

    // gestion request
    // int workRequested;

    void
    setRoot(const int * root);

    bool
    next();
    void
    improve(const subproblem * pr);
    void
    branch(subproblem * pr);

    int
    chooseBranching(std::vector<subproblem *> forward, std::vector<subproblem *> backward);
    int
    chooseBranching(int * cb, int * ce);

    bool
    bound(subproblem * pr);
    void
    insert(std::vector<subproblem *>&ns);
    void
    del(std::vector<subproblem *>&ns);
    void
    insert(subproblem * n);
    std::vector<subproblem *>
    decompose_fast(subproblem& n);                          // ,std::vector < subproblem * >&ns);
    std::vector<subproblem *>
    decompose(subproblem& n);

    int
    size();
    void
    push(subproblem * p);
    void
    push_back(subproblem * p);
    void
    pop_back();
    void
    pop();
    subproblem *
    top();
    subproblem *
    back();
    bool
    empty();
    subproblem *
    take();
    // bool
    // init();
    void
    clearPool();


    // int mysize();
    // bool has_request();
    // int who_request();
};
#endif // ifndef TREE_H
