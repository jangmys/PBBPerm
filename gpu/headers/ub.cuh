__device__ void tile_localSearch(thread_block_tile<32> g, int *schedule,int l1, int l2)
{
    int lane = g.thread_rank();
    int cmax = 0;


    for(int i=l1+1; i<l2; i++){

        // for(int j=0; )

        cmax = INT_MAX;
        for(int j=i+1+lane; j<l2; j+=g.size()){
            cmax = thread_evalSol_d(schedule, i, j);
        }
        cmax = tile_min(g,cmax);
    }


}




__global__ void sampleSolutions(const int *pos, const int *end, const int *dir, const int *mat, const int* state, int *schedules){
    thread_block_tile<32> g = tiled_partition<32>(this_thread_block());

    int ivm = (blockIdx.x * blockDim.x + threadIdx.x) / g.size(); // global ivm id
    int lane = g.thread_rank();
    int warpID = threadIdx.x / g.size();

    extern __shared__ int sharedSol[];
    int *l1      = (int *)&sharedSol[4 * size_d];
    int *l2      = (int *)&l1[4];

    if(ivm < nbIVM_d){
        tile_localSearch(g, &schedules[ivm*size_d],-1, size_d);
    }

    // int depth = firstSplit(&pos[ivm*size_d],&end[ivm*size_d]);


}

__global__ void searchSolutions(const int *permutations, int *makespans, const int best)
{
    thread_block_tile<32> g = tiled_partition<32>(this_thread_block());

    int ivm = (blockIdx.x * blockDim.x + threadIdx.x) / g.size(); // global ivm id
    int warpID = threadIdx.x / g.size();

    extern __shared__ int sharedMem[];
    int *prmu = (int*)&sharedMem;

    // load permutation to smem
    prmu += warpID * size_d;
    for(int i = g.thread_rank(); i<size_d; i+=g.size())
    {
        prmu[i]=permutations[ivm*size_d+i];
    }
    g.sync();

    int swap1,swap2,cost;
    int bestCost=INT_MAX;
    int bestInd=0;

    for(int i=g.thread_rank(); i<_nbJobPairs; i+=g.size())
    {
        swap1 = _jobPairs[i];
        swap2 = _jobPairs[_nbJobPairs+i];
        cost = thread_evalSol_insert_d(prmu, swap1, swap2);
        if(cost<bestCost)
        {
            bestCost=cost;
            bestInd=i;
        }
    }
    tile_minloc(g,bestCost,bestInd);
    bestCost = g.shfl(bestCost,0);
    bestInd = g.shfl(bestInd,0);
    g.sync();

    if(g.thread_rank() == 0)
    {
        swap1 = _jobPairs[bestInd];
        swap2 = _jobPairs[_nbJobPairs+bestInd];

        int tmp=prmu[swap2];
        for(int i=swap2;i>swap1;i--)
        {
            prmu[i]=prmu[i-1];
        }
        prmu[swap1]=tmp;

        // swap_d(&prmu[swap1],&prmu[swap2]);

        makespans[ivm]=bestCost;
    }
}
