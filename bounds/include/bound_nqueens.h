#ifndef BOUND_NQUEENS_H
#define BOUND_NQUEENS_H

#include "instance_abstract.h"
#include "bound_abstract.h"

struct bound_nqueens : public bound_abstract {
    int branchingMode;
    int size;
    void
    init();

    // defined in bound_abstract
    void
    set_instance(instance_abstract * _instance);
    void
    bornes_calculer(int permutation[], int limite1, int limite2, int * couts, int);
    void
    bornes_calculer(int permutation[], int limite1, int limite2){ };
    void
    boundChildren(int * schedule, const int limit1, const int limit2, int * costsBegin, int * costsEnd, int * prioBegin,
      int * prioEnd);

    void
    freeMem();
    int
    evalSolution(int * permutation);

    ~bound_nqueens();
};

#endif // ifndef BOUND_NQUEENS_H
