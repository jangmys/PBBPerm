#include <pthread.h>
#include <semaphore.h>
#include "../include/misc.h"
#include "../include/arguments.h"
#include "../include/statistics.h"

#include "../../bounds/include/libbounds.h"
#include "../../heuristic/include/IG.h"


#ifndef PBAB_H
#define PBAB_H

#define ALIGN 64

class instance_abstract;
class bound_abstract;
class solution;
class ttime;

class pbab
{
public:
    int size;

    instance_abstract * instance;
    solution* sltn;
    solution* root_sltn;

    // weights* wghts;//only for interval?
    ttime* ttm;

    IG* ils;

    bool foundSolution;

    statistics stats;
    void printStats();

    void set_instance(char problem[],char inst_name[]);
    void set_heuristic();

    // pbab(instance_abstract * _instance);
    pbab();
    ~pbab();

    void reset();

    pthread_mutex_t mutex_instance;

    void buildPriorityTables();
    void buildInitialUB();
    // int *posFreq;
    // int *fwdFreq;
    // int *bwdFreq;

    std::vector<std::tuple< std::vector<int>,std::vector<int> >> remain;
    std::vector<std::vector<int>> begins;
    std::vector<std::vector<int>> ends;
};

#endif // ifndef PBAB_H
