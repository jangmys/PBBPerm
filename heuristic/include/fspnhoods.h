#ifndef FSPNHOODS_H_
#define FSPNHOODS_H_

#include "../../bounds/include/instance_abstract.h"

#include "fastinsertremove.h"


class fspnhood{
public:
    fspnhood(instance_abstract* inst);
    ~fspnhood();

    fastInsertRemove* m;

    int N;


    int fastBREmove(int* perm, int pos);
    int kImove(int* perm,int pos, int kmax);
    int fastkImove(int* perm,int kmax);
    int fastkImove(int* perm,int kmax,int l1,int l2);

};


#endif
