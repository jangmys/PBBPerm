#include "../bounds/include/libbounds.h"
// #include "../bounds/headers/instance_flowshop.h"

#include "../common/include/misc.h"
// #include "../common/include/permutation.h"

#include "./include/fastNEH.h"
#include "./include/IG.h"
#include "./include/fspnhoods.h"

int main(int argc,char **argv)
{
    srand(time(NULL));

    instance_abstract * inst = NULL;
    inst = new instance_flowshop(argv[1]);

    printf("Hello\n");


    delete inst;

    // IG* ils=new IG(inst);
    //
    // fastNEH* h=new fastNEH();
    // h->set_instance(inst);
    // h->allocate();
    //
    // int *perm=(int*)malloc(inst->size*sizeof(int));
    // for(int i=0;i<inst->size;i++){
    //     perm[i]=i;
    // }
    //
    // int cost=1;
    //
    // gnomeSortByKeyInc(perm, h->sumPT, 0, h->nbJob-1);
    // h->runNEH(perm, cost);
    // printf("%d %d\n",h->evalMakespan(perm, h->nbJob),cost);
    //
    // memcpy(ils->perm, perm, h->nbJob*sizeof(int));
    //
    // printf("\n\n");
    // printf("%d\n",h->evalMakespan(ils->perm, h->nbJob));
    //
    // // ils->destruction(ils->perm, ils->removed, 4);
    // // ils->construction(ils->perm, ils->removed, 4);
    //
    // // ils->runIG();
    //
    // // printf("%d\n",h->evalMakespan(ils->perm, h->nbJob));
    //
    // // int arr[20]={4,11,12,18,7,5,16,8,3,6,9,1,13,17,0,19,10,2,15,14};
    //
    // int arr[50]={13,36,2,17,7,49,4,41,32,39,3,44,16,26,19,20,12,48,42,10,9,40,23,14,15,18,43, 31,25,27,45,0,35,38,46,24,29,6,1,30,22,5,47,21,28,33,8,34,37,11};
    //
    //
    // subproblem *start=new subproblem(ils->nbJob);
    //
    // for(int a=0;a<49;a++){
    //     for(int b=49;b>a;b--){
    //         memcpy(start->schedule, arr, 50*sizeof(int));
    //         // printf("%d %d %d \n",a,b,b-a);
    //         shuffle(&start->schedule[a],b-a-1);
    //
    //         // start->print();
    //         printf("%d %d %d %d :\t",a,b,b-a,h->evalMakespan(start->schedule, h->nbJob));
    //         //
    //         //
    //         start->limit1=a;
    //         start->limit2=b;
    //
    //         ils->runIG(start);
    //     }
    // }
    //
    //
    // // delete start;
    // //
    // //
    // // printf("%d\n",h->evalMakespan(perm, h->nbJob));
    //
    // free(perm);
    //
    // delete h;
    //
}
