#include "../../common/include/macros.h"
#include "../../common/include/pbab.h"
#include "../../common/include/solution.h"
#include "../../common/include/subproblem.h"

#include "../../bounds/include/libbounds.h"

#include "../include/ivm.h"
#include "../include/ivm_bound.h"


//static members
int* ivm_bound::rootRow = NULL;
int  ivm_bound::rootDir = 0;
int  ivm_bound::first = true;

ivm_bound::ivm_bound(pbab* _pbb){
    pbb=_pbb;
    size=pbb->size;

    local_best=INT_MAX;

    allocate();

    initialUB = pbb->sltn->getBest();

    if(arguments::problem[0]=='f'){
        bound_fsp_weak *bd=new bound_fsp_weak();
        bd->set_instance(pbb->instance);
        bd->init();
        bd->branchingMode=arguments::branchingMode;
        bound[WEAK]=bd;

        // bound[STRONG]=new bound_fsp_strong();

        bound_fsp_strong *bd2=new bound_fsp_strong();
        bd2->set_instance(pbb->instance);
        bd2->init();
        // bound[STRONG]=new bound_fsp_weak();
        // bound[STRONG]->set_instance(pbb->instance);
        // bound[STRONG]->init(arguments::branchingMode,arguments::earlyStopJohnson,
        //\ arguments::johnsonPairs);
        bd2->branchingMode=arguments::branchingMode;
        bd2->earlyExit=arguments::earlyStopJohnson;
        bd2->machinePairs=arguments::johnsonPairs;
        bound[STRONG]=bd2;
    }
    if(arguments::problem[0]=='n'){


        // bound[WEAK]=new bound_nqueens();
        // bound[WEAK]->set_pbab(pbb);
        // bound[WEAK]->set_instance(pbb->instance);
        // bound[WEAK]->init(arguments::branchingMode,0,0);
        //
        // bound[STRONG]=NULL;
    }

    //TOY PROBLEM FOR TESTING
    if(arguments::problem[0]=='t'){
        // bound[WEAK]=new bound_nqueens();
        // bound[WEAK]->set_instance(pbb->instance);
        // bound[WEAK]->init(arguments::branchingMode,0,0);
        //
        // bound[STRONG]=new bound_test();
        // bound[STRONG]->set_instance(pbb->instance);
        // bound[STRONG]->init(arguments::branchingMode,0,0);
    }
}

ivm_bound::~ivm_bound()
{
    delete node;
    free(costsBegin[WEAK]);
    free(costsEnd[WEAK]);
    free(costsBegin[STRONG]);
    free(costsEnd[STRONG]);

    free(priorityBegin);
    free(priorityEnd);
    free(prio);
}

void
ivm_bound::allocate(){
    node=new subproblem(size);

    posix_memalign((void **) &costsBegin[WEAK], ALIGN, size * sizeof(int));
    posix_memalign((void **) &costsEnd[WEAK], ALIGN, size * sizeof(int));
    posix_memalign((void **) &costsBegin[STRONG], ALIGN, size * sizeof(int));
    posix_memalign((void **) &costsEnd[STRONG], ALIGN, size * sizeof(int));

    if(!rootRow){
        posix_memalign((void **) &rootRow, ALIGN, size * sizeof(int));
        memset(rootRow, 0, size * sizeof(int));
    }

    posix_memalign((void **) &priorityBegin, ALIGN, size * sizeof(int));
    posix_memalign((void **) &priorityEnd, ALIGN, size * sizeof(int));
    posix_memalign((void **) &prio, ALIGN, size * sizeof(int));

    // posix_memalign((void **) &fixedJobs, ALIGN, size * sizeof(int));
    // memset(fixedJobs, 0, size * sizeof(int));

    memset(costsBegin[WEAK], 0, size * sizeof(int));
    memset(costsBegin[STRONG], 0, size * sizeof(int));
    memset(costsEnd[WEAK], 0, size * sizeof(int));
    memset(costsEnd[STRONG], 0, size * sizeof(int));
}

