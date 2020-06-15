#ifndef BOUND_ABSTRACT_H
#define BOUND_ABSTRACT_H

#include "../include/instance_abstract.h"

struct bound_abstract {
    instance_abstract * instance;
    void set_instance(instance_abstract * _instance);

    virtual void init() = 0;

    // boundSubproblem
    virtual void bornes_calculer(int permutation[], int limite1, int limite2, int * couts, int) = 0;
    virtual void bornes_calculer(int permutation[], int limite1, int limite2) = 0;

    //in : subproblem p
    //out : costsBegin / costsEnd
    //compute bounds of children nodes of subproblem p
    //goal: avoid redundant computation of parts that are common to children nodes
    virtual void boundChildren(int *schedule, int limit1, int limit2, int * costsBegin, int * costsEnd, int* prioBegin, int* prioEnd) = 0;

    virtual void freeMem() = 0;
    virtual int evalSolution(int * permut) = 0;
    virtual ~bound_abstract() = 0;
};

#endif
