//fast job insertion and removal
//
//cf. Deroussi,Gourgand,Norre "New effective neighborhoods for the permutation flow shop problem", 2006
#include <iostream>

#include <float.h>
#include <limits.h>
#include <random>

#include "../include/fastinsertremove.h"

fastInsertRemove::fastInsertRemove(instance_abstract * _instance)
{
    instance = _instance;

    (instance->data)->seekg(0);
    (instance->data)->clear();
    *(instance->data) >> nbJob;
    *(instance->data) >> nbMachines;

    PTM = (int **) malloc(nbMachines * sizeof(int *));
    for (int i = 0; i < nbMachines; i++) {
        PTM[i] = (int *) malloc(nbJob * sizeof(int));
    }

    for (int j = 0; j < nbMachines; j++)
        for (int i = 0; i < nbJob; i++)
            *(instance->data) >> PTM[j][i];

    head  = (int **) malloc(nbMachines * sizeof(int *));
    tail  = (int **) malloc(nbMachines * sizeof(int *));
    inser = (int **) malloc(nbMachines * sizeof(int *));

    for (int i = 0; i < nbMachines; i++) {
        head[i]  = (int *) malloc(nbJob * sizeof(int));
        tail[i]  = (int *) malloc(nbJob * sizeof(int));
        inser[i] = (int *) malloc(nbJob * sizeof(int));
    }

    sumPT=(int*)malloc(nbJob*sizeof(int));
    for (int i = 0; i < nbJob; i++){
        sumPT[i]=0;
        for (int j = 0; j < nbMachines; j++){
            sumPT[i]+=PTM[j][i];
        }
    }

    tabujobs=new tabulist(nbJob);
    tabupos=new tabulist(nbJob);
}

fastInsertRemove::~fastInsertRemove()
{
    for (int i = 0; i < nbMachines; i++) {
        free(head[i]);// =(int*)malloc(nbJob*sizeof(int));
        free(tail[i]);// =(int*)malloc(nbJob*sizeof(int));
        free(inser[i]);// =(int*)malloc(nbJob*sizeof(int));
        free(PTM[i]);
    }
    free(head);
    free(tail);
    free(inser);
    free(PTM);

    free(sumPT);

    delete tabupos;
    delete tabujobs;
}

int fastInsertRemove::computeHeads(int* const perm, int len)
{
    //machine 1
    head[0][0]=PTM[0][perm[0]];
    for(int j=1;j<len;j++){
        head[0][j]=head[0][j-1]+PTM[0][perm[j]];
    }

    //job 1
    int job=perm[0];
    for(int k=1;k<nbMachines;k++){
        head[k][0]=head[k-1][0]+PTM[k][job];
    }
    //rest
    for(int j=1;j<len;j++){
        int job=perm[j];
        for(int k=1;k<nbMachines;k++){
            head[k][j]=std::max(head[k-1][j],head[k][j-1])+PTM[k][job];
        }
    }

    return head[nbMachines-1][len-1];
}

// compute tails
void fastInsertRemove::computeTails(int *perm, int len)
{
    //#machine M
    tail[nbMachines-1][len-1]=PTM[nbMachines-1][perm[len-1]];
    for(int j=len-2;j>=0;j--){
        tail[nbMachines-1][j]=tail[nbMachines-1][j+1]+PTM[nbMachines-1][perm[j]];
    }
    //#job len-1
    int job=perm[len-1];
    for(int k=nbMachines-2;k>=0;k--){
        tail[k][len-1]=tail[k+1][len-1]+PTM[k][job];
    }
    for(int j=len-2;j>=0;j--){
        int job=perm[j];
        for(int k=nbMachines-2;k>=0;k--){
            tail[k][j]=\
            std::max(tail[k+1][j],tail[k][j+1])+\
            PTM[k][job];
        }
    }
}

void fastInsertRemove::computeInser(int *perm, int len, int job)
{
//  #insert before (position 1)
    inser[0][0]=PTM[0][job];
    for(int k=1;k<nbMachines;k++){
        inser[k][0]=inser[k-1][0]+PTM[k][job];
    }
//  #insert 2nd pos to last
    for(int j=1;j<=len;j++){
        inser[0][j]=head[0][j-1]+PTM[0][job];
        for(int k=1;k<nbMachines;k++){
            inser[k][j]=std::max(inser[k-1][j],head[k][j-1])+PTM[k][job];
        }
    }
}