void
ivm_bound::prepareSchedule(const ivm* IVM)
{
    const int* const jM  = IVM->jobMat;
    const int* const pV  = IVM->posVect;
    int _line = IVM->line;

    node->limit1 = -1;
    node->limit2 = size;

    // memset(fixedJobs,0,size*sizeof(int));

    for (int l = 0; l < _line; l++) {
        int pointed = pV[l];
        int job     = absolute(jM[l * size + pointed]);

        // fixedJobs[job]=1;

        if (IVM->dirVect[l] == 0) {
            node->schedule[++node->limit1] = job;
        } else {
            node->schedule[--node->limit2] = job;
        }
    }
    for (int l = 0; l < size - _line; l++){
        node->schedule[node->limit1 + 1 + l] = absolute(jM[_line * size + l]);
    }
} // prepareSchedule

void
ivm_bound::computeStrongBounds(const int be){
    int _limit1 = node->limit1 + (be==FRONT?1:0);
    int _limit2 = node->limit2 - (be==BACK?1:0);

    int *costsFirst;
    int *costsSecond;
    int *priority;

    if(be==FRONT){
        costsFirst = costsBegin[STRONG];
        costsSecond = costsBegin[WEAK];
        priority = priorityBegin;
    }else{    // if(be==BACK)
        costsFirst = costsEnd[STRONG];
        costsSecond = costsEnd[WEAK];
        priority = priorityEnd;
    }

    memset(costsFirst, 0, size*sizeof(int));
    memset(priority, 0, size*sizeof(int));

    int fillPos = (be==FRONT?_limit1:_limit2);
    int costs[2];

    for (int i = node->limit1 + 1; i < node->limit2; i++) {
        int job = node->schedule[i];

        if(!gtBest(costsSecond[job])){
            swap(&node->schedule[fillPos], &node->schedule[i]);
            bound[WEAK]->bornes_calculer(node->schedule, _limit1, _limit2, costs, arguments::earlyStopJohnson?local_best:-1);
            // bound[STRONG]->bornes_calculer(node->schedule, _limit1, _limit2, costs, arguments::earlyStopJohnson?local_best:-1);

            costsFirst[job] = costs[0];
            priority[job]=costs[1];

            swap(&node->schedule[fillPos], &node->schedule[i]);

            pbb->stats.johnsonBounds++;
            // countJohnson++;
        }
    }

#ifdef DEBUG
    if(be==FRONT){
        for (int i = 0; i < size; i++)
            printf("%d ",costsBegin[STRONG][i]);
        printf("\n");
    }
    if(be==BACK){
        for (int i = 0; i < size; i++)
            printf("%d ",costsEnd[STRONG][i]);
        printf("\n");
    }
#endif
}

void
ivm_bound::boundNode(const ivm* IVM)
{
    int dir=IVM->dirVect[IVM->line];

    if (dir == 1){
        computeStrongBounds(BACK);
    }else if(dir == 0){
        computeStrongBounds(FRONT);
    }else if(dir == -1){
        // printf("eval BE johnson\n");
        computeStrongBounds(FRONT);
        computeStrongBounds(BACK);
    }else{
        perror("boundNode");exit(-1);
    }
}

void
ivm_bound::computeWeakBounds()
{
    memset(costsBegin[STRONG], 0, size*sizeof(int));
    memset(costsEnd[STRONG], 0, size*sizeof(int));
	// std::cout<<*node<<"\n";

    bound[WEAK]->boundChildren(node->schedule,node->limit1,node->limit2,costsBegin[WEAK],costsEnd[WEAK],priorityBegin,priorityEnd);

    //if begin-end
    if(arguments::branchingMode>0){
        pbb->stats.simpleBounds+=2*(node->limit2-node->limit1-1);
    }else{
        pbb->stats.simpleBounds+=(node->limit2-node->limit1-1);
    }

	// for(int i=0;i<size;++i)printf("%d ",costsBegin[WEAK][i]);
    // printf("\n");
    // for(int i=0;i<size;++i)printf("%d ",costsEnd[WEAK][i]);
    // printf("\n");
    // printf("\n");
}

void
ivm_bound::boundLeaf(ivm* IVM)
{
    pbb->stats.leaves++;

    int cost;
    if(bound[STRONG])cost=bound[STRONG]->evalSolution(node->schedule);
    else cost=bound[WEAK]->evalSolution(node->schedule);
    // printf("leaf %d\n",cost);

    if (!gtBest(cost)) {
        //update local best...
        local_best=cost;
        //...and global best (mutex)
        pbb->sltn->update(node->schedule,cost);
        pbb->foundSolution=true;

        //print new best solution (not for NQUEENS)
        if(arguments::printSolutions){

            IVM->displayVector(IVM->posVect);
            pbb->sltn->print();
        }
    }
    //mark solution as visited
    int pos = IVM->posVect[IVM->line];
    int job = IVM->jobMat[IVM->line * size + pos];
    IVM->jobMat[IVM->line * size + pos] = negative(job);
}

