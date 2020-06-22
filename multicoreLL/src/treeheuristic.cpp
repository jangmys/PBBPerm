#include "log.h"
#include "pbab.h"
#include "solution.h"

#include "treeheuristic.h"

bool
treeheuristic::next()
{
    bool continuer = false;

    // FILE_LOG(logDEBUG)<<"poolsize "<<size()<<"\t"<<local_best;

    subproblem * n;
    if (!empty()) {
        n = take();

        // FILE_LOG(logDEBUG)<<"take node "<<n->depth<<"\t"<<n->ub<<"\t"<<size();
        FILE_LOG(logDEBUG)<<"take node "<<*n<<"\t"<<local_best;

        if(bound(n)){
            if (n->leaf()) {
				n->ub=n->cost;
                improve(n); //leaf with better cost than UB
                restartWithBest(); //
            } else  {
                std::vector<subproblem*>ns;
                ns=decompose_fast(*n);

                // for DEQUE
                // for(auto i:ns)
                // {
                //     i->ub=lb[SIMPLE]->evalSolution(i->schedule);
                // }
                // std::sort(ns.begin(),ns.end(),
                //     [](subproblem* a,subproblem* b){
                //         return a->ub < b->ub;
                //     }
                // );
                // for(auto i:ns)
                // {
                //     std::cout<<i->ub<<" ";
                // }
                // std::cout<<"\n";

                insert(ns);
            }
        }
        continuer = true;
        delete(n);

        // FILE_LOG(logDEBUG)<<"in pool: "<<size();
    }

    if(empty())
    {
        FILE_LOG(logINFO)<<"Pool empty... "<<local_best<<" is optimal.";
        return continuer;
    }

    if(size()>node_ulimit){
        // FILE_LOG(logINFO)<<"RESTART";
        // FILE_LOG(logINFO)<<"TOP "<<top()->cost<<" "<<top()->ub<<" "<<local_best;

        // subproblem* s=new subproblem(*bestFound);
        subproblem* s=new subproblem(*top());
        clearPool();

        // std::cout<<"RESTART";
        // std::cout<<"\t"<<*s<<std::endl;

        setRoot(s->schedule);
        // std::cout<<"\t\t"<<*bestFound<<std::endl;
        delete s;
    }

    return continuer;
}

void
treeheuristic::restartWithBest()
{
    clearPool();
    setRoot(bestFound->schedule);
}

void
treeheuristic::improve(subproblem * pr)
{
    // std::cout<<pr->ub<<std::endl;
    // FILE_LOG(logDEBUG)<<"Improved:\t"<<*pr;

    if(pr->ub < local_best)
        local_best = pr->ub;

    *bestFound = *pr;
    foundSolution = true;
}

void
treeheuristic::insert(std::vector<subproblem *>&ns)
{
    if (!ns.size())
        return;

    for (auto i = ns.rbegin(); i != ns.rend(); i++) {
        if ((*i)->cost >= local_best) {
            // FILE_LOG(logDEBUG)<<(*i)->cost<<"\t"<<local_best<<"\t => delete";
            delete (*i);
            continue;
        } else  {
            // FILE_LOG(logDEBUG4)<<"insert "<<*(*i);
            (*i)->ub = lb[SIMPLE]->evalSolution((*i)->schedule);

			if((*i)->limit2-(*i)->limit1 > 10)
				(*i)->ub=ls->localSearch((*i)->schedule, (*i)->limit1+1, (*i)->limit2);

            // FILE_LOG(logDEBUG)<<"child "<<(*i)->ub;
            // FILE_LOG(logDEBUG)<<"child "<<*(*i);
            push(std::move(*i));
        }
    }

    if(top()->ub < bestFound->ub)
    {
        FILE_LOG(logDEBUG)<<"RESTART AFTER INSERT";
        top()->cost = top()->ub;
        improve(top());
        restartWithBest();
        // exit(1);
    }
}

int
treeheuristic::run(subproblem* s, int _ub)
{
    strategy=PRIOQ;

    *bestFound=*s;
    bestFound->ub = lb[SIMPLE]->evalSolution(bestFound->schedule);
    // local_best=bestFound->ub;
    local_best=_ub;

    iter=0; //stopping criterion

    setRoot(bestFound->schedule);

    struct timespec startt,nowt;
    clock_gettime(CLOCK_MONOTONIC, &startt);

    FILE_LOG(logDEBUG)<<"Starting "<<size()<<" | "<<*top()<<" "<<_ub<<" "<<bestFound->ub;

    while(1){
        if(!next())break;

        if(++iter>100)
        {
            clock_gettime(CLOCK_MONOTONIC, &nowt);
            if((nowt.tv_sec - startt.tv_sec) > sec_ulimit)
                break;
            iter=0;
        }
    }

    clearPool();

    // bestFound->ub = lb[SIMPLE]->evalSolution(bestFound->schedule);
    *s=*bestFound;
    FILE_LOG(logDEBUG)<<"Return "<<*s;

    return s->ub;
}