void fastInsertRemove::computeInserBlock(int *perm, int len, int * block, int bl)
{
    // printf("blocksize: %d\n",bl);
    std::vector<int> tmp(nbMachines, 0);
    int job;

//  #insert before (position 1)
    for(int b=0;b<bl;b++){
        job=block[b];

        tmp[0]+=PTM[0][job];
        for(int k=1;k<nbMachines;k++){
            tmp[k]=std::max(tmp[k-1],tmp[k])+PTM[k][job];
        }
    }
    for(int k=0;k<nbMachines;k++){
        inser[k][0]=tmp[k];
    }
    //============================

    //  #insert 2nd pos to last
    for(int j=1;j<=len;j++){
        for(int k=0;k<nbMachines;k++)tmp[k]=0;


        for(int k=0;k<nbMachines;k++){
            tmp[k]=head[k][j-1];
        }

        for(int b=0;b<bl;b++){
            job=block[b];
            tmp[0]+=PTM[0][job];
            for(int k=1;k<nbMachines;k++){
                tmp[k]=std::max(tmp[k-1],tmp[k])+PTM[k][job];
            }
        }

        for(int k=0;k<nbMachines;k++){
            inser[k][j]=tmp[k];
        }
    }

    // for(int k=0;k<nbMachines;k++){
    //     for(int j=0;j<=len;j++){
    //         printf("%4d ",inser[k][j]);
    //     }
    //     printf("\n");
    // }
}




//get len+1 makespans obtained by inserting "job" into positions 0,1,...,len of partial permuation of length len
//returns cmax before job insertion
int fastInsertRemove::insertMakespans(int* perm, int len, int job, int* makespans)
{
    // printf("Len %d\n",len); fflush(stdout);
    //
    // for(int j=0;j<len-1;j++){
    //     printf("%3d ",perm[j]);
    // }
    // printf("\n");

    computeHeads(perm, len);
    computeTails(perm, len);
    computeInser(perm, len, job);

    for(int i=0;i<=len;i++){
        makespans[i]=0;
        for(int j=0;j<nbMachines;j++){
            int val=inser[j][i]+tail[j][i];
            makespans[i]=std::max(makespans[i],val);
        }
    }

    // for(int i=0;i<=len;i++){
    //     printf("%d ",makespans[i]);
    // }
    // printf("\n");

    return head[nbMachines-1][len-1];
    // return 0;
}


//get len+1 makespans obtained by inserting "job" into positions 0,1,...,len of partial permuation of length len
//returns cmax before job insertion
int fastInsertRemove::insertBlockMakespans(int* perm, int len, int* block, int bl, int* makespans)
{
    computeHeads(perm, len);
    computeTails(perm, len);
    computeInserBlock(perm, len, block, bl);

    // printf("PERM / BLOCK\n");
    // for(int i=0;i<len;i++){
    //     printf("%2d ",perm[i]);
    // }
    // printf("\n");
    // for(int i=0;i<bl;i++){
    //     printf("%2d ",block[i]);
    // }
    // printf("\n");

    for(int i=0;i<len;i++){
        makespans[i]=0;
        for(int j=0;j<nbMachines;j++){
            int val=inser[j][i]+tail[j][i];
            makespans[i]=std::max(makespans[i],val);
        }
    }
    for(int j=0;j<nbMachines;j++){
        makespans[len]=std::max(makespans[len],inser[j][len]);
    }

    // for(int i=0;i<=len;i++){
    //     printf("%d ",makespans[i]);
    // }
    // printf("\n");


    return head[nbMachines-1][len-1];//return makespan before insertion
    // return 0;
}

//get len makespans obtained by removing job at position 0,...,len-1 from partial permutation of length len
//returns cmax before removal
int fastInsertRemove::removeMakespans(int* perm, int len, int* makespans)
{
    computeHeads(perm, len);
    computeTails(perm, len);

    int i=0;

    //remove first job (i=0)
    int maxi=0;
    for(int j=0;j<nbMachines;j++){
        maxi=std::max(maxi,tail[j][1]);
    }
    makespans[i]=maxi;

    for(i=1;i<len-1;i++){
        maxi=0;
        for(int j=0;j<nbMachines;j++){
            maxi=std::max(maxi,head[j][i-1]+tail[j][i+1]);
        }
        makespans[i]=maxi;
    }

    i=len-1;
    maxi=0;
    for(int j=0;j<nbMachines;j++){
        maxi=std::max(maxi,head[j][i-1]);
    }
    makespans[i]=maxi;

    return head[nbMachines-1][len-1];
}

//insert job at position pos in part. permutation of length len
void fastInsertRemove::insert(int* perm, int &len, int pos, int job)
{
    if(len>=nbJob){
        for(int i=0;i<nbJob;i++)printf("%d ",perm[i]);
        printf("\n");
        printf("permutation full %d %d\n",len,nbJob);
    }

    for(int p=len;p>pos;p--){
        perm[p]=perm[p-1];
    }
    perm[pos]=job;
    len++;
}

//insert block of length bl at position pos in partial permutation of length len
void fastInsertRemove::blockInsert(int* perm, int &len, int pos, int*block, int bl)
{
    if(len>=nbJob){
        for(int i=0;i<nbJob;i++)printf("%d ",perm[i]);
        printf("\n");
        printf("permutation full %d %d\n",len,nbJob);
    }

    int job;
    for(int b=0;b<bl;b++)
    {
        job=block[b];
        for(int p=len;p>pos+b;p--){
            perm[p]=perm[p-1];
        }
        perm[pos+b]=job;
        len++;
    }
}