void
ivm_bound::boundRoot(ivm* IVM){
	pbb->sltn->getBest(local_best);

    node->limit1=-1;
    node->limit2=size;

    if(!first){
        memcpy(IVM->jobMat, rootRow, size*sizeof(int));
        IVM->dirVect[0] = rootDir;
    }else{
        first = false;
        // std::cout<<"FIRST"<<std::endl;

        //first line of Matrix
        for(int i=0; i<size; i++){
            node->schedule[i] = pbb->root_sltn->perm[i];
            IVM->jobMat[i] = pbb->root_sltn->perm[i];
            // node->schedule[i] = i;
            // IVM->jobMat[i] = i;
        }
        IVM->line=0;
        node->limit1=-1;
        node->limit2=size;

        // if(arguments::problem[0]=='f')
        //     strongBoundPrune(IVM);
        // else
            weakBoundPrune(IVM);

        // IVM->displayVector(costsBegin[WEAK]);
        // IVM->displayVector(costsEnd[WEAK]);

        //save first line of matrix (bounded root decomposition)
        rootDir = IVM->dirVect[0];
        memcpy(rootRow, IVM->jobMat, size*sizeof(int));

        // IVM->displayMatrix();

        //std::cout<<"===========\n";

        // bool solved=true;
        // for(int i=0;i<size;i++){
        //     solved &= (IVM->jobMat[i]<0);
        // }
        // if(solved){
        //     printf("problem solved at level 0\n");
        //     for(int i=0; i<size; i++){
        //         std::cout<<IVM->jobMat[i]<<" ";
        //     }
        //     std::cout<<std::endl;
        //     // IVM->posVect[0]=size;
        // }
    }

    memset(costsBegin[STRONG], 0, size*sizeof(int));
    memset(costsEnd[STRONG], 0, size*sizeof(int));
}

// void ivm_bound::sortNodes(ivm* IVM)
// {
//
// }

int
ivm_bound::eliminateJobs(ivm* IVM, int *cost1, int *cost2,const int dir)
{
    int _line=IVM->line;

    int * jm = IVM->jobMat + _line * size;

    switch (arguments::sortNodes) {
        case 1://non-decreasing cost1
        {
            jm = IVM->jobMat + _line * size;
            gnomeSortByKeyInc(jm, cost1, 0, size-_line-1);
            break;
        }
        case 2://non-decreasing cost1, break ties by priority (set in chooseChildrenSet)
        {
            jm = IVM->jobMat + _line * size;
            gnomeSortByKeysInc(jm, cost1, prio, 0, size-_line-1);
            break;
        }
        case 3:
        {
            jm = IVM->jobMat + _line * size;
            gnomeSortByKeyInc(jm, prio, 0, size-_line-1);
            break;
        }
        case 4:
        {
            jm = IVM->jobMat + _line * size;
            gnomeSortByKeysInc(jm, cost1, prio, 0, size-_line-1);
            break;
        }
    }

    // eliminate
    for (int i = 0; i < size-_line; i++) {
        int job = jm[i];
        // if(job<0 || job>=size){
        //     printf("ivm_bound:invalid job %d (line %d %d)\n",job,_line,IVM->line);
        //
        //     IVM->displayVector(cost1);
        //     IVM->displayVector(cost2);
        //
        //     IVM->displayVector(IVM->posVect);
        //     IVM->displayVector(IVM->endVect);
        //     IVM->displayMatrix();//Vector(jm);
        //
        //     return -1;
        // }

        // if(prio[job]<local_best)
        //     printf("%d\n",prio[job]);

        // printf("%d %d -- ",cost1[job],prio[job]);

        if(gtBest(cost1[job])){
            jm[i] = negative(job);
        }else if(gtBest(cost2[job])) {//johnson
            jm[i] = negative(job);
        }
    }

    // printf("\n");

    // if(arguments::truncateSearch && _line >= arguments::truncateDepth)
    // {
    //     for (int i = 0; i < size-_line; i++) {
    //         job = jm[i];
    //         jm[i] = negative(job);
    //     }
    // }

    return 0;
}

