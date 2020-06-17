#ifndef TREEHEURISTIC_H_
#define TREEHEURISTIC_H_


#include "tree.h"

class treeheuristic : public Tree
{
public:
    treeheuristic(unsigned int i, pbab* pbb) : Tree(i,pbb)
    {
        iter=0;
        foundSolution=false;
        bestFound=new subproblem(pbb->size);

        std::string str(arguments::inifile);
        INIReader reader(str);

        ls=new IG(pbb->instance);

        sec_ulimit=reader.GetInteger("heuristic", "treeHeuristicMaxTime", 10);
        node_ulimit=reader.GetInteger("heuristic", "treeHeuristicMaxNodes", 100000);
    };
    ~treeheuristic(){};

    IG* ls;

    bool next();
    void improve(subproblem * pr);
    void insert(std::vector<subproblem *>&ns);

    void restartWithBest();

    int run(subproblem* s,int _ub);

    bool foundSolution;
    unsigned long long int iter;
    subproblem *bestFound;

    unsigned int node_ulimit;
    unsigned int sec_ulimit;
};


#endif
