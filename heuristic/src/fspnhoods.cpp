#include <iostream>

#include "../../common/include/misc.h"
#include "../include/fspnhoods.h"

fspnhood::fspnhood(instance_abstract* inst){
    m=new fastInsertRemove(inst);

    N=m->nbJob;
}

fspnhood::~fspnhood()
{
    delete m;
}

//Deroussi et al...
int fspnhood::fastBREmove(int* perm, int pos)
{
    int len=N;

    int cmax0=m->computeHeads(perm, len);
    //remove job from position pos...
    int rjob=m->remove(perm,len,pos);

    int cmax1;
    m->tabupos->add(pos);//position
    m->bestInsert(perm, len, rjob, cmax1); //insert at best other position
    m->tabupos->clear();

    // int cmax2=cmax1;
    if(cmax0<cmax1){ //if no improvement
        int rjob2;
        m->tabujobs->add(rjob);//job
        int pos2=m->bestRemove(perm, len, rjob2, cmax1);
        m->tabujobs->clear();

        m->tabupos->add(pos2);//position
        m->bestInsert(perm, len, rjob2, cmax1);
        m->tabupos->clear();
    }

    return cmax1;
}

int fspnhood::kImove(int* perm,int pos, int kmax)
{
    bool found=false;
    int k=0;

    int len=N;
    int cmax0,cmax1;
    int rjob;

    cmax0=m->computeHeads(perm, len);

    // std::cout<<"... "<<cmax0<<"\n";

    m->tabupos->clear();
    m->tabujobs->clear();
    //remove job at position pos (and get removed)
    rjob=m->remove(perm, len, pos);

    //make job and position tabu
    m->tabujobs->add(rjob);
    m->tabupos->add(pos);

    while(!found && k<kmax)
    {
        //find best position to insert removed job (and get resulting makespan)
        m->bestInsert(perm, len, rjob, cmax1);
        // std::cout<<"... "<<cmax1<<" "<<cmax0<<"\n";

        if(cmax1<=cmax0)
        {
            found=true;
            break;
        }
        else
        {
            k++;
            if(k==kmax)break;

            m->tabupos->clear();
            pos=m->bestRemove2(perm, len, rjob, cmax1);

            m->tabujobs->add(rjob);
            m->tabupos->add(pos);
        }
    }
    // printf("===\n");
    // if(found){
    //     for(int i=0;i<N;i++){
    //         printf("%d ",perm[i]);
    //     }
    //     printf("\n");
    // }

    return cmax1;
}

int fspnhood::fastkImove(int* perm,int kmax)
{
    bool found=false;
    int k=0;

    int len=N;
    int cmax0,cmax1;
    int rjob;

    cmax0=m->computeHeads(perm, len);
    // printf("start with %d\n",cmax0);

    m->tabupos->clear();
    m->tabujobs->clear();
    int pos=m->bestRemove2(perm, len, rjob, cmax1);
    m->tabujobs->add(rjob);
    m->tabupos->add(pos);

    while(!found && k<kmax)
    {
        m->bestInsert(perm, len, rjob, cmax1);

        if(cmax1<cmax0)
        {
            // printf("%d ---> %d\n",cmax0,cmax1);
            found=true;
            break;
        }
        else
        {
            k++;
            if(k==kmax)break;
            m->tabupos->clear();

            pos=m->bestRemove(perm, len, rjob, cmax1);
            m->tabujobs->add(rjob);
            m->tabupos->add(pos);
        }
    }

    return cmax1;
}

int fspnhood::fastkImove(int* perm,int kmax,int l1,int l2)
{
    bool found=false;
    int k=0;

    int len=N;
    int cmax0,cmax1;
    int rjob;

    cmax0=m->computeHeads(perm, len);
    // printf("start with %d\n",cmax0);

    m->tabupos->clear();
    m->tabujobs->clear();
    for(int i=0;i<l1;i++)
    {
        m->tabujobs->add(perm[i]);
        m->tabupos->add(i);
    }
    for(int i=l2;i<len;i++)
    {
        m->tabujobs->add(perm[i]);
        m->tabupos->add(i);
    }

    int pos=m->bestRemove(perm, len, rjob, cmax1);
    m->tabujobs->add(rjob);
    m->tabupos->add(pos);

    while(!found && k<kmax)
    {
        m->bestInsert(perm, len, rjob, cmax1);
        // printf("get %d\n",cmax1);

        if(cmax1<cmax0)
        {
            found=true;
            break;
        }
        else
        {
            k++;
            // m->tabupos->clear();
            m->tabupos->rem(pos);

            pos=m->bestRemove(perm, len, rjob, cmax1);
            m->tabujobs->add(rjob);
            m->tabupos->add(pos);
        }
    }

    if(found)return cmax1;
    else return cmax0;
}
