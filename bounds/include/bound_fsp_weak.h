#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#ifndef BOUND_FSP_WEAK_H
#define BOUND_FSP_WEAK_H

#include "bound_abstract.h"

class pbab;

struct bound_fsp_weak : public bound_abstract
{
    int branchingMode=1;

    int nbJob;
    int nbMachines;
    // int nbMaPairs;

    //static?
    int **tempsJob;      //PTM
    int  *minTempsArr;   //earliest date a job can start on machine k
    int  *minTempsDep;   //earliest completion time after release of last job on machine k
    // int  *tempsLag;

    int *front;
    int *back;
    int *remain;

	void init();
	void fillMinTimeArrDep();

    void scheduleFront(int *permut, int limit1, int limit2);
    void scheduleBack(int *permut, int limit2);
    void sumUnscheduled(int *permut, int limit1, int limit2);

	void computePartial(int *permut, int limit1, int limit2);
    int addFrontAndBound(int job, int &prio);
    int addBackAndBound(int job, int &prio);

	void boundChildren(int *permut, int limit1, int limit2, int* costsBegin, int* costsEnd, int* prioBegin, int* prioEnd);

    int evalSolution(int *permut);

    void bornes_calculer(int  permutation[],int limite1,int limite2,int *couts,int best);
	void bornes_calculer(int  permutation[],int limite1,int limite2);
    // void set_instance(instance_abstract *_instance);

    void partial_cost(int permutation[], int limit1, int limit2, int * couts,int jobin, int here);

	void freeMem();
 };

#endif
