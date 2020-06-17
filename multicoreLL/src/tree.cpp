#include "macros.h"
#include "solution.h"
#include "pbab.h"
#include "subproblem.h"
#include "log.h"

#include "tree.h"

#include "../../bounds/include/libbounds.h"


Tree::Tree(unsigned int id, pbab * pbb)
{
    //data structure used for pool of subproblems
    strategy = DEQUE;
    // strategy = PRIOQ;

    this->pbb = pbb;
    this->local_best = INT_MAX;
    pbb->sltn->getBest(this->local_best);
    initialUB=local_best;

    //set and configure bound 0
    bound_fsp_weak *bd=new bound_fsp_weak();
    bd=new bound_fsp_weak();
    bd->set_instance(pbb->instance);
    bd->init();
    bd->branchingMode=arguments::branchingMode;
    lb[WEAK]=bd;

    //set and configure bound 1
    if(arguments::boundMode>1){
        bound_fsp_strong *bd2=new bound_fsp_strong();
        bd2->set_instance(pbb->instance);
        bd2->init();
        bd2->branchingMode=arguments::branchingMode;
        bd2->earlyExit=arguments::earlyStopJohnson;
        bd2->machinePairs=arguments::johnsonPairs;
        lb[STRONG]=bd2;
    }

    prioFwd = (int*)malloc(pbb->size*sizeof(int));
    prioBwd = (int*)malloc(pbb->size*sizeof(int));

    FILE_LOG(logINFO)<<"Tree constructed";
}

void
Tree::setRoot(const int *perm)
{
    subproblem * root = new subproblem(pbb->size);

    for(int i=0;i<pbb->size;i++){
        root->schedule[i]=perm[i];
    }
    root->cost = 0;

    push(root);
    // FILE_LOG(logINFO)<<size()<<" nodes:\t"<<*top();

}

//=============================================================================================
//================================================= gestion pools ===========================
//=============================================================================================
//======================================================================
int
Tree::size()
{
    switch (strategy) {
        case DEQUE:
            return deq.size();
        case STACK:
            return pile.size();
        case PRIOQ:
            return pque.size();
        default:
            std::cout << "Undefined DS";
            exit(1);
    }
}

void
Tree::push_back(subproblem * p)
{
    deq.push_back(p);
}

void
Tree::push(subproblem * p)
{
    switch (strategy) {
        case DEQUE:
            deq.push_front(p);
            break;
        case STACK:
            pile.push(p);
            break;
        case PRIOQ:
            pque.push(p);
            break;
        default:
            std::cout << "Undeended strategy";
            exit(1);
    }
}

void
Tree::pop()
{
    switch (strategy) {
        case DEQUE:
            deq.pop_front();
            break;
        case STACK:
            pile.pop();
            break;
        case PRIOQ:
            pque.pop();
            break;
        default:
            std::cout << "Undeended strategy";
            exit(1);
    }
}

subproblem *
Tree::top()
{
    switch (strategy) {
        case DEQUE:
            return deq.front();
        case STACK:
            return pile.top();
        case PRIOQ:
            return pque.top();
        default:
            std::cout << "Undeended strategy";
            exit(1);
    }
}

bool
Tree::empty()
{
    switch (strategy){
        case DEQUE:
            return deq.empty();
        case STACK:
            return pile.empty();
        case PRIOQ:
            return pque.empty();
        default:
            std::cout << "Undeended strategy";
            exit(1);
    }
}

subproblem *
Tree::take()//take problem from top
{
    subproblem *n=(empty()) ? NULL : top();
    if(n) pop();

    return n;
}


