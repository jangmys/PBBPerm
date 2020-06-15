//INCLUDE INSTANCES + BOUNDS
#include "libbounds.h"

int main(int argc,char **argv){
    instance_abstract * inst = NULL;
    bound_abstract *bound=NULL;
    bound_abstract *bound2=NULL;

    switch(argv[1][0])//DIFFERENT PROBLEMS...
    {
        case 'f': //FLOWSHOP
        {
            switch (argv[2][0]) {//different benchmark sets
                case 't': {
                    inst = new instance_flowshop(argv[2]);
                    break;
                }
                case 'V': {
                    inst = new instance_vrf(argv[2]);
                    break;
                }
            }

            //set bound1
            bound_fsp_weak* bd=new bound_fsp_weak();
            bd->set_instance(inst);
            bd->init();
            bd->branchingMode = atoi(argv[3]);
            bound=bd;

            //set bound2
            bound_fsp_strong* bd2=new bound_fsp_strong();
            bd2->set_instance(inst);
            bd2->init();
            bd2->branchingMode = atoi(argv[3]);
            bd2->earlyExit = atoi(argv[4]);
            bd2->machinePairs = atoi(argv[5]);
            bound2=bd2;

            break;
        }
        case 'n': //NQUEENS
        {
            inst = new instance_nqueens(argv[2]);

            bound_nqueens* bd=new bound_nqueens();
            bd->set_instance(inst);
            bd->init();

            bound=bd;

            break;
        }
        // add other problems...
        case 't': //TEST
        {
            // inst = new instance_nqueens(argv[2]);
            break;
        }
    }

    int costs[2];

    //a permutation
    int *perm=(int*)malloc(inst->size*sizeof(int));
    for(int i=0;i<inst->size;i++){
        perm[i]=i;
    }

    //evaluate objective function
    costs[0] = bound->evalSolution(perm);
    printf("Makespan: %d\n",costs[0]);

    //empty partial schedules
    int l1=-1;
    int l2=inst->size;

    //LB
    bound->bornes_calculer(perm, l1, l2, costs, 99999);
    printf("LB1: %d\n",costs[0]);

    //LB2
    if(bound2){
        bound2->bornes_calculer(perm, l1, l2, costs, 99999);
        printf("LB2: %d\n",costs[0]);
    }

    l1=0; //fix first job
    l2=inst->size-1; //fix last job
    bound->bornes_calculer(perm, l1, l2, costs, 99999);
    printf("LB1: %d\n",costs[0]);

    if(bound2){
        bound2->bornes_calculer(perm, l1, l2, costs, 99999);
        printf("LB2: %d\n",costs[0]);
    }



    free(perm);
}
