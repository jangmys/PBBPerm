#include <limits.h>
#include <string.h>
#include <cmath>

#include "bound_abstract.h"
#include "instance_abstract.h"
#include "bound_fsp_weak.h"

/*INITIALIZATIONS*/
//==============================================================

// void bound_fsp_weak::set_instance(instance_abstract*_instance)
// {
//     instance=_instance;
// }

void bound_fsp_weak::init()
{
    //get instance parameters (N/M)
    (instance->data)->seekg (0);
    (instance->data)->clear ();
    *(instance->data)>>nbJob;
    *(instance->data)>>nbMachines;

    //allocate PTM and get processing times
    tempsJob=(int **)malloc(nbMachines*sizeof(int *)) ;
    for(int i=0; i<nbMachines; i++){
        posix_memalign((void**)&tempsJob[i], 64, nbJob*sizeof(int));
    }
    for (int j = 0; j < nbMachines; j++)
        for (int i = 0; i < nbJob; i++)
            *(instance->data) >> tempsJob[j][i];

    //for each machine k, earliest date a job can start
    posix_memalign((void**)&minTempsArr, 64, nbMachines*sizeof(int));
    //for each machine k, minimum time between release of last job on machine k and the end of operations on the last machine
    posix_memalign((void**)&minTempsDep, 64, nbMachines*sizeof(int));
    fillMinTimeArrDep();

    front=(int*)malloc(nbMachines*sizeof(int));
    back=(int*)malloc(nbMachines*sizeof(int));
    remain=(int*)malloc(nbMachines*sizeof(int));
}

void
bound_fsp_weak::freeMem(){
    free(tempsJob);
    free(minTempsDep);
    free(minTempsArr);

    free(front);
    free(back);
    free(remain);
}


/*compute lower bounds*/
//==============================================================

void bound_fsp_weak::scheduleFront(int *permut, int limit1,int limit2)
{
    int job;

    //no jobs scheduled in front
    if(limit1==-1){
        for(int i=0;i<nbMachines;i++)
            front[i]=minTempsArr[i];//minRelease[i];
        return;
    }

    for(int i=0;i<nbMachines;i++)
        front[i]=0;

    for(int i=0;i<limit1+1;i++){
        job=permut[i];
        front[0]+=tempsJob[0][job];
        for(int j=1;j<nbMachines;j++){
            front[j]=std::max(front[j-1],front[j])+tempsJob[j][job];
        }
    }
}

void bound_fsp_weak::scheduleBack(int *permut, int limit2)
{
    int job;

    if(limit2==nbJob){
        for(int i=0;i<nbMachines;i++)
            back[i]=minTempsDep[i];
        return;
    }

    for (int j = nbMachines - 1; j >= 0; j--)
        back[j]=0;

    //reverse schedule (allowing incremental update)
    for (int k = nbJob - 1; k>=limit2; k--) {
        job=permut[k];
        back[nbMachines-1]+=tempsJob[nbMachines-1][job];//ptm[(nbMachines-1) * nbJob + job];
        for (int j = nbMachines - 2; j >= 0; j--){
            back[j]=std::max(back[j],back[j+1])+tempsJob[j][job];
        }
    }
}

void bound_fsp_weak::sumUnscheduled(int *permut, int limit1, int limit2)
{
    int job;

    for (int j = 0; j < nbMachines; j++)remain[j]=0;

    for (int k = limit1+1; k<limit2; k++) {
        job=permut[k];
        for (int j = 0; j < nbMachines; j++){
            remain[j]+=tempsJob[j][job];//ptm[j * nbJob + job];
        }
    }
}

void bound_fsp_weak::computePartial(int* schedule,int limit1,int limit2){
    scheduleFront(schedule,limit1,limit2);//set front[]
    scheduleBack(schedule,limit2);//set back[]
    sumUnscheduled(schedule,limit1,limit2);//set unscheduled[]
}

//m*(3 add+2 max)
int bound_fsp_weak::addFrontAndBound(int job, int &prio){
    int lb=0;
    int tmp0,tmp1,cmax;
    tmp0=front[0]+tempsJob[0][job];
    lb=front[0]+remain[0]+back[0];

    prio=0;//sum of idle time

    for(int j=1;j<nbMachines;j++){
        // prio=std::max(prio,std::abs(tmp0-front[j]));
        // prio+=(tmp0-front[j]);
        prio+=std::abs(tmp0-front[j]);
        // prio+=std::max(0,tmp0-front[j]);//sum idle time


        tmp1=std::max(front[j],tmp0);//+ptm[j*nbJob+job];

        cmax=tmp1+remain[j]+back[j];
        tmp0=tmp1+tempsJob[j][job];
        if(cmax>lb){
            lb=cmax;
        }
    }

    return lb;
}