//======================== B&B-step=======================
bool
Tree::next()
{
    pbb->sltn->getBest(local_best);
    bool continuer = false;

    // FILE_LOG(logDEBUG)<<"poolsize "<<size()<<"\t"<<local_best;

    subproblem * n;
    if (!empty()) {
        n = take();

        FILE_LOG(logDEBUG)<<"take node "<<*n<<"\t"<<local_best;

        if(bound(n)){
            if (n->leaf()) {
                pbb->stats.leaves++;
                improve(n); //leaf with better cost than UB
            } else  {
                std::vector<subproblem*>ns;
                // ns=decompose(*n);
                ns=decompose_fast(*n);
                insert(ns);
                pbb->stats.totDecomposed++;
                pbb->stats.simpleBounds += 2*(n->limit2-n->limit1-1);
            }
        }
        continuer = true;
        delete(n);

        FILE_LOG(logDEBUG)<<"in pool: "<<size();
    }

    return continuer;
}

void
Tree::improve(const subproblem * pr)
{
    local_best = pr->cost;

    FILE_LOG(logINFO)<<"Leaf:\t"<<*pr;

    //...and global best (mutex)
    pbb->sltn->update(pr->schedule,local_best);
    pbb->foundSolution=true;
    pbb->sltn->print();
}

bool
Tree::bound(subproblem * pr)
{
    FILE_LOG(logDEBUG3) << "LB: " << pr->cost << " Best: " << local_best;

    return (pr->cost < local_best);
}

// dynamic branching policies : choose between two sets of children nodes;
int
Tree::chooseBranching(std::vector<subproblem*>begin,std::vector<subproblem*>end)
{
    int retval=BEGIN_ORDER;

    switch (arguments::branchingMode) {
        case -3:
        {
            retval=(begin.size()%2)?BEGIN_ORDER:END_ORDER;
            break;
        }
        case -2:
        {
            retval=END_ORDER;
            break;
        }
        case -1:
        {
            retval=BEGIN_ORDER;
            break;
        }
        case 1:
        {
            int sumBegin = 0, sumEnd = 0;
            for (auto const& i: begin) {
                sumBegin+=i->cost;
            }
            for (auto const& i: end) {
                sumEnd+=i->cost;
            }
            if (sumBegin >= sumEnd)retval = BEGIN_ORDER;//begin
            else retval = END_ORDER;//end
            break;
        }
        case 2:
        {
            if(begin.size()<end.size())return BEGIN_ORDER;
            if(end.size()<begin.size())return END_ORDER;

            int sum0 = 0, sum1 = 0;
            // int elim0 = 0, elim1 = 0;
            //
            // //count pruned begin:
            // for (auto const& i: begin) {
            //     if(i->cost >= local_best)elim0++;
            //     else sum0 += i->cost;
            // }
            // //count pruned end:
            // for (auto const& i: end) {
            //     if(i->cost >= local_best)elim1++;
            //     else sum1 += i->cost;
            // }
            //
            // if(elim0>elim1)return BEGIN_ORDER;
            // if(elim0<elim1)return END_ORDER;

            return (sum0>=sum1)?BEGIN_ORDER:END_ORDER;
        }
        case 3:
        {
            int min=INT_MAX;
            int minBegin = 0, minEnd = 0;
            int elimBegin = 0, elimEnd = 0;

            //find min lower bound
            for (auto const& i: begin)
                if(i->cost < min && i->cost > 0)min=i->cost;
            for (auto const& i: end)
                if(i->cost < min && i->cost > 0)min=i->cost;

            for (auto const& i: begin){
                if(i->cost == min)minBegin++;
                if(i->cost >= initialUB)elimBegin++;
            }
            for (auto const& i: end){
                if(i->cost == min)minEnd++;
                if(i->cost >= initialUB)elimEnd++;
            }
            // std::cout<<minBegin<<" "<<minEnd<<" "<<elimBegin<<" "<<elimEnd<<std::endl;

            //where min LB is realized less often...
            //...else where more nodes pruned
            if(minBegin>minEnd)retval = END_ORDER;
            else if(minBegin<minEnd)retval = BEGIN_ORDER;
            else (elimBegin>elimEnd)?BEGIN_ORDER:END_ORDER;

            break;
        }


    }

    // int nb1 = 0;

    return retval;
}

