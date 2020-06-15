#ifndef SEQUENTIALBB_H_
#define SEQUENTIALBB_H_

// #include "../../common/headers/pbab.h"

#include "../include/ivm.h"
#include "../include/ivm_bound.h"
#include "../../common/include/ttime.h"


#include "../../heuristic/include/IG.h"

#include "../../bounds/include/libbounds.h"

class pbab;

class sequentialbb{
public:
    sequentialbb(pbab* _pbb);
    ~sequentialbb();

    bool initAtInterval(int * pos, int * end);
    void initFullInterval();
    void setRoot(int* varOrder);
    bool solvedAtRoot();

    void run();
    void run(int* firstRow);

    void jumpback(int level);

    bool next();
    void clear();

    ivm* IVM;
private:
    pbab* pbb;

protected:
    ivm_bound* bd;
    int size;

    void unfold(int mode);
};


#endif
