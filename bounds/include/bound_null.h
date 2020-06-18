#ifndef BOUND_NULL_H
#define BOUND_NULL_H

#include "instance_abstract.h"
#include "bound_abstract.h"

struct bound_null : public bound_abstract
{
	int branchingMode=0;

	int size;

	int *costMatrix;

	// void init(const int _branchingMode);
    void init();
	void set_instance(instance_abstract *_instance);
	void boundChildren(int *permut, int limit1, int limit2, int* costsBegin, int* costsEnd, int* prioBegin, int* prioEnd);

	void bornes_calculer(int  permutation[], int  limite1, int  limite2, int *couts, int);

	void bornes_calculer(int  permutation[], int  limite1, int  limite2){};
  	int evalSolution(int *permutation);

	void freeMem();
	~bound_null();
};

#endif
