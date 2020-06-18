#include "../../common/include/arguments.h"
#include "../../common/include/pbab.h"
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

	arguments::singleNode=true;

	//multicoreBB as a worker in distributed execution behaves differently...
    printf("=== solving %s / instance %s\n", arguments::problem, arguments::inst_name);

    FILELog::ReportingLevel() = logERROR;//INFO;
    FILE* log_fd = fopen( "./mc.log", "w" );
    Output2FILE::Stream() = log_fd;

	pbab * pbb = new pbab();

    //set initial solution
    pbb->buildInitialUB();


	std::cout<<"Initial solution:\n"<<*(pbb->sltn)<<"\n";

    enum algo{seqbb=1,multicore=2,heuristic=3,increaseLB=4};

	int choice=multicore;
	if(arguments::nbivms_mc==1)
		choice=seqbb;

	// experimental....
	// int choice=heuristic;
    // int choice=increaseLB;

	struct timespec tstart, tend;
	clock_gettime(CLOCK_MONOTONIC, &tstart);
    switch(choice){
        case seqbb:
        {
            std::cout<<"=== Single-threaded IVM-BB\n";

            sequentialbb *sbb=new sequentialbb(pbb);

            sbb->setRoot(pbb->root_sltn->perm);
            sbb->initFullInterval();

            while(sbb->next());

            delete sbb;
            break;
        }
		case multicore:
		{
			std::cout<<"=== Multi-core exploration...\n";

			matrix_controller *mc = new matrix_controller(pbb);
			mc->initFullInterval();
			mc->next();

			delete mc;
			break;
		}
        case heuristic:
        {
            std::cout<<"=== Incomplete Single-threaded search\n";

            sequentialbb *sbb=new sequentialbb(pbb);

            for(int i=0;i<10;i++){
                pbb->foundSolution=false;

                std::cout<<"RESTART "<<*(pbb->sltn)<<"\n";

                sbb->setRoot(pbb->sltn->perm);
                sbb->initFullInterval();
                while(sbb->next());
            }

            delete sbb;
            break;
        }
        case increaseLB:
        {
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
        }
    }
	clock_gettime(CLOCK_MONOTONIC, &tend);

	pbb->printStats();

    // clock_gettime(CLOCK_MONOTONIC, &tend);
    printf("\nWalltime :\t %2.8f\n", (tend.tv_sec - tstart.tv_sec) + (tend.tv_nsec - tstart.tv_nsec) / 1e9f);

    return EXIT_SUCCESS;
} // main
