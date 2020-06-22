#include <limits.h>

#include "../../common/include/misc.h"

#include "../include/fastNEH.h"
#include "../include/fastinsertremove.h"

fastNEH::fastNEH(instance_abstract* _inst)
{
    set_instance(_inst);
    allocate();
}

fastNEH::~fastNEH()
{
    free(PTM);
    free(sumPT);

    for(int i=0; i<nbMachines; i++){
        free(head[i]);
        free(tail[i]);
        free(inser[i]);
    }
    free(head);
    free(tail);
    free(inser);
}

void fastNEH::set_instance(instance_abstract*_instance)
{
    instance=_instance;

    (instance->data)->seekg (0);
    (instance->data)->clear ();
    *(instance->data)>>nbJob;
    *(instance->data)>>nbMachines;

    PTM=(int *)malloc(nbMachines*nbJob*sizeof(int)) ;

    for (int j = 0; j < nbMachines; j++)
        for (int i = 0; i < nbJob; i++)
            *(instance->data) >> PTM[j*nbJob+i];

    sumPT=(int*)malloc(nbJob*sizeof(int));
    for (int i = 0; i < nbJob; i++){
        sumPT[i]=0;
        for (int j = 0; j < nbMachines; j++){
            sumPT[i]-=PTM[j*nbJob+i];
        }
    }
}

void fastNEH::allocate()
{
    head=(int **)malloc(nbMachines*sizeof(int *)) ;
    tail=(int **)malloc(nbMachines*sizeof(int *)) ;
    inser=(int **)malloc(nbMachines*sizeof(int *)) ;

    for(int i=0; i<nbMachines; i++){
        head[i]=(int*)malloc(nbJob*sizeof(int));
        tail[i]=(int*)malloc(nbJob*sizeof(int));
        inser[i]=(int*)malloc(nbJob*sizeof(int));
    }
}

int fastNEH::evalMakespan(int *permut, int len){
    std::vector<int>tmp(nbMachines,0);

    for(int i=0;i<len;i++){
        int job=permut[i];
        tmp[0]+=PTM[job];
        for(int j=1;j<nbMachines;j++){
            tmp[j]=std::max(tmp[j-1],tmp[j])+PTM[j*nbJob+job];
        }
    }

    return tmp[nbMachines-1];
}


void fastNEH::computeHeads(int *perm, int len)
{
    //machine 1
    head[0][0]=PTM[perm[0]];
    for(int j=1;j<len;j++){
        head[0][j]=head[0][j-1]+PTM[perm[j]];
    }
    //job 1
    int job=perm[0];
    for(int k=1;k<nbMachines;k++){
        head[k][0]=head[k-1][0]+PTM[k*nbJob+job];
    }
    //rest
    for(int j=1;j<len;j++){
        int job=perm[j];
        for(int k=1;k<nbMachines;k++){
            head[k][j]=std::max(head[k-1][j],head[k][j-1])+PTM[k*nbJob+job];
        }
    }
}


// compute tails
void fastNEH::computeTails(int *perm, int len)
{
    //#machine M
    tail[nbMachines-1][len-1]=PTM[(nbMachines-1)*nbJob+perm[len-1]];
    for(int j=len-2;j>=0;j--){
        tail[nbMachines-1][j]=tail[nbMachines-1][j+1]+PTM[(nbMachines-1)*nbJob+perm[j]];
    }
    //#job len-1
    int job=perm[len-1];
    for(int k=nbMachines-2;k>=0;k--){
        tail[k][len-1]=tail[k+1][len-1]+PTM[k*nbJob+job];
    }
    for(int j=len-2;j>=0;j--){
        int job=perm[j];
        for(int k=nbMachines-2;k>=0;k--){
            tail[k][j]=\
            std::max(tail[k+1][j],tail[k][j+1])+\
            PTM[k*nbJob+job];
        }
    }
}

