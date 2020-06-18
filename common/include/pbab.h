#include <pthread.h>
#include <semaphore.h>
#include "../include/misc.h"
#include "../include/arguments.h"
#include "../include/statistics.h"

#include "../../bounds/include/libbounds.h"
#include "../../heuristic/include/IG.h"

#ifndef PBAB_H
# define PBAB_H

# define ALIGN 64

class instance_abstract;
class bound_abstract;
class solution;
class ttime;

class pbab
{
public:
    int size;

    instance_abstract * instance;

    solution * sltn;
    solution * root_sltn;
    bool foundSolution;

    ttime * ttm;

    statistics stats;
    void
    printStats();

    void
    set_instance(char problem[], char inst_name[]);
    // void set_heuristic();

    pbab();
    ~pbab();

    void
    reset();

    pthread_mutex_t mutex_instance;

    void
    buildPriorityTables();
    void
    buildInitialUB();

    std::vector<std::tuple<std::vector<int>, std::vector<int> > > remain;
};

#endif // ifndef PBAB_H
