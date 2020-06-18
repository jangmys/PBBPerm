#ifndef BOUND_FSP_STRONG_H
#define BOUND_FSP_STRONG_H

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "bound_abstract.h"

class pbab;

struct bound_fsp_strong : public bound_abstract {
    int          branchingMode;
    int          earlyExit;
    int          machinePairs;

    int          nbJob;      // Nombre de jobs
    int          nbMachines; // Nombre de machines
    int          somme;      // 5:10, 10:45, 20:190.Number of machine pairs

    static int * tabJohnson;  // [somme][nbJob]Donne l'ordonnancement selon
    static int * tempsJob;    // [nbMachines][nbJob]Donne le temps des jobs sur
    static int * tempsLag;    // [somme][nbJob]Donne le temps mis par un job
    static int * minTempsArr; // [nbMachines]Donne le temps minimale mis pour
    static int * minTempsDep; // [nbMachines]Donne le temps minimale mis pour
    static int * machine;     // [2];           // [somme]Pour chaque couple nous donne le numero

    int *        front;
    int *        back;
    int *        remain;
    int *        flag;

    int *        rewards;

    int *        countMachinePairs;
    int *        machinePairOrder;
    int *        pluspetit[2];

    void
    init();
    void
    configureBound(const int, const int, const int);

    void
    initCmax(int * tmp, int * ma, int ind);
    void
    cmaxFin(int * tmp, int * ma);
    void
    heuristiqueCmax(int * tmp, int * ma, int ind);
    int
    borneInfMakespan(int * valBorneInf, int minCmax);

    int
    borneInfLearn(int * valBorneInf, int UB, bool earlyExit);

    void
    remplirNbJobNbMachines();
    void
    remplirTempsJob();

    void
    initSomme();
    void
    allouerMemoire();
    void
    remplirLag();

    void
    remplirMachine();
    void
    remplirTabJohnson();
    void
    remplirTempsArriverDepart();
    void
    initialiserVar();
    int
    estInf(int i, int j);
    int
    estSup(int i, int j);
    int
    partionner(int * ordo, int deb, int fin);
    void
    quicksort(int * ordo, int deb, int fin);
    void
    Johnson(int * ordo, int m1, int m2, int s);

    void
    machineBound(int * cost);
    int
    calculBorne(int minCmax);
    int
    johnsonUB(int permutation[], int limit2, int ind);

    int nbbounds;

    void
    scheduleFront(int permutation[], int limite1, int limite2, int * idle);

    void
    setFlags(int permutation[], int limite1, int limite2);
    //
    void
    scheduleBack(int permutation[], int limite2, int * idle);
    int
    evalMakespan(int permutation[]);

    void
    boundChildren(int permutation[], int limite1, int limite2, int * costsBegin, int * costsEnd, int * prioBegin,
      int * prioEnd){ };

    void
    bornes_calculer(int permutation[], int limite1, int limite2, int * couts, int);
    void
    bornes_calculer(int permutation[], int limite1, int limite2);

    int
    evalSolution(int * permut);

    void
    myswap(int * a, int * b);
    void
    partial_cost(int permutation[], int limit1, int limit2, int * couts, int jobin, int here);

    void
    freeMem();
    ~bound_fsp_strong(){ };
};

#endif // ifndef BOUND_FSP_STRONG_H
