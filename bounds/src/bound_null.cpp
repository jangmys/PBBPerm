#include <limits.h>
#include <string.h>

#include "instance_abstract.h"
#include "bound_abstract.h"
#include "bound_null.h"


void
bound_null::set_instance(instance_abstract * _instance)
{
    instance = _instance;
    // init();
}

void
bound_null::init(const int _branchingMode,const int _earlyExit,const int _machinePairs)
{
    (instance->data)->clear();
    (instance->data)->seekg(0);
    *(instance->data) >> size;

	branchingMode = _branchingMode;

    // instance data... some linearized 2D array
    costMatrix = (int *) malloc(size * size * sizeof(int));

    // some cost matrix...
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            costMatrix[i * size + j] = (i+j==size)?0:1; //rand() % 100;
        }
    }
}

/*
 * limit1 : the last job of the partial schedule in front
 * limit2 : the first job of the partial schedule in back
 *
 * Example:
 * permutation = [0 4 2 1 3]
 * limit1 = 0
 * limit2 = 4
 * encodes subproblem 0/ {421} /3
 * where
 * 0 is scheduled in the beginning
 * 3 is scheduled in the end
 * 4,2,1 are unscheduled
 *
 * input: permutation, limit1, limit2,
 * (best) is optional current upper bound
 * output: couts[] --> couts[0] is the lower bound !!
 */
void
bound_null::bornes_calculer(int permutation[], int limit1, int limit2, int * couts, int best)
{
    int lb = 0;

    // partial cost
    for (int i = 0; i < limit1; i++) {
        int from = permutation[i];
        int to = permutation[i+1];

        lb += costMatrix[from*size + to];
    }
    // ...lb is a naive lb for TSP

    // this is the return value
    couts[0] = lb;
}

void
bound_null::boundChildren(int *schedule, int limit1, int limit2, int * costsBegin, int * costsEnd, int* prioBegin, int* prioEnd)
{
    int _limit1 = limit1 + 1;
    int _limit2 = limit2;

    //needed for "historical" reasons
    int costs[2];

    //position to try in permutation
    int fillPos = _limit1;

    if (branchingMode < 0) {
        //reset costs... (not sure... better be safe)
        memset(costsBegin, 0, size*sizeof(int));

        //LOOP over free elements in permutation
        for (int i = limit1 + 1; i < limit2; i++) {

            //temporarily generate child subproblem
            std::swap(schedule[fillPos], schedule[i]);
            //compute bound
            bornes_calculer(schedule, _limit1, _limit2, costs, INT_MAX);
            //store in cost array...
            costsBegin[ schedule[i] ] = costs[0];
            //revert previous swap
            std::swap(schedule[fillPos], schedule[i]);
        }
    } else  {
        //if elements fixed at both ends of permutation
        printf("not implemented\n");
        exit(-1);
    }
}

int
bound_null::evalSolution(int * permutation){
    int cost = 0;

    int from,to;
    // partial cost
    for (int i = 0; i < size-1; i++) {
        from = permutation[i];
        to = permutation[i+1];

        cost += costMatrix[from*size + to];
    }

    from = permutation[size-1];
    to = permutation[0];

    cost += costMatrix[from*size + to];

    return cost;
}

// lazy...........
bound_null::~bound_null(){ }

void
bound_null::freeMem(){ }
