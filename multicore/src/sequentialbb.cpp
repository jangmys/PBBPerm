#include "../../common/include/pbab.h"
#include "../../common/include/solution.h"

#include "../include/sequentialbb.h"

sequentialbb::sequentialbb(pbab *_pbb)
{
    pbb=_pbb;
    size=pbb->size;

    IVM = new ivm(size);
    bd = new ivm_bound(pbb);
}

sequentialbb::~sequentialbb()
{
    delete IVM;
    delete bd;
}

void
sequentialbb::clear()
{
    IVM->clearInterval();
}

void
sequentialbb::setRoot(int *varOrder)
{
    clear();
    for(int i=0; i<size; i++){
        IVM->jobMat[i] = varOrder[i];

        // pbb->root_sltn->bestpermut[i];
        // node->schedule[i] = pbb->root_sltn->bestpermut[i];
        // node->schedule[i] = i;
        // IVM->jobMat[i] = i;
    }
    IVM->line=0;
    // node->limit1=-1;
    // node->limit2=size;

    bd->prepareSchedule(IVM);
    bd->boundRoot(IVM);
}

bool
sequentialbb::solvedAtRoot()
{
    bool solved=true;
    for(int i=0;i<size;i++){
        solved &= (IVM->jobMat[i]<0);
    }
    if(solved){
        printf("problem solved at level 0\n");
        for(int i=0; i<size; i++){
            std::cout<<IVM->jobMat[i]<<" ";
        }
        std::cout<<std::endl;
        // IVM->posVect[0]=size;
    }
    return solved;
}

//not thread-safe (setRoot)
void
sequentialbb::initFullInterval()
{
    int * zeroFact = (int *) malloc(size * sizeof(int));
    int * endFact  = (int *) malloc(size * sizeof(int));

    for (int i = 0; i < size; i++) {
        zeroFact[i] = 0;
        endFact[i]  = size - i - 1;
    }

    initAtInterval(zeroFact, endFact);

    free(zeroFact);
    free(endFact);
}

bool
sequentialbb::initAtInterval(int * pos, int * end)
{
    // printf("init at interval\n");

    // ===========================
    // setRoot(pbb->root_sltn->bestpermut);

    int l = 0;
//    while(IVM->posVect[l]==pos[l] && l<IVM->line)l++;
    IVM->line = l;

    for (int i = 0; i < size; i++) {
        IVM->posVect[i] = pos[i];
        IVM->endVect[i] = end[i];
    }

    pbb->stats.simpleBounds = 0;

    if (IVM->beforeEnd()) {
        unfold(arguments::boundMode);
        return true;
    }else{
        return false;
    }
}

void
sequentialbb::jumpback(int level)
{
    std::vector<int> pv(size);
    std::vector<int> ev(size);

    for(int i=0;i<size;i++){
        pv[i]=IVM->posVect[i];
    }

    while(IVM->line>level){
        IVM->goUp();
    }

    for(int i=0;i<=IVM->line;i++){
        ev[i]=IVM->posVect[i];
    }
    for(int i=IVM->line+1;i<size;i++){
        ev[i]=size-i-1;
    }

    pbb->remain.push_back(std::make_tuple(pv,ev));
}

bool sequentialbb::next()
{
    // get best (shared)
    bd->local_best = INT_MAX;
    pbb->sltn->getBest(bd->local_best);

    int state = 0;

    while (IVM->beforeEnd()) {
        if (IVM->lineEndState()) {
            //backtrack...
            IVM->goUp();

            //jumpback
            if(arguments::truncateSearch && \
                IVM->line>arguments::truncateDepth){
                    jumpback(arguments::truncateDepth);
                }

            continue;
        } else if (IVM->pruningCellState()) {
            IVM->goRight();
            continue;
        } else if (!IVM->pruningCellState()) {

            state = 1;// exploring
            pbb->stats.totDecomposed++;// atomic
            // std::cout<<pbb->stats.totDecomposed<<std::endl;

            IVM->goDown();// branch

            // decode IVM -> subproblems
            bd->prepareSchedule(IVM);


            // if (!IVM->beforeEnd()) { //DEBUG
            //     printf("Whaaaaaaat\n"); fflush(stdout);
            // }

            if (IVM->isLastLine()) {
                bd->boundLeaf(IVM);

                state = 0;
                continue;
            }

                // computeBounds=IVM->line;
            break;
        }
    }

    // IVM->displayMatrix();

    // int n=pbb->stats.totDecomposed;
    // IVM->avgline+=(((float)IVM->line-IVM->avgline)/n);

    // pbb->ttm->on(threadtime);
    int bdmode = arguments::boundMode;

    if(state == 1)
    {
        // if(!IVM->beforeEnd())
        // {
        //     printf("state %d : thats a mistake...\n",state);
        //     IVM->displayVector(IVM->posVect);
        //     IVM->displayVector(IVM->endVect);
        //     exit(-1);
        // }

        switch (bdmode) {
            case 0:
                bd->weakBoundPrune(IVM);
                break;
            case 1:
                bd->strongBoundPrune(IVM);
                break;
            case 2:
                bd->mixedBoundPrune(IVM);
                break;
        }

        if (IVM->line >= size - 1) {
            printf("too deeep\n");
            exit(0);
        }
    }

    return (state == 1);
}


// void sequentialbb::run()
// {
//     initFullInterval();
//     while(next());
// }
//
// void sequentialbb::run(int *firstRow)
// {
//     initFullInterval();
//     while(next());
// }

void
sequentialbb::unfold(int mode)
{
    for (int i = 0; i < size; i++) {
        if ((IVM->posVect[i] < 0) || (IVM->posVect[i] >= size - i)) {
            std::cout << " incorrect position vector " << i << " " << IVM->posVect[i] << std::endl;
            // displayVector(posVect);
            exit(-1);
        }
        if ((IVM->endVect[i] < 0) || (IVM->endVect[i] >= size - i)) {
            std::cout << " incorrect end vector " << i << " " << IVM->endVect[i] << std::endl;
            std::cout << " pos " << i << " " << IVM->posVect[i] << std::endl;
            // displayVector(endVect);
            exit(-1);
        }
    }


    while (IVM->line < size - 2) {
        if (IVM->pruningCellState()) {
            for (int i = IVM->line + 1; i < size; i++) {
                IVM->posVect[i] = 0;
            }
            break;
        }

        IVM->line++;
        IVM->generateLine(IVM->line, false);

        // decomposedNodes++;
        // pbb->stats.totDecomposed++;

        bd->prepareSchedule(IVM);

        for (int i = 0; i < size-IVM->line; i++) {
            int job = IVM->jobMat[IVM->line*size+i];
            if(job<0 || job>=size){
                printf("UNFOLD:invalid job %d (line %d)\n",job,IVM->line);

                IVM->displayVector(IVM->posVect);
                IVM->displayVector(IVM->endVect);
                IVM->displayMatrix();//Vector(jm);
                exit(-1);
            }
        }

        switch (mode) {
            case 0:
                bd->weakBoundPrune(IVM);
                break;
            case 1:
                bd->strongBoundPrune(IVM);
                break;
            case 2:
                bd->mixedBoundPrune(IVM);
                break;
        }
    }
} // matrix::unfold