int
ivm_bound::chooseChildrenSet(const ivm* IVM, const int*cb, const int* ce, const int mode)
{
    int retval=-1;

    switch(mode){
        case -3://ALTERnATE 0 1 0 1 0 ...
        {
            retval=(IVM->line%2)?BACK:FRONT;
            break;
        }
        case -2://END
        {
            retval=BACK;
            break;
        }
        case -1://BEGIN
            retval=FRONT;
            break;
        case 1://MaxSum (requires unscheduled reset to 0)
        {
            // std::cout<<"sum\n";
            int sum = 0;
           /* int _line=IVM->line;
            int * jm = IVM->jobMat + _line * size;
            for (int i = 0; i < size-IVM->line; i++) {
                int job = jm[i];

            // for (int i = 0; i < size; ++i) {
                sum += (cb[job]-ce[job]);
            }*/

            for (int i = 0; i < size; ++i) {
                sum += (cb[i]-ce[i]);
            }


            retval = (sum < 0)?BACK:FRONT; //choose larger sum, tiebreak : FRONT

            // std::cout<<sum<<" "<<retval<<"\n";



            break;
        }
        case 2: //MinBranch
        {
            //count eliminated - based on initial UB... don't use current UB in distributed parallelBB !
            int ub=local_best;
            if(!arguments::singleNode)ub=initialUB;

            int elim = 0;
            int sum = 0;

            for (int i = 0; i < size; ++i) {
                if (cb[i]>=ub)elim++; //"pruned"
                else sum += cb[i];
                if (ce[i]>=ub)elim--; //"pruned"
                else sum -=ce[i];
            }
            //take set with lss open nodes / tiebreaker: greater average LB
            if(elim > 0)retval = FRONT;
            else if(elim < 0)retval = BACK;
            else retval = (sum < 0)?BACK:FRONT;

            break;
        }
        case 3: //MinMin, tiebreak: MinBranch
        {
            int ub=local_best;
            if(!arguments::singleNode)ub=initialUB;

            int min=INT_MAX;
            int minCount=0;
            int elimCount=0;

            //find min lower bound
            for (int i = 0; i < size; ++i) {
                if(cb[i]<min && cb[i]>0)min=cb[i];
                if(ce[i]<min && ce[i]>0)min=ce[i];
            }
            //how many times lowest LB realized?
            for (int i = 0; i < size; ++i) {
                if(cb[i]==min)minCount++;
                if(ce[i]==min)minCount--;
                if (cb[i]>=ub)elimCount++;
                if (ce[i]>=ub)elimCount--;
            }
            //take set where min LB is realized LESS often
            if(minCount > 0)retval = BACK;
            else if(minCount < 0)retval = FRONT;
            else retval = (elimCount > 0)?FRONT:BACK;//break ties

            break;
        }
    }

    //priorities for sorting sibling nodes
    if(retval==BACK){
        if(arguments::nodePriority==4){
                // std::cout<<"BACK\n";
            for (int i = 0; i < size; ++i) {
                // std::cout<<i<<" "<<ce[i]<<" "<<local_best<<"\n";
                if(ce[i]>=local_best){
                    prio[i]=999999;
                    continue;
                }
                completeSchedule(i,BACK);

                prio[i]=bound[WEAK]->evalSolution(node->schedule);
                // std::cout<<*node<<"\n";

                // std::cout<<*node<<" "<<prio[i]<<"\n";
            }
        }
        for (int i = 0; i < size; ++i) {
            if(arguments::nodePriority==1)prio[i]=ce[i];
            if(arguments::nodePriority==2)prio[i]=ce[i]-cb[i];
            if(arguments::nodePriority==3)prio[i]=(ce[i]>0)?priorityEnd[i]:99999;
        }
    }else{
        if(arguments::nodePriority==4){
            // std::cout<<"FRONT\n";
            for (int i = 0; i < size; ++i) {
                if(cb[i]>=local_best){
                    prio[i]=999999;
                    continue;
                }
                completeSchedule(i,FRONT);

                prio[i]=bound[WEAK]->evalSolution(node->schedule);

                // if(prio[job]<local_best)
                //     printf("%d\n",prio[job]);

                // std::cout<<*node<<" "<<prio[i]<<"\n";
            }
        }
        for (int i = 0; i < size; ++i) {
            if(arguments::nodePriority==1)prio[i]=cb[i];
            if(arguments::nodePriority==2)prio[i]=cb[i]-ce[i];
            if(arguments::nodePriority==3)prio[i]=(cb[i]>0)?priorityBegin[i]:99999;
        }
    }
    // std::cout<<"\n";


    return retval;
}

