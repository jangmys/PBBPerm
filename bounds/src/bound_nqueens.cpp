#include <algorithm>
#include <utility>
#include <climits>
#include <string.h>
#include <stdio.h>

#include "instance_abstract.h"
#include "bound_abstract.h"
#include "bound_nqueens.h"

// #include "../headers/pbab.h"

// uncomment this to perform FULL enumeration of 10! with './bb -z p=nq,10'
#define ENUM_ALL

void
bound_nqueens::set_instance(instance_abstract * _instance)
{
    instance = _instance;
    // init();
}

void
bound_nqueens::init()
{
    (instance->data)->clear();
    (instance->data)->seekg(0);
    *(instance->data) >> size;

    // branchingMode = _branchingMode;
}

void
bound_nqueens::bornes_calculer(int permutation[], int limit1, int limit2, int * couts, int best)
{
    int col;
    int vsPos = 0;
    int delta = 0;
    int row   = 0;

    couts[0] = 0;

    #ifdef ENUM_ALL
    return;

    #endif

    int conflicts = 0;

    // placing a queen in [row=limit1,col]
    col = permutation[limit1];

    while (row < limit1) {
        vsPos = permutation[row];
        delta = limit1 - row;
        // printf("%d job %d vs %d\n",delta,col,vsPos);
        if (col - delta == vsPos || col + delta == vsPos) {
            conflicts++;// count nb conflicts
            break;// stop at first found conflict
        }
        row++;
    }

    couts[0] = (conflicts > 0) ? INT_MAX : 0;
}

// not passing "class subproblem"
void
bound_nqueens::boundChildren(int * schedule, const int limit1, const int limit2, int * costsBegin, int * costsEnd,
  int * prioBegin, int * prioEnd)
{
    int _limit1 = limit1 + 1;
    int _limit2 = limit2;
    int fillPos = _limit1;

    int costs[2];

    if (branchingMode < 0) {
        memset(costsBegin, 0, size * sizeof(int));
        for (int i = limit1 + 1; i < limit2; i++) {
            int job = schedule[i];

            std::swap(schedule[fillPos], schedule[i]);
            bornes_calculer(schedule, _limit1, _limit2, costs, 42);
            costsBegin[job] = costs[0];
            std::swap(schedule[fillPos], schedule[i]);
            // printf("job %d : %d\n",job,costs[0]);fflush(stdout);
        }
    } else {
        printf("bound_nqueens: only forward branching implemented\n");
        printf("set adaptiveBranching to -1 !\n");
        exit(-1);
    }
}

int
bound_nqueens::evalSolution(int * permutation)
{
    int conflicts = 0;

    int col;
    int vsPos = 0;
    int delta = 0;
    int row   = 0;

    #ifdef ENUM_ALL
    return 0;

    #endif

    for (int l = size - 1; l >= 0; l--) {
        col = permutation[l];
        row = 0;
        while (row < l) {
            vsPos = permutation[row];
            delta = l - row;
            // printf("%d job %d vs %d\n",delta,col,vsPos);
            if (col - delta == vsPos || col + delta == vsPos) {
                conflicts++;// infeasible
                break;
            }
            row++;
        }
    }
    // printf("#conflicts %d\n",conflicts);

    return conflicts;
}

// lazy...........
bound_nqueens::~bound_nqueens(){ }

void
bound_nqueens::freeMem(){ }
