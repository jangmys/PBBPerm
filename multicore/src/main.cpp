#include "../../common/include/arguments.h"
#include "../../common/include/pbab.h"
#include "../../common/include/ttime.h"
#include "../../common/include/solution.h"
#include "../../common/include/log.h"

//INCLUDE INSTANCES
#include "../../bounds/include/libbounds.h"
#include "../include/matrix_controller.h"
#include "../include/sequentialbb.h"

int
main(int argc, char ** argv)
{
    strcpy(arguments::inifile,"./mcconfig.ini");

    // initializions...
    arguments::readIniFile();
    arguments::parse_arguments(argc, argv); //overrides arguments from IniFile
    arguments::initialize();


	//multicoreBB as a worker in distributed execution behaves differently...
	if(!arguments::singleNode){
        std::cout<<"Enable single-node mode!\n"<<std::endl;
        exit(0);
    }else{
        printf("=== solving %s / instance %s\n", arguments::problem, arguments::inst_name);
    }

    FILELog::ReportingLevel() = logERROR;//INFO;
    FILE* log_fd = fopen( "./mc.log", "w" );
    Output2FILE::Stream() = log_fd;

	pbab * pbb = new pbab();

    //set initial solution
    pbb->buildInitialUB();


	struct timespec tstart, tend;
    // if(arguments::init_mode==-2){
	// 	pbb->buildInitialUB();
    //     printf("Initial Solution:\n");
    //     pbb->sltn->print();
    // }
    // if(arguments::init_mode==0){
    //     pbb->sltn->bestcost=arguments::initial_ub;
    // }
    // pbb->root_sltn = pbb->sltn;

	std::cout<<"Initial solution:\n"<<*(pbb->sltn)<<"\n";

	// int bbmode=0;
	// bool sequential=true;

    enum algo{seqbb=1,heuristic=2,multicore=3,increaseLB=4};

    // int choice=seqbb;
    // int choice=heuristic;
    int choice=multicore;

    switch(choice){
        case seqbb:
        {
            std::cout<<"=== Single-threaded IVM-BB\n";

            sequentialbb *sbb=new sequentialbb(pbb);

            sbb->setRoot(pbb->root_sltn->perm);

            sbb->initFullInterval();
            clock_gettime(CLOCK_MONOTONIC, &tstart);
            while(sbb->next());
            clock_gettime(CLOCK_MONOTONIC, &tend);

            delete sbb;
            break;
        }
        case heuristic:
        {
            std::cout<<"=== Incomplete Single-threaded search\n";

            sequentialbb *sbb=new sequentialbb(pbb);

            clock_gettime(CLOCK_MONOTONIC, &tstart);
            for(int i=0;i<10;i++){
                pbb->foundSolution=false;

                std::cout<<"RESTART "<<*(pbb->sltn)<<"\n";

                sbb->setRoot(pbb->sltn->perm);
                sbb->initFullInterval();
                // while(!pbb->foundSolution)
                    // sbb->next();
                while(sbb->next());

                // pbb->ils->localSearchKI(pbb->sltn->bestpermut,sqrt(pbb->size));

            }
            clock_gettime(CLOCK_MONOTONIC, &tend);

            delete sbb;
            break;
        }
        case multicore:
        {
            std::cout<<"=== Multi-core exploration...\n";
            clock_gettime(CLOCK_MONOTONIC, &tstart);
			//
            matrix_controller *mc = new matrix_controller(pbb);
            mc->initFullInterval();
            mc->next();
            delete mc;
            clock_gettime(CLOCK_MONOTONIC, &tend);

            break;
        }
        case increaseLB:
        {
            clock_gettime(CLOCK_MONOTONIC, &tstart);
            matrix_controller *mc = new matrix_controller(pbb);

            int c=arguments::initial_ub;
            while(!pbb->foundSolution)
            {
                pbb->reset();
                pbb->sltn->cost=c++;

                mc->initFullInterval();
                if(!mc->solvedAtRoot())
                    mc->next();
                else
                    FILE_LOG(logINFO) << "solved at root";

                    // mc->resetExplorationState();

                pbb->printStats();
            }
            clock_gettime(CLOCK_MONOTONIC, &tend);
        }
    }

	pbb->printStats();

    // clock_gettime(CLOCK_MONOTONIC, &tend);
    printf("\nWalltime :\t %2.8f\n", (tend.tv_sec - tstart.tv_sec) + (tend.tv_nsec - tstart.tv_nsec) / 1e9f);

    return EXIT_SUCCESS;
} // main