void
ivm_bound::completeSchedule(const int job,const int order)
{
    int* fixedJobs = new int[size];
    memset(fixedJobs,0,size*sizeof(int));

    fixedJobs[job]=1;
    int i,j;

    for(i=0;i<=node->limit1;i++){
        j=node->schedule[i];
        fixedJobs[j]=1;
    }
    for(i=node->limit2;i<size;i++){
        j=node->schedule[i];
        fixedJobs[j]=1;
    }
    // for(int j=0; j<size; j++){
    //     std::cout<<fixedJobs[j]<<" ";
    // }
    // std::cout<<"\n";

    //==============
    if(order == FRONT)
    {
        i=node->limit1+1;

        node->schedule[i++]=job;
        for(int k=0; k<size; k++){
            j=pbb->root_sltn->perm[k];
            if(fixedJobs[j]==1)continue;
            node->schedule[i++]=j;
        }
    }
    if(order == BACK)
    {
        i=node->limit1+1;

        for(int k=0; k<size; k++){
            j=pbb->root_sltn->perm[k];
            if(fixedJobs[j]==1)continue;
            node->schedule[i++]=j;
        }
        node->schedule[i++]=job;
    }
    delete[] fixedJobs;
}

void
ivm_bound::prune(ivm* IVM, const int first, const int second)
{
    int _line=IVM->line;
    int* jm;// = IVM->jobMat + _line * size;

    switch (arguments::sortNodes) {
        case 0:
        {
            jm = IVM->jobMat + _line * size;
            int prev_dir=(_line>0)?IVM->dirVect[_line-1]:0;
            if(prev_dir!=IVM->dirVect[_line])
            {
                // std::cout<<"line "<<_line<<" dir "<<IVM->dirVect[_line]<<" reverse\n";

                int i1=0;
                int i2=size-_line-1;
                while(i1<i2){
                    swap(&jm[i1], &jm[i2]);
                    i1++; i2--;
                }
            }
            if(prev_dir==1 && IVM->dirVect[_line]==0){
                for (int l = 0; l < size - _line; l++){
                    node->schedule[node->limit1 + 1 + l] = absolute(jm[l]);
                }
            }
            break;
        }
    }

    if (IVM->dirVect[IVM->line] == 1){
        eliminateJobs(IVM, costsEnd[first],costsEnd[second],1);
    }else if(IVM->dirVect[IVM->line] == 0){
        eliminateJobs(IVM, costsBegin[first],costsBegin[second],0);
    }
}


void
ivm_bound::weakBoundPrune(ivm* IVM){
    computeWeakBounds();

    IVM->dirVect[IVM->line]=chooseChildrenSet(IVM,costsBegin[WEAK],costsEnd[WEAK],arguments::branchingMode);
    prune(IVM,WEAK,WEAK);

    // IVM->displayVector(&IVM->jobMat[IVM->line*size]);
    // std::cout<<*node<<"\n";
}

void
ivm_bound::mixedBoundPrune(ivm* IVM){
    computeWeakBounds();
    IVM->dirVect[IVM->line]=chooseChildrenSet(IVM,costsBegin[WEAK],costsEnd[WEAK],arguments::branchingMode);
    boundNode(IVM);
    prune(IVM,WEAK,STRONG);
}

void
ivm_bound::strongBoundPrune(ivm* IVM){
    IVM->dirVect[IVM->line]=-1;
    memset(costsBegin[WEAK],INT_MAX,size*sizeof(int));
    memset(costsEnd[WEAK],INT_MAX,size*sizeof(int));
    boundNode(IVM);
    IVM->dirVect[IVM->line]=chooseChildrenSet(IVM,costsBegin[STRONG],costsEnd[STRONG],arguments::branchingMode);
    prune(IVM,STRONG,STRONG);
}

bool
ivm_bound::gtBest(const int val){
    if(arguments::findAll)return val>local_best;
    else return val>=local_best;
}
