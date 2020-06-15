#ifndef IVM_BOUND_H
#define IVM_BOUND_H

// #define WEAK (0)
// #define STRONG (1)

#define FRONT (0)
#define BACK (1)

class ivm;
class pbab;
class bound_abstract;
class subproblem;

class ivm_bound{
private:
    int size;

public:
    subproblem* node;
    pbab* pbb;
    bound_abstract *bound[2];

    int initialUB;

    int local_best;
    int* costsBegin[2];
    int* costsEnd[2];

    int * prio;
    int * priorityBegin;
    int * priorityEnd;

    static int* rootRow;
    static int rootDir;
    static int first;

    ivm_bound(pbab* _pbb);
    ~ivm_bound();

    void completeSchedule(const int job,const int order);

    // void buildPriorityTables();

    void allocate();
    bool gtBest(const int);

    void prepareSchedule(const ivm* IVM);

    void computeWeakBounds();
    void computeStrongBounds(const int be);

    int chooseChildrenSet(const ivm* IVM, const int*cb, const int* ce, const int mode);
    int eliminateJobs(ivm *IVM,int *cost1,int *cost2, const int dir);
    void prune(ivm* IVM, const int first,const int second);

    void weakBoundPrune(ivm* IVM);
    void strongBoundPrune(ivm* IVM);
    void mixedBoundPrune(ivm* IVM);

    void boundNode(const ivm* IVM);
    void boundLeaf(ivm* IVM);

    void boundRootWeak(ivm *IVM);
    void boundRootStrong(ivm *IVM);
    void boundRoot(ivm* IVM);

};

#endif /* IVM_BOUND_H */
