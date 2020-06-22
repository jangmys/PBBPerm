#ifndef IG_H_
#define IG_H_

#include "../../common/include/misc.h"
#include "../../common/include/subproblem.h"

#include "fspnhoods.h"
#include "fastNEH.h"

class IG{
public:
    int igiter;

    int nbJob;
    int nbMachines;

    int *perm;
    int *removed;

    fastNEH *neh;
    fspnhood * nhood;

    int *visitOrder;

    int destructStrength;
    float acceptanceParameter;
    float avgPT;

    IG(instance_abstract * inst);
    ~IG();

    int makespan(subproblem* s);

    void runIG();
    // void runIG(permutation* current, int l1, int l2);

    int runIG(subproblem* s);
    // int runIG(subproblem* current,subproblem* guide);

    void shuffle(int *array, int n);

    void destruction(int *perm, int *permOut, int k);
    void construction(int *perm, int *permOut, int k);
    void blockConstruction(int *perm, int *permOut, int k);

    bool acceptance(int tempcost, int cost, float param);

    void destruction(int *perm, int *permOut, int k, int a, int b);
    void construction(int *perm, int *permOut, int k,int a, int b);

    void perturbation(int *perm, int k, int a, int b);

    int localSearch(int *perm, int l1, int l2);
    int localSearchBRE(int *arr);
    int localSearchKI(int *arr,int kmax);
    int localSearchPartial(int *arr,const int len);

    // int ris(subproblem* curr,subproblem* guiding);
    // int vbih(subproblem* current, subproblem* guiding);
};


#endif
