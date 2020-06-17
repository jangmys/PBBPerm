#include "../../common/headers/arguments.h"

//INCLUDE INSTANCES
#include "../../bounds/headers/libbounds.h"

// #include "../headers/matrix_controller.h"
#include "../../common/headers/pbab.h"
#include "../../common/headers/solution.h"
#include "../../common/headers/ttime.h"
#include "../../common/headers/log.h"

// only for GPU version
#ifdef USE_GPU
#include "../headers/gpubb.h"
#endif


int
main(int argc, char ** argv)
{
    // initializions...
    arguments::readIniFile();
    arguments::parse_arguments(argc, argv);
    arguments::initialize();

    pbab * pbb = new pbab();//, bound1, bound2);

    FILELog::ReportingLevel() = logINFO;
    FILE* log_fd = fopen( "./logs/bblog.txt", "w" );
    Output2FILE::Stream() = log_fd;

    struct timespec tstart, tend;
    clock_gettime(CLOCK_MONOTONIC, &tstart);

    pbb->buildInitialUB();
    // if(arguments::init_mode==0){
    //     FILE_LOG(logINFO) << "Initializing at optimum " << arguments::initial_ub;
    //     FILE_LOG(logINFO) << "Guiding solution " << *(pbb->sltn);
    //     pbb->sltn->bestcost = arguments::initial_ub;
    // }else{
    //     FILE_LOG(logINFO) << "Start search with heuristic solution\n" << *(pbb->sltn);
    // }


    // ###############################
    // ###### SINGLE NODE ######## (no MPI)
    // ###############################
    if (arguments::singleNode) {
        printf("=== SINGLE NODE MODE\n");
        printf("=== solving %s / instance %s\n", arguments::problem, arguments::inst_name);

        #ifdef USE_GPU
            cudaFree(0);

            int device_nb = 0;
            cudaSetDevice(device_nb);

            int device,count;
            cudaGetDeviceCount(&count);
            cudaGetDevice(&device);
            printf("=== Device %d/%d ==\n", device, count-1);

            cudaDeviceSetCacheConfig(cudaFuncCachePreferShared);

            gpubb* gbb = new gpubb(pbb);//%numDevices);
            gbb->initialize();// allocate IVM on host/device
#ifdef FSP
            printf("=== FSP\n");fflush(stdout);
            gbb->initializeBoundFSP();
#endif
#ifdef TEST
            printf("\n=== TEST\n");fflush(stdout);
            gbb->initializeBoundTEST();
#endif
            gbb->copyH2D();
            gbb->initFullInterval();

            gbb->next();

            gbb->printStats();
            delete gbb;
#else
            printf("GPU mode : compile with -DUSE_GPU! Aborting...\n");
#endif
    }else{
        printf("=== ENABLE SINGLE NODE MODE!\n");
    }

    clock_gettime(CLOCK_MONOTONIC, &tend);
    printf("\nWalltime :\t %2.8f\n", (tend.tv_sec - tstart.tv_sec) + (tend.tv_nsec - tstart.tv_nsec) / 1e9f);

    return EXIT_SUCCESS;
} // main