void fastNEH::insertJob(int *perm, int len, int job)
{
    // printf("job %3d to 0-%3d\n",job,len);

//  #insert before (position 1)
    inser[0][0]=PTM[job];
    for(int k=1;k<nbMachines;k++){
        inser[k][0]=inser[k-1][0]+PTM[k*nbJob+job];
    }
//  #insert 2nd pos to last
    for(int j=1;j<=len;j++){
        inser[0][j]=head[0][j-1]+PTM[job];
        for(int k=1;k<nbMachines;k++){
            inser[k][j]=std::max(inser[k-1][j],head[k][j-1])+PTM[k*nbJob+job];
        }
    }
    // for(int i=0; i<nbMachines; i++){
    //     for(int j=0;j<nbJob;j++){
    //         printf("%3d ",inser[i][j]);
    //     }
    //     printf("\n");
    // }
    // printf("===\n");
}

void fastNEH::findBestPos(int &cost, int &bestpos, int len)
{
    std::vector<int>tmp(len+1,0);

    // int *tmp=(int*)calloc(len+1,sizeof(int));
    for(int i=0;i<=len;i++){
        int maxi=0;
        for(int j=0;j<nbMachines;j++){
            int a=inser[j][i]+tail[j][i];
            maxi=(a>maxi)?a:maxi;
        }
        tmp[i]=maxi;
    }

    // for(int i=0;i<=len;i++){
    //     printf("%d ",tmp[i]);
    // }

    cost=INT_MAX;
    for(int i=0;i<=len;i++){
        if(tmp[i]<cost){
            bestpos=i;
            cost=tmp[i];
        }
    }

    // free(tmp);
}


void fastNEH::findBestPos(int &cost, int &bestpos, int len, int a, int b)
{
    std::vector<int>tmp(len+1,0);

    // int *tmp=(int*)calloc(len+1,sizeof(int));

    for(int i=0;i<=len;i++){
        int maxi=0;
        for(int j=0;j<nbMachines;j++){
            int a=inser[j][i]+tail[j][i];
            maxi=(a>maxi)?a:maxi;
        }
        tmp[i]=maxi;
    }

    cost=INT_MAX;
    for(int i=a;i<b;i++){
        if(tmp[i]<cost){
            bestpos=i;
            cost=tmp[i];
        }
    }
}


void fastNEH::insertJobInBestPosition(int *perm,int k)
{
    int cost,bestpos;

    computeHeads(perm,k); //compute heads
    computeTails(perm,k); //compute tails
    insertJob(perm, k, perm[k]);
    findBestPos(cost,bestpos,k);

    int tmp=perm[k];
    for(int p=k;p>bestpos;p--){
        perm[p]=perm[p-1];
    }
    perm[bestpos]=tmp;
}
    //     cost,bestpos = findmin(step4(k))
    //     inser!(perm,bestpos,k+1)
    //     return perm
    // end

void fastNEH::initialSort(int* perm){
    gnomeSortByKeyInc(perm, sumPT, 0, nbJob-1);
}

void fastNEH::runNEH(int* perm, int &cost){
    // gnomeSortByKeyInc(perm, sumPT, 0, nbJob-1);

    int c1=evalMakespan(perm, 2);
    swap(&perm[0],&perm[1]);
    int c2=evalMakespan(perm, 2);
    if(c1<c2)swap(&perm[0],&perm[1]);

    for(int k=2;k<nbJob;k++){
        insertJobInBestPosition(perm,k);
    }

    cost=evalMakespan(perm,nbJob);
}

void fastNEH::insertLS(int *perm)
{
    // int cost,bestpos;
    //
    // computeHeads(perm, nbJob);
    // computeTails(perm, nbJob);
    //
    // for(int i=0;i<nbJob;i++){
    //     insertJob(perm, nbJob-1, perm[i]);
    //     findBestPos(cost,bestpos,nbJob);
    //     printf("\n === %d %d %d\n",i,bestpos,cost);
    // }
}
