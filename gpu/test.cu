#include "../common/headers/arguments.h"

//INCLUDE INSTANCES
#include "../bounds/headers/libbounds.h"

#include "../common/headers/pbab.h"
#include "../common/headers/solution.h"
#include "../common/headers/ttime.h"
#include "../common/headers/log.h"

#include <cooperative_groups.h>
using namespace cooperative_groups;

#include "./headers/gpuerrchk.h"
#include "./headers/gpu_helper.cuh"
#include "./headers/gpu_fsp_bound_weak.cuh"


void initializeFSP_LB(pbab* pbb)
{
    (pbb->instance->data)->seekg(0);
    (pbb->instance->data)->clear();

    *(pbb->instance->data) >> nbJob_h;
    *(pbb->instance->data) >> nbMachines_h;

    allocate_host_bound_tmp();

    for (int i = 0; i < nbMachines_h; i++) {
        for (int j = 0; j < nbJob_h; j++)
            *(pbb->instance->data) >> tempsJob_h[i * nbJob_h + j];
        fillMinTempsArrDep();
        fillSumPT();
    }

    gpuErrchk(cudaMemcpyToSymbol(_sumPT, sumPT_h, nbMachines_h * sizeof(int)));
    gpuErrchk(cudaMemcpyToSymbol(_minTempsDep, minTempsDep_h, nbMachines_h * sizeof(int)));
    gpuErrchk(cudaMemcpyToSymbol(_minTempsArr, minTempsArr_h, nbMachines_h * sizeof(int)));
    gpuErrchk(cudaMemcpyToSymbol(_tempsJob, tempsJob_h, nbJob_h * nbMachines_h * sizeof(int)));

    gpuErrchk(cudaMemcpyToSymbol(_nbMachines, &nbMachines_h, sizeof(int)));
    gpuErrchk(cudaMemcpyToSymbol(_nbJob, &nbJob_h, sizeof(int)));

    // gpuErrchk(cudaMemcpyToSymbol(_sum, &somme_h, sizeof(int)));
    gpuErrchk(cudaMemcpyToSymbol(size_d, &nbJob_h, sizeof(int)));
    // gpuErrchk(cudaMemcpyToSymbol(_nbJobPairs, &nbJobPairs_h, sizeof(int)));

    free_host_bound_tmp();
}

int main(int argc,char *argv[])
{
    arguments::readIniFile();
    arguments::parse_arguments(argc, argv);
    arguments::initialize();

    pbab * pbb = new pbab();

    FILELog::ReportingLevel() = logINFO;
    FILE* log_fd = fopen( "./logs/test.txt", "w" );
    Output2FILE::Stream() = log_fd;

    //========================================
    //host lower bound
    bound_abstract * bound;
    if (arguments::problem[0] == 'f') {
        bound = new bound_fsp_weak();
        bound->set_instance(pbb->instance);
        bound->init(arguments::branchingMode,0,0);
    }
    //======================================

    //init device, allocate memory...
    gpuErrchk(cudaSetDevice(0));
    gpuErrchk(cudaFree(0));
    initializeFSP_LB(pbb);

    int *schedule;
    int *lim1;
    int *costs;
    int *state;
    int nbIVM = arguments::nbivms_gpu;

	gpuErrchk(cudaMallocManaged(&schedule,nbIVM*nbJob_h*sizeof(int)));
	gpuErrchk(cudaMallocManaged(&costs,nbIVM*2*nbJob_h*sizeof(int)));
	gpuErrchk(cudaMallocManaged(&lim1,nbIVM*sizeof(int)));
	gpuErrchk(cudaMallocManaged(&state,nbIVM*sizeof(int)));

    int nbChildren=0;
    //creating some subproblems (only begin-scheduling)
    for(int i=0;i<nbIVM;i++)
    {
        for(int j=0;j<nbJob_h;j++){
            schedule[i*nbJob_h+j]=j;
            costs[i*2*nbJob_h+j]=0;
        }
        helper::shuffle(schedule,nbJob_h);
        state[i]=1;
        lim1[i]=helper::intRand(-1,nbJob_h-5);//-1 means no job scheduled...

        nbChildren += (nbJob_h-lim1[i]-1);
    }

    struct timespec startt,endt;
    clock_gettime(CLOCK_MONOTONIC,&startt);

    //use 32 threads per subproblem to evaluate all children nodes
    int NN=4;
    size_t smem = (NN * (nbJob_h + 3 * nbMachines_h)) * sizeof(int);
    boundWeak_Begin << < (nbIVM+NN-1) / NN, NN * 32, smem >>>
    (schedule, lim1, costs, state);

    gpuErrchk( cudaPeekAtLastError() );
    gpuErrchk( cudaDeviceSynchronize() );

    clock_gettime(CLOCK_MONOTONIC,&endt);
    auto t=(endt.tv_sec-startt.tv_sec)+(endt.tv_nsec-startt.tv_nsec)/1e9;
    std::cout<<"Computed " <<nbChildren << " Child subproblems in: "<<t<<" sec"<<std::endl;
    std::cout<<"Per LB: " <<(float)nbChildren/t<<std::endl;

    //===============================
    clock_gettime(CLOCK_MONOTONIC,&startt);

    boundWeak_Begin_One <<< (nbIVM+127) / 128, 128 >>> (schedule, lim1, costs, state);
    gpuErrchk( cudaPeekAtLastError() );
    gpuErrchk( cudaDeviceSynchronize() );

    clock_gettime(CLOCK_MONOTONIC,&endt);
    t=(endt.tv_sec-startt.tv_sec)+(endt.tv_nsec-startt.tv_nsec)/1e9;
    std::cout<<"Computed " <<nbIVM << " Parent subproblems in: "<<t<<" sec"<<std::endl;
    std::cout<<"Per LB: " <<(float)nbIVM/t<<std::endl;

    int *costsCPU=new int[nbIVM];

    int c[2];
    for(int i=0;i<nbIVM;i++)
    {
        bound->bornes_calculer(&schedule[i*nbJob_h], lim1[i], nbJob_h,c, 999999);
        costsCPU[i]=c[0];
    }

    bool ok=true;
    for(int i=0;i<nbIVM;i++)
    {
        ok &= (costs[i]==costsCPU[i]);
    }
    if(!ok)
    {
        for(int i=0;i<nbIVM;i++)
            printf("%2d\t %2d %4d\t %4d\n",i,lim1[i],costs[i],costsCPU[i]);
    }else{
        printf("test passed");
    }
    printf("\n");


    free(costsCPU);
    cudaFree(schedule);
    cudaFree(costs);
    cudaFree(lim1);
    cudaFree(state);

}
