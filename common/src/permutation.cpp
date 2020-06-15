#include <stdlib.h>

#include "../headers/misc.h"
#include "../headers/permutation.h"

permutation::permutation(const int *a, int _N)
{
    N=_N;
    arr=(int*)calloc(N,sizeof(int));
    for(int i=0;i<N;i++){
        arr[i]=a[i];
    }
}

permutation::permutation(int _N)
{
    N=_N;
    arr=(int*)calloc(N,sizeof(int));
}

permutation::~permutation()
{
    free(arr);
}

void permutation::id()
{
    for(int i=0;i<N;i++)arr[i]=i;
}

void permutation::random()
{
    for(int i=0;i<N;i++)arr[i]=i;
    shuffle(arr, N);
}

void permutation::print()
{
    for(int i=0;i<N;i++){
        printf("%3d ",arr[i]);
    }
    printf("\n");
}

void permutation::copy(permutation *p)
{
    for(int i=0;i<N;i++){
        arr[i]=p->arr[i];
    }
}

bool permutation::isValid()
{
    for(int i=0;i<N;i++)
    {
        bool foundOnce=false;
        for(int j=0;j<N;j++){
            if(arr[j]==i){
                if(foundOnce){
                    printf("double entry\n");
                    return false;
                }
                foundOnce=true;
            }
        }
        if(!foundOnce){
            printf("missing entry\n");
            return false;
        }
    }
    return true;
}

int& permutation::operator[](int index)
{
    if (index >= N) {
        printf("Array index out of bound, exiting\n");
        exit(0);
    }
    return arr[index];
}
