#ifndef FASTINSERTREMOVE_H_
#define FASTINSERTREMOVE_H_

#include "../../bounds/include/instance_abstract.h"

struct tabulist
{
    int nmax;
    int num;
    int *arr;

    tabulist(int N)
    {
        nmax=N;
        num=0;
        arr=(int*)malloc(nmax*sizeof(int));
    }

    ~tabulist(){
        free(arr);
    }

    bool isTabu(int a){
        for(int i=0;i<num;i++){
            if(arr[i]==a)return true;
        }
        return false;
    };
    void add(int a){
        arr[num++]=a;
    };
    void clear(){
        for(int i=0;i<nmax;i++){
            arr[i]=0;
        }
        num=0;
    };
    void rem(int a){
        int i=num-1;
        while(arr[i]!=a && i>=0)i--;

        if(i<0){
            printf("can't remove %d from tblist(%d) : not found\n",a,num);
            for(int k=0;k<num;k++){
                printf("%d ",arr[k]);
            }
            printf("\n");fflush(stdout);
            exit(-1);
        }
        else{
            for(int j=i;j<num-2;j++)
            {
                arr[j]=arr[j+1];
            }
        }
        num--;
    };
};


class fastInsertRemove{
public:
    instance_abstract * instance;

    int nbJob;
    int nbMachines;
    int **PTM;
    int *sumPT;

    fastInsertRemove(instance_abstract* inst);
    ~fastInsertRemove();

    // std::vector<std::vector<int>>heads;

    int **head;
    int **tail;
    int **inser;

    tabulist *tabujobs;
    tabulist *tabupos;

    int insertMakespans(int* perm, int len, int job, std::vector<int>& makespans);
    int removeMakespans(int* perm, int len, int* makespans);

    int computeHeads(int* const perm, int len);
    void computeTails(int* perm, int len);
    void computeInser(int *perm, int len, int job);

    void insert(int* perm, int &len, int pos, int job);
    int remove(int *perm, int &len, const int pos);

    int bestInsert(int *perm, int &len, int job, int &cmax);
    int bestRemove(int *perm, int &len, int &remjob, int &cmax);

    int bestInsert2(int *perm, int &len, int job, int &cmax);
    int bestRemove2(int *perm, int &len, int &remjob, int &cmax);

    void computeInserBlock(int *perm, int len, int * block, int bl);
    int insertBlockMakespans(int* perm, int len, int* block, int bl, int* makespans);
    void blockInsert(int* perm, int &len, int pos, int*block, int bl);
    int bestBlockInsert(int* perm, int len, int*block, int bl);
};

#endif
