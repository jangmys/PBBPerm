#include "../../bounds/include/libbounds.h"

#include "../../common/include/misc.h"
#include "../include/fastNEH.h"
#include "../include/IG.h"
#include "../include/fspnhoods.h"

#include <iostream>
#include <random>

void printp(int *arr, int N)
{
    for(int i=0;i<N;i++){
        printf("%3d ",arr[i]);
    }
    printf("\n");
}


int main(int argc,char **argv)
{
    srand(time(NULL));

    instance_abstract * inst = NULL;
    inst = new instance_flowshop(argv[1]);
    IG* ils=new IG(inst);

    if(argc>2){
        ils->acceptanceParameter=atof(argv[2]);
    }
    if(argc>3){
        ils->destructStrength=atoi(argv[3]);
    }

    int N=inst->size;

    int ta51[50]={16,30,36,38,26,19,42,33,41,0,34,46,31,35,28,13,37,32,45,27,14,5,4,6,7,15,48, 44,21,29,10,20,1,11,22,12,23,9,25,40,43,2,17,3,47,18,8,39,24,49};
//16  30  36  38  19  26  42  33  41   0  34  46  31  37  35  28  13  32  23  14  45  10   4  27   6  44  21  48  29   9  20   7  15   1  11  22  12  25  40  43   5  17   2   3  47  18   8  39  24  49


    int ta60[50]={11,18,7,21,13,49,0,8,2,14,1,39,10,35,34,17,31,9,24,33,12,46,36,30,15, 19,44,20,40,4,26,28,41,43,22,27,16,37,6,23,38,5,25,48,45,42,29,47,3};

    int ta89[100]={42,88,59,14,73,40,46,89,49,45,62,55,61,3,41,94,79,11,96,32,81,18,83,93,  63,80,36,38,78,6,70,9,74,76,56,23,77,64,43,58,25,22,75,53,44,67,12,82,15,7,57,  60,86,26,2,10,13,85,20,29,21,31,87,47,1,24,37,68,50,54,65,72,99,69,52,28,84,39,  98,33,0,17,91,16,71,35,19,8,66,5,34,92,30,27,95,4,51,48,90,97};

    //================================================

    subproblem *s=new subproblem(N);
    subproblem *s2=new subproblem(N);

    for(int i=0;i<N;i++){
        s->schedule[i]=i;
        s2->schedule[i]=i;
    }
    s->limit1=-1;
    s->limit2=N;
    s2->limit1=-1;
    s2->limit2=N;

    ils->igiter=100;

    // int c=ils->vbih(s,s2);
    // printf("cost %d\n",c);
    // return 0;

    // s->shuffle();

    printf("cost: %d \n",ils->neh->evalMakespan(s->schedule, N));

    // if(!s->isPermutation())
    // {
    //     printf("ERRRROR\n");
    //     s->print();
    // }

    //database...
    int M=1;
    // int *solutions=(int*)malloc(M*N*sizeof(int));
    // int *posFreq=(int*)malloc(N*N*sizeof(int));
    // int *fwdFreq=(int*)malloc(N*N*sizeof(int));
    // int *bwdFreq=(int*)malloc(N*N*sizeof(int));


    struct timespec startt,endt;

    for(int l=0;l<M;l++){
        // printf("---\n");
        // shuffle(s->schedule, N);
        clock_gettime(CLOCK_MONOTONIC, &startt);
        int c=ils->runIG(s);
        clock_gettime(CLOCK_MONOTONIC, &endt);

        printf("cost %d\t",c);
        s->print();
        std::cout<<(endt.tv_sec-startt.tv_sec)+(endt.tv_nsec-startt.tv_nsec)/1e9<<std::endl;
    }


    delete ils;
    delete inst;

    delete s;
    delete s2;
}