int bound_fsp_weak::addBackAndBound(int job, int &prio){
    int lb=0;

    //add job to back and compute max of machine-bounds;
    int tmp0,tmp1,cmax;

    tmp0=back[(nbMachines-1)]+tempsJob[nbMachines-1][job];
    lb=front[nbMachines-1]+remain[nbMachines-1]+back[(nbMachines-1)];//-tempsJob[nbMachines-1][job]);

    prio=0;

    for(int j=nbMachines-2;j>=0;j--){
        // prio=std::max(prio,std::abs(tmp0-back[j]));
        // prio+=(tmp0-back[j]);
        prio+=std::abs(tmp0-back[j]);
        // prio+=std::max(0,tmp0-back[j]);

        tmp1=std::max(tmp0,back[j]);
        cmax=front[j]+remain[j]+tmp1;
        tmp0=tmp1+tempsJob[j][job];
        if(cmax>lb){
            lb=cmax;
        }
    }

    return lb;
}


void bound_fsp_weak::boundChildren(int*schedule, int limit1, int limit2, int *costsBegin, int *costsEnd, int* prioBegin, int* prioEnd)
{
    int job;

    //BEGIN/END LOWER BOUNDS
    if(branchingMode>0){
        memset(costsBegin, 0, nbJob*sizeof(int));
        memset(costsEnd, 0, nbJob*sizeof(int));

        computePartial(schedule,limit1,limit2);

        for (int i = limit1 + 1; i < limit2; i++) {
            job = schedule[i];
            costsBegin[job] = addFrontAndBound(job, prioBegin[job]);
            costsEnd[job] = addBackAndBound(job, prioEnd[job]);
        }
    //BEGIN
    }else{
        memset(costsBegin, 0, nbJob*sizeof(int));
        computePartial(schedule,limit1,limit2);
        for (int i = limit1 + 1; i < limit2; i++) {
            job = schedule[i];

            costsBegin[job] = addFrontAndBound(job,prioBegin[job]);
        }
    }
}

int bound_fsp_weak::evalSolution(int *permut){
    int *tmp = new int[nbMachines];
    int job=0;

    for(int i=0;i<nbMachines;i++)
        tmp[i]=0;

    for(int i=0;i<nbJob;i++){
        job=permut[i];
        tmp[0]+=tempsJob[0][job];
        for(int j=1;j<nbMachines;j++){
            tmp[j]=std::max(tmp[j-1],tmp[j])+tempsJob[j][job];
        }
    }

    int ret=tmp[nbMachines-1];
    delete[]tmp;

    return ret;
}


/////////////////////////////////////////////

void bound_fsp_weak::fillMinTimeArrDep()
{
    int *tmp = new int[nbMachines];

    //min run-out times on each machine
    for (int k = 0; k < nbMachines; k++)minTempsDep[k]=INT_MAX;    
    minTempsDep[nbMachines-1]=0;//always 0 par definition

    for (int i = 0; i<nbJob; i++){
        for (int k = nbMachines-1; k>=0; k--)tmp[k]=0;

        tmp[nbMachines-1]+=tempsJob[nbMachines-1][i];
        for (int k = nbMachines - 2; k >= 0; k--){
            tmp[k]=tmp[k+1]+tempsJob[k][i];
        }
        for (int k = nbMachines-2; k>=0; k--) {
            minTempsDep[k]=(tmp[k+1]<minTempsDep[k])?tmp[k+1]:minTempsDep[k];
        }
    }

    //min start times on each machine
    for (int k = 0; k < nbMachines; k++)minTempsArr[k]=INT_MAX;
    minTempsArr[0]=0;

    for (int i = 0; i < nbJob; i++) {
        for (int k = 0; k < nbMachines; k++)tmp[k]=0;

        tmp[0]+=tempsJob[0][i];
        for (int k = 1; k < nbMachines; k++) {
            tmp[k]=tmp[k-1]+tempsJob[k][i];
        }
        for (int k = 1; k < nbMachines; k++) {
            minTempsArr[k]=(tmp[k-1]<minTempsArr[k])?tmp[k-1]:minTempsArr[k];
        }
    }

    delete[]tmp;
}

void bound_fsp_weak::bornes_calculer(int permutation[], int limite1, int limite2,int*couts, int best)
{
    scheduleFront(permutation, limite1, limite2);
    scheduleBack(permutation, limite2);
    sumUnscheduled(permutation, limite1, limite2);

    int lb;
    int tmp0,tmp1,cmax;

    tmp0=front[0]+remain[0];
    lb=tmp0+back[0];

    for(int j=1;j<nbMachines;j++){
        tmp1=front[j]+remain[j];
        tmp1=std::max(tmp1,tmp0);
        cmax=tmp1+back[j];
//        printf("%d\n",cmax);
        lb=std::max(lb,cmax);
        tmp0=tmp1;
    }

    couts[0]=lb;
}

void bound_fsp_weak::bornes_calculer(int permutation[], int limite1, int limite2)
{

}