//dynamic branching policies : choose between two sets of bounds;
int
Tree::chooseBranching(int *cb,int *ce)
{
    int retval=BEGIN_ORDER;

    switch (arguments::branchingMode) {
        // case -3:
        // {
        //     retval=(begin.size()%2)?BEGIN_ORDER:END_ORDER;
        //     break;
        // }
        // case -2:
        // {
        //     retval=END_ORDER;
        //     break;
        // }
        // case -1:
        // {
        //     retval=BEGIN_ORDER;
        //     break;
        // }
        case 1:
        {
            int sum = 0;
            for (int i = 0; i < pbb->size; ++i) {
                sum += (cb[i]-ce[i]);
            }

            retval = (sum < 0)?END_ORDER:BEGIN_ORDER; //choose larger sum, tiebreak : FRONT
            break;
        }
        case 2:
        {
            //count eliminated - based on initial UB... don't use current UB in distributed parallelBB !
            int ub=local_best;
            if(!arguments::singleNode)ub=initialUB;

            int elim = 0;
            int sum = 0;

            for (int i = 0; i < pbb->size; ++i) {
                if (cb[i]>=ub)elim++; //"pruned"
                else sum += cb[i];
                if (ce[i]>=ub)elim--; //"pruned"
                else sum -=ce[i];
            }
            //take set with lss open nodes / tiebreaker: greater average LB
            if(elim > 0)retval = BEGIN_ORDER;
            else if(elim < 0)retval = END_ORDER;
            else retval = (sum < 0)?END_ORDER:BEGIN_ORDER;
            break;
        }
        case 3:
        {
            int ub=local_best;
            if(!arguments::singleNode)ub=initialUB;

            int min=INT_MAX;
            int minCount=0;
            int elimCount=0;

            //find min lower bound
            for (int i = 0; i < pbb->size; ++i) {
                if(cb[i]<min && cb[i]>0)min=cb[i];
                if(ce[i]<min && ce[i]>0)min=ce[i];
            }
            //how many times lowest LB realized?
            for (int i = 0; i < pbb->size; ++i) {
                if(cb[i]==min)minCount++;
                if(ce[i]==min)minCount--;
                if (cb[i]>=ub)elimCount++;
                if (ce[i]>=ub)elimCount--;
            }
            //take set where min LB is realized LESS often
            if(minCount > 0)retval = END_ORDER;
            else if(minCount < 0)retval = BEGIN_ORDER;
            else retval = (elimCount > 0)?BEGIN_ORDER:END_ORDER;//break ties

            break;
        }
    }

    // int nb1 = 0;

    return retval;
}


//decompose and bound nodes
//generate children nodes and evaluate
std::vector<subproblem *>
Tree::decompose(subproblem& n)
{
    subproblem * tmp;

    if (n.simple()) { //2 solutions ...
        std::vector<subproblem *> leaves;
        FILE_LOG(logDEBUG4) << "is simple";

        tmp        = new subproblem(n, n.limit1 + 1, BEGIN_ORDER);
        tmp->cost=lb[SIMPLE]->evalSolution(tmp->schedule);
        leaves.push_back(tmp);

        tmp        = new subproblem(n, n.limit1+2 , BEGIN_ORDER);
        tmp->cost=lb[SIMPLE]->evalSolution(tmp->schedule);
        leaves.push_back(tmp);

        return leaves;
    } else {
        std::vector<subproblem *> begin;
        std::vector<subproblem *> end;

        //use temporary subproblem to avoid realloc ...
        tmp=new subproblem(n);

        int costs[2];
        for (int j = n.limit1 + 1; j < n.limit2; j++) {
            //slower alternative...
            // begin.emplace_back(new subproblem(n, j, BEGIN_ORDER));
            // tmp = begin.back();
            // lb[SIMPLE]->bornes_calculer(tmp->schedule,tmp->limit1,tmp->limit2,costs,local_best);
            // tmp->cost=costs[0];

            //generate tmp child node BEGIN
            tmp->limites_set(n, BEGIN_ORDER);
            tmp->permutation_set(n, j, BEGIN_ORDER);

            //only insert promising nodes
            if(costs[0]<local_best){
                tmp->cost=costs[0];
                begin.emplace_back(new subproblem(*tmp));
            }

            // end.emplace_back(new subproblem(n, j, END_ORDER));
            // tmp = end.back(); //new subproblem(n, j, END_ORDER);
            // lb[SIMPLE]->bornes_calculer(tmp->schedule,tmp->limit1,tmp->limit2,costs,local_best);
            // tmp->cost=costs[0];

            //generate tmp child node END
            tmp->limites_set(n, END_ORDER);
            tmp->permutation_set(n, j, END_ORDER);

            if(costs[0]<local_best){
                tmp->cost=costs[0];
                end.emplace_back(new subproblem(*tmp));
            }
        }

        int dir=chooseBranching(begin, end);
        if(dir==BEGIN_ORDER){
            FILE_LOG(logDEBUG4) << "keep begin";
            del(end);
            return begin;
        }else{
            FILE_LOG(logDEBUG4) << "keep end";
            del(begin);
            return end;
        }
    }
}

