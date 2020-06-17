#include <sys/sysinfo.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <atomic>

#include "arguments.h"
#include "pbab.h"
#include "ttime.h"
#include "solution.h"
#include "log.h"

#include "../../bounds/include/libbounds.h"
#include "tree_controller.h"
#include "tree.h"
#include "treeheuristic.h"

int
main(int argc, char ** argv)
{
    // initializions...
    arguments::readIniFile();
    arguments::parse_arguments(argc, argv);
    arguments::initialize();

    pbab * pbb = new pbab();

	//create logfile
    FILELog::ReportingLevel() = logDEBUG; //INFO; //logDEBUG;
    FILE* log_fd = fopen( "./logs/lllog.txt", "w" );
    Output2FILE::Stream() = log_fd;

    struct timespec tstart, tend;

    //initial upper bound
    clock_gettime(CLOCK_MONOTONIC, &tstart);
    pbb->buildInitialUB();
    clock_gettime(CLOCK_MONOTONIC, &tend);

    if(arguments::init_mode==0){
        FILE_LOG(logINFO) << "Initializing at optimum " << arguments::initial_ub;
        FILE_LOG(logINFO) << "Guiding solution " << *(pbb->sltn);
        pbb->sltn->cost = arguments::initial_ub;
    }else{
        FILE_LOG(logINFO) << "Start search with heuristic solution\n" << *(pbb->sltn);
    }
    pbb->root_sltn = pbb->sltn;

    FILE_LOG(logINFO) << "TIME(BuildInitialUB):\t"<<
        (tend.tv_sec - tstart.tv_sec) + (tend.tv_nsec - tstart.tv_nsec) / 1e9f;

// ###############################
// ###### SINGLE NODE ########
// ###############################
    enum algo{seqbb=1,heuristic=2,multicore=3};
    // int choice=seqbb;
    int choice=heuristic;
    // int choice=multicore;

    switch (choice) {
        case seqbb:
        {
            FILE_LOG(logINFO)<<"Sequential LL exploration";
            clock_gettime(CLOCK_MONOTONIC, &tstart);

            Tree *tr=new Tree(0,pbb);
            tr->strategy=DEQUE;
            tr->setRoot(pbb->sltn->perm);
            while(tr->next());

            delete tr;
            clock_gettime(CLOCK_MONOTONIC, &tend);
            FILE_LOG(logINFO)<<"Done";
            pbb->printStats();
            break;
        }
        case heuristic:
        {
            FILE_LOG(logINFO)<<"LL Heuristic Tree Search";
            clock_gettime(CLOCK_MONOTONIC, &tstart);

            treeheuristic *th=new treeheuristic(0,pbb);
            th->strategy=PRIOQ;

            subproblem *s=new subproblem(pbb->size);
            for(int i=0;i<pbb->size;i++){
                s->schedule[i]=pbb->sltn->perm[i];
            }
            s->ub = pbb->sltn->cost;

            int c = th->run(s,pbb->sltn->cost);

            std::cout<<"THsol "<<c<<"\t"<<*s<<"\n";

            delete th;
            clock_gettime(CLOCK_MONOTONIC, &tend);
            break;
        }
        case multicore:
        {
            clock_gettime(CLOCK_MONOTONIC, &tstart);
            printf(" === SINGLE NODE MODE\n");
            printf(" === solving %s / instance %s\n",arguments::problem,arguments::inst_name);

            TreeController * tc = NULL;
            tc = new TreeController(pbb);
            tc->next();
            clock_gettime(CLOCK_MONOTONIC, &tend);
            pbb->printStats();
            break;
        }
    }

    std::cout << "TIME(Exploration):\t"<<
        (tend.tv_sec - tstart.tv_sec) + (tend.tv_nsec - tstart.tv_nsec) / 1e9f << "\n";

    return 0;// exit(0);
} // main