int fastInsertRemove::remove(int *perm, int &len, const int pos){
    int rjob=perm[pos];

    for(int p=pos;p<len-1;p++){
        perm[p]=perm[p+1];
    }
    perm[len-1]=0;
    len--;

    return rjob;
}

//insert in position which gives best cmax
int fastInsertRemove::bestInsert(int *perm, int &len, int job, int &cmax)
{
    int *makespans=(int*)malloc((len+1)*sizeof(int));

    //makespans obtained when inserting job at positions 0,...,len
    insertMakespans(perm, len, job, makespans);

    int minpos=-1;
    int mini=INT_MAX;
    for(int i=0;i<=len;i++){
        if(tabupos->isTabu(i))continue;

        if(makespans[i]<mini){
            mini=makespans[i];
            minpos=i;
        }
    }

    // for(int i=0;i<=len;i++){
    //     int job=perm[i];
    //     printf("%2.6f\n",(float)oldcmax/(float)(makespans[i]-oldcmax));
    // }
    // printf("\n\n");

    // for(int i=0;i<=len;i++){
    //     int job=perm[i];
    //     printf("%2.2f ",exp((float)(oldcmax-makespans[i])/(float)sumPT[job]));
    // }
    // printf(" = %d = \n",minpos);

    if(minpos>=0){
        insert(perm,len,minpos,job);
    }
    cmax = mini;
    //
    free(makespans);
    //
    return minpos;
}

//insert in position which gives best cmax
int fastInsertRemove::bestInsert2(int *perm, int &len, int job, int &cmax)
{
    int *makespans=(int*)malloc((len+1)*sizeof(int));

    //makespans obtained when inserting job at positions 0,...,len
    int oldcmax=insertMakespans(perm, len, job, makespans);

    std::vector<float> weights;
    for(int i=0; i<=len; ++i) {
        float val=(float)oldcmax/(float)(makespans[i]-oldcmax);
        if(tabupos->isTabu(i))val=0.0;
        weights.push_back(val);
    }

    std::default_random_engine generator;
    std::discrete_distribution<int> d1(weights.begin(), weights.end());

    int number = d1(generator);

    insert(perm,len,number,job);

    cmax = makespans[number];
    //
    free(makespans);
    //
    return number;
}

//insert in position which gives best cmax
int fastInsertRemove::bestBlockInsert(int *perm, int len, int* block, int bl)
{
    //block can be inserted at len+1 positions
    int *makespans=(int*)malloc((len+1)*sizeof(int));

    insertBlockMakespans(perm, len, block, bl, makespans);

    int minpos=-1;
    int mini=INT_MAX;
    for(int i=0;i<=len;i++){
        if(tabupos->isTabu(i))continue;

        if(makespans[i]<mini){
            mini=makespans[i];
            minpos=i;
        }
    }

    if(minpos>=0){
        blockInsert(perm, len, minpos, block, bl);
    }

    // cmax = mini;
    free(makespans);

    return mini;//return makespan
}



//remove least well inserted job from perm
//return removed job in remjob
//return position of removed job
int fastInsertRemove::bestRemove(int *perm,int &len,int& remjob, int &cmax)
{
    int *makespans=(int*)malloc(len*sizeof(int));

    int oldcmax=removeMakespans(perm, len, makespans);

    int bestpos=-1;
    float maxi=FLT_MIN;

    for(int i=0;i<len;i++){
        int job=perm[i];

        if(tabujobs->isTabu(job))continue;

        float val=(float)(oldcmax-makespans[i])/(float)sumPT[job];

        // printf("%d %d %d %f\n",job,oldcmax-makespans[i],sumPT[job],val);

        if(val>maxi){
            maxi=val;
            bestpos=i;
        }
    }
    // printf("\n\n");

    remjob=-1;
    if(bestpos>=0){
        remjob=remove(perm,len,bestpos);
    }
    cmax=makespans[bestpos];

    free(makespans);

    return bestpos;
}


//remove least well inserted job from perm
//return removed job in remjob
//return position of removed job
int fastInsertRemove::bestRemove2(int *perm,int &len,int& remjob, int &cmax)
{
    int *makespans=(int*)malloc(len*sizeof(int));

    int oldcmax=removeMakespans(perm, len, makespans);

    std::vector<float> weights;
    for(int i=0; i<len; ++i) {
        int job=perm[i];
        float val=(float)(oldcmax-makespans[i])/(float)sumPT[job];
        if(tabujobs->isTabu(job))val=0.0;
        weights.push_back(val*val);
    }

    // for(auto i:weights)
    //     std::cout<<i<<" ";
    // std::cout<<std::endl;

    std::default_random_engine generator;
    std::discrete_distribution<int> d1(weights.begin(), weights.end());

    int number = d1(generator);

    // std::cout<<" === "<<number<<std::endl;

    cmax=makespans[number];

    remjob=remove(perm,len,number);

    free(makespans);
    return number;
}