//decompose and bound nodes
//FSP-specific optimization: function takes advantage of computing all children of a node at once. Compute all LBs of children nodes in O(mn) steps instead of O(mn^2)
// void
std::vector<subproblem*>
Tree::decompose_fast(subproblem& n)
{
    std::vector<subproblem *>children;

    //temporary for evaluation
    subproblem * tmp;

    if (n.simple()) { //2 solutions ...
        tmp        = new subproblem(n, n.limit1 + 1, BEGIN_ORDER);
        tmp->cost=lb[SIMPLE]->evalSolution(tmp->schedule);
        children.push_back(tmp);

        tmp        = new subproblem(n, n.limit1+2 , BEGIN_ORDER);
        tmp->cost=lb[SIMPLE]->evalSolution(tmp->schedule);
        children.push_back(tmp);
        // insert(tmp);
    } else {
        int *costsFwd = new int[pbb->size];
        int *costsBwd = new int[pbb->size];

        //get costs of children subproblems
        lb[SIMPLE]->boundChildren(n.schedule,n.limit1,n.limit2,costsFwd,costsBwd,prioFwd,prioBwd);
        //choose branching direction
        int dir=chooseBranching(costsFwd, costsBwd);

        if(dir==BEGIN_ORDER){
            for (int j = n.limit1 + 1; j < n.limit2; j++) {
                int job = n.schedule[j];
                if(costsFwd[job]<local_best){
                    tmp = new subproblem(n, j, BEGIN_ORDER);
                    tmp->cost=costsFwd[job];

                    children.push_back(tmp);
                }
            }
        }else{
            for (int j = n.limit2 - 1; j > n.limit1; j--) {
                int job = n.schedule[j];
                if(costsBwd[job]<local_best){
                    tmp = new subproblem(n, j, END_ORDER);
                    tmp->cost=costsBwd[job];

                    children.push_back(tmp);
                }
            }
        }

        delete[] costsFwd;
        delete[] costsBwd;
    }
    return children;
}

void
Tree::insert(std::vector<subproblem *>&ns)
{
    //no children (decomposition avoid generation of unpromising children)
    if (!ns.size())
        return;

    //children inserted with push_back [ 1 2 3 ... ]
    //for left->right exploration, insert (push) in reverse order
    for (auto i = ns.rbegin(); i != ns.rend(); i++) {
        if ((*i)->cost >= local_best) {
            FILE_LOG(logDEBUG4)<<(*i)->cost<<"\t"<<local_best<<"\t => delete";
            delete (*i);
            continue;
        } else  {
            FILE_LOG(logDEBUG4)<<"insert "<<*(*i);
            push(std::move(*i));
        }
    }
}

void
Tree::insert(subproblem * n)
{
    push(n);
}

void
Tree::del(std::vector<subproblem *>&ns)
{
    for(auto i:ns)
        delete i;
    ns.clear();
}


static int count=0;
void
Tree::clearPool()
{
    while(!empty()){
        delete top();
        pop();
    }
}
