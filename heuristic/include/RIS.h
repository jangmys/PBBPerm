#ifndef RIS_H_
#define RIS_H_

#include "../../common/headers/misc.h"
#include "../../common/headers/subproblem.h"
#include "../../common/headers/permutation.h"

#include "../headers/fspnhoods.h"
#include "fastNEH.h"

class RIS{
public:
    // int igiter;
    //
    int nbJob;
    int nbMachines;
    //
    // int *perm;
    // int *removed;
    //
    // fastNEH *neh;
    fspnhood * nhood;
    //
    // int *visitOrder;
    //
    // int destructStrength;
    // float acceptanceParameter;
    // float avgPT;

    RIS(instance_abstract * inst);
    ~RIS();

    // int makespan(subproblem* s);
    //
    // void runIG();
    // void runIG(permutation* current, int l1, int l2);
    int run(subproblem* s, subproblem* guide);
    //
    // void shuffle(int *array, size_t n);
    //
    // void destruction(int *perm, int *permOut, int k);
    // void construction(int *perm, int *permOut, int k);
    // bool acceptance(int tempcost, int cost, float param);
    //
    // void destruction(int *perm, int *permOut, int k, int a, int b);
    // void construction(int *perm, int *permOut, int k,int a, int b);
    //
    // void perturbation(int *perm, int k, int a, int b);
    //
    // int localSearch(int *perm, int l1, int l2);
    // int localSearchBRE(int *arr);
    // int localSearchKI(int *arr,int kmax);
};


#endif
