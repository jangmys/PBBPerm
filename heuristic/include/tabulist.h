#ifndef TABULIST_H_
#define TABULIST_H_

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
        arr[++num]=a;
    };
    void clear(){
        for(int i=0;i<nmax;i++){
            arr[i]=0;
        }
        num=0;
    };
    void rem(int a){
        for(int i=num-1;i>=0;i--){
            if(arr[i]!=a)continue
            else
            {
                for(int j=i;j<num-1;j++)
                {
                    arr[j]=arr[j+1];
                }
            }
        }
        num--;
    };
};

#endif
