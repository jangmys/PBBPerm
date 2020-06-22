#ifndef FASTNEH_H_
#define FASTNEH_H_

#include <array>

#include "../../bounds/include/libbounds.h"


class fastNEH{
public:
    instance_abstract * instance;

    int nbJob;
    int nbMachines;
    int *PTM;

    int **head;
    int **tail;
    int **inser;

    int *sumPT;

    fastNEH(instance_abstract*_inst);
    ~fastNEH();

    void set_instance(instance_abstract*_instance);
    void allocate();

    int evalMakespan(int *perm, int len);

    void computeHeads(int* perm, int len);
    void computeTails(int* perm, int len);
    void insertJob(int *perm,int len, int job);

    void findBestPos(int &cost,int &bestPos,int len);
    void findBestPos(int &cost, int &bestpos, int len, int a, int b);
    void insertJobInBestPosition(int *perm,int job);

    void initialSort(int* perm);
    void runNEH(int *perm, int &cost);

    void insertLS(int *perm);
};


#endif
