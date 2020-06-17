// #include <cuda.h>
// #include <cuda_runtime.h>
// #include <cuda_runtime_api.h>
//
// //#define MAXMACH 40
// //#define MAXSOMME 780 //M*(M-1)/2
// #define MAXSOMME 190 //M*(M-1)/2

#define TILE_SZ 32
#define MAXJOBS 800
#define MAXMACH 20


// // constant GPU data
__device__ int _nbJob;
__device__ int _nbMachines;

// __device__ __constant__ int _sum;
// __device__ __constant__ int _nbJobPairs;
//
__device__ __constant__ int _sumPT[MAXMACH];            //20
__device__ __constant__ int _minTempsArr[MAXMACH];            //20
__device__ __constant__ int _minTempsDep[MAXMACH];            //20
__device__ __constant__ int _tempsJob[MAXJOBS * MAXMACH];

// __device__ int root_d[MAXJOBS];
// __device__ int root_dir_d;


// #if MAXJOBS <= 400
// __device__ __constant__ int _tempsJob[MAXJOBS * MAXMACH];
// #elif
// __device__ int _tempsJob[MAXJOBS * MAXMACH];     //10000
// #endif

// #elif MAXJOBS == 50
// __device__ __constant__ int _tempsJob[MAXJOBS * MAXMACH];     //1000
// #elif MAXJOBS == 100
// __device__ __constant__ int _tempsJob[MAXJOBS * MAXMACH];     //2000
// #elif MAXJOBS == 200
// __device__ __constant__ int _tempsJob[MAXJOBS * MAXMACH];     //4000
// #elif MAXJOBS >= 300
// __device__ int _tempsJob[MAXJOBS * MAXMACH];     //10000
// #endif
//
// __device__ int _jobPairs[MAXJOBS*(MAXJOBS-1)];
// __device__ int freqTable_d[MAXJOBS * MAXJOBS];
//
// // bounding
int nbMachines_h;
int nbJob_h;
// int somme_h;
// int nbJobPairs_h;
//
int *tempsJob_h;
// // int *tabJohnson_h;
// // int *tempsLag_h;
int *minTempsArr_h;
int *minTempsDep_h;
int *sumPT_h;
//
// int *machine_h;
// int *jobPairs_h;
//
void
allocate_host_bound_tmp()
{
    tempsJob_h    = (int *) malloc(nbMachines_h * nbJob_h * sizeof(int));
    minTempsDep_h = (int *) malloc(nbMachines_h * sizeof(int));
    minTempsArr_h = (int *) malloc(nbMachines_h * sizeof(int));
    sumPT_h = (int *) malloc(nbMachines_h * sizeof(int));
}

void free_host_bound_tmp(){
  free(tempsJob_h);
  free(minTempsDep_h);
  free(minTempsArr_h);
  free(sumPT_h);
}

//
//==============================
//HOST FUNCTIONS
//PREPARING BOUNDING DATA ON CPU
//==============================
/**
 * minTempsDep/minTempsArr
 * earlist possible starting time of a job on machines and
 * shortest possible completion time of a job after release from machines
 * size : nbMachines
 * requires : PTM
 */
void fillMinTempsArrDep(){
    for (int k = 0; k < nbMachines_h; k++) {
        minTempsDep_h[k]=9999999;
    }
    minTempsDep_h[nbMachines_h-1]=0;
    int *tmp=new int[nbMachines_h];

    for (int i = 0; i<nbJob_h; i++){
       for (int k = nbMachines_h-1; k>=0; k--) {
           tmp[k]=0;
       }
       tmp[nbMachines_h-1]+=tempsJob_h[(nbMachines_h-1)*nbJob_h + i];//ptm[(nbMachines-1) * nbJob + job];
       for (int k = nbMachines_h - 2; k >= 0; k--){
           tmp[k]=tmp[k+1]+tempsJob_h[k*nbJob_h + i];
       }
       for (int k = nbMachines_h-2; k>=0; k--) {
           minTempsDep_h[k]=(tmp[k+1]<minTempsDep_h[k])?tmp[k+1]:minTempsDep_h[k];
       }
    }

    for (int k = 0; k < nbMachines_h; k++) {
       minTempsArr_h[k]=9999999;
    }
    minTempsArr_h[0]=0;

    for (int i = 0; i < nbJob_h; i++) {
       for (int k = 0; k < nbMachines_h; k++) {
           tmp[k]=0;
       }
       tmp[0]+=tempsJob_h[i];
       for (int k = 1; k < nbMachines_h; k++) {
           tmp[k]=tmp[k-1]+tempsJob_h[k*nbJob_h+i];
       }
       for (int k = 1; k < nbMachines_h; k++) {
           minTempsArr_h[k]=(tmp[k-1]<minTempsArr_h[k])?tmp[k-1]:minTempsArr_h[k];
       }
    }

    delete[]tmp;
}

void fillSumPT()
{
    for (int k = 0; k < nbMachines_h; k++) {
        sumPT_h[k]=0;
        for (int i = 0; i < nbJob_h; i++) {
            sumPT_h[k] += tempsJob_h[k*nbJob_h + i];
        }
    }
}
//
// /*********************************
//  ****** evaluate bounds **********
//  *********************************/
// //§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
// inline __device__ void
// initCmax_d(const int * tempsMachines, const int nbAffectDebut, int &tmp0, int &tmp1, int &ma0, int &ma1, const int ind) {
//   ma0 = _machine[ind];
//   ma1 = _machine[_sum + ind];
//
//   int coeff = __cosf(nbAffectDebut);//=1 ifff nbAffect=0
//   tmp0 = (1 - coeff) * tempsMachines[ma0] + coeff * _minTempsArr[ma0];
//   tmp1 = (1 - coeff) * tempsMachines[ma1] + coeff * _minTempsArr[ma1];
//
//
// }
// //§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
// // inline __device__ void
// // heuristiqueCmax_d(const int * job, int &tmp0, int &tmp1, const int ma0, const int ma1, const int ind, const int * _tabJohnson, const unsigned char * tempsJob) {
// //   register int jobCour;
// //
// //   //#pragma unroll 5
// //   for (int j = 0; j < size_d; j++) {
// //     //jobCour = tex1Dfetch(tabJohnson_tex,ind*size_d+j);
// //     jobCour = _tabJohnson[ind * size_d + j];
// //     if (job[jobCour] == 0) {
// //       tmp0 += tempsJob[ma0 * size_d + jobCour];
// //
// //       tmp1 = max(tmp1, tmp0 + _tempsLag[ind * size_d + jobCour]) + tempsJob[ma1 * size_d + jobCour];
// //     }
// //   }
// // }
// // //§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§!!
// // inline __device__ int
// // cmaxFin_d(const int *  tempsMachinesFin, const int tmp0, const int tmp1, const int ma0, const int ma1) {
// //   return max(tmp1 + tempsMachinesFin[ma1],
// //              tmp0 + tempsMachinesFin[ma0]);
// // }
// // //§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
// // //compute front
// // template <typename T>
// // inline __device__ void
// // set_tempsMachines_d(const int* front, int *tempsMachines, const T* permutation, const T toSwap1, const T toSwap2, const int limit1, const int * tempsJob, const int matId) {
// //     int job, m = 0;
// //
// //     for (m = 0; m < _nbMachines; m++)
// //         tempsMachines[m] = front[m];
// //
// //     if(toSwap2 == limit1){
// //         job = permutation[index2D(toSwap1, matId)];
// //
// //         tempsMachines[0] += tempsJob[job];
// //
// //         for (m = 1; m < _nbMachines; m++){
// //             tempsMachines[m] = max(tempsMachines[m], tempsMachines[m - 1]);
// //             tempsMachines[m] += tempsJob[m * size_d + job];
// //         }
// //     }
// // }
// //
// // template <typename T>
// // inline __device__ void
// // set_tempsMachines_retardDebut_d(int *tempsMachines, const T* permutation, const T toSwap1, const T toSwap2, const int limit1, const int * tempsJob, const int matId) {
// //   int job, m = 0;
// //
// //   memset(tempsMachines,0,_nbMachines*sizeof(int));
// //
// //   for (int j = 0; j <= limit1; j++) {
// //     if (j == toSwap1)
// //       job = permutation[index2D(toSwap2, matId)];
// //     else if (j == toSwap2)
// //       job = permutation[index2D(toSwap1, matId)];
// //     else
// //       job = permutation[index2D(j, matId)];
// //
// //     tempsMachines[0] = tempsMachines[0] + tempsJob[job]; //=_tempsJob[0][job]
// //
// //     for (m = 1; m < _nbMachines; m++)
// //       tempsMachines[m] = max(tempsMachines[m], tempsMachines[m - 1]) + tempsJob[m * size_d + job];
// //   }
// // }
// // //§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§!
// // template <typename T>
// // inline __device__ void
// // set_job_d(int *job, const T *permutation, const T toSwap1, const T toSwap2, const int limit1, const int limit2,
// //                                         const int matId) {
// //   int j = 0;
// //
// //   for (j = 0; j < size_d; j++) {
// //     job[j] = 1;
// //   }
// //
// //   for (j = limit1 + 1; j < limit2; j++) {
// //     if (j == toSwap1)
// //       job[permutation[index2D(toSwap2, matId)]] = 0;
// //     else if (j == toSwap2)
// //       job[permutation[index2D(toSwap1, matId)]] = 0;
// //     else
// //       job[permutation[index2D(j, matId)]] = 0;
// //   }
// // }
// //
// // template <typename T>
// // inline __device__ void
// // set_job_jobFin_d(int *job, int *jobFin, const T *permutation, const T toSwap1, const T toSwap2, const int limit1, const int limit2, const int matId) {
// //   int j = 0;
// //
// //   for (j = 0; j <= limit1; j++) {
// //
// //     if (j == toSwap1)
// //       job[permutation[index2D(toSwap2, matId)]] = j + 1;
// //     else if (j == toSwap2)
// //       job[permutation[index2D(toSwap1, matId)]] = j + 1;
// //     else
// //       job[permutation[index2D(j, matId)]] = j + 1;
// //   }
// //   for (j = limit1 + 1; j < limit2; j++) {
// //     if (j == toSwap1)
// //       job[permutation[index2D(toSwap2, matId)]] = 0;
// //     else if (j == toSwap2)
// //       job[permutation[index2D(toSwap1, matId)]] = 0;
// //     else
// //       job[permutation[index2D(j, matId)]] = 0;
// //   }
// //   for (j = limit2; j < size_d; j++) {
// //     if (j == toSwap1) {
// //       job[permutation[index2D(toSwap2, matId)]] = j + 1;
// //       jobFin[j] = permutation[index2D(toSwap2, matId)];
// //     } else if (j == toSwap2) {
// //       job[permutation[index2D(toSwap1, matId)]] = j + 1;
// //       jobFin[j] = permutation[index2D(toSwap1, matId)];
// //     } else {
// //       job[permutation[index2D(j, matId)]] = j + 1;
// //       jobFin[j] = permutation[index2D(j, matId)];
// //     }
// //   }
// // }
// // //§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§!
// // template <typename T>
// // inline __device__ void
// // set_tempsMachinesFin_d(const int* back, const T *prmu, int *tempsMachinesFin, const T swap1, const T swap2, const int limit2, const int *tempsJob) {
// //   int jobCour=0;
// //
// //   for (int m = 0; m < _nbMachines; m++)
// //     tempsMachinesFin[m] = back[m];
// //
// //   if(swap2 == limit2){
// //     jobCour = prmu[swap1];
// //     tempsMachinesFin[_nbMachines-1] += tempsJob[(_nbMachines-1)*size_d + jobCour];
// //     for (int j = _nbMachines - 2; j >= 0; j--){
// //         tempsMachinesFin[j]=max(tempsMachinesFin[j],tempsMachinesFin[j+1])+tempsJob[j * size_d + jobCour];
// //     }
// //   }
// // }
// //

// //§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
// //thread evaluates
// //swap(permutation,toSwap1,toSwap2)
// template <typename T>
inline __device__ int
thread_LBBegin(const int* perm, const int lim1) {
    int tmpMa[MAXMACH]={0};
    int tmpRem[MAXMACH];
    int tmpBack[MAXMACH];

    //remaining work on each machine
    for(int i=0;i<_nbMachines;i++)
    {
        tmpRem[i]=_sumPT[i];
    }

    //if no jobs scheduled in front (initial problem)
    if(lim1==-1){
        for(int i=0;i<_nbMachines;i++)
            tmpMa[i]=_minTempsArr[i];
    }else{
        //schedule front
        for(int i=0;i<_nbMachines;i++)
            tmpMa[i]=0;
        for(int i=0;i<=lim1;i++){
            int job=perm[i];
            int ptm=_tempsJob[job];
            tmpMa[0]+=ptm;
            tmpRem[0]-=ptm;

            for(int j=1;j<_nbMachines;j++){
                ptm=_tempsJob[j*_nbJob+job];
                tmpMa[j]=maxi(tmpMa[j-1],tmpMa[j])+ptm;
                tmpRem[j]-=ptm;
            }
        }
    }

    //compute bound
    int lb,tmp0,tmp1,cmax;
    tmp0=tmpMa[0]+tmpRem[0];
    lb=tmp0+_minTempsDep[0];

    for(int j=1;j<_nbMachines;j++){
        tmp1=tmpMa[j]+tmpRem[j];
        tmp1=maxi(tmp1,tmp0);
        cmax=tmp1+_minTempsDep[j];
        lb=maxi(lb,cmax);
        tmp0=tmp1;
    }

    return lb;
}

// //insert {toSwap1 < toSwap2}
// template <typename T>
// inline __device__ int
// thread_evalSol_insert_d(const T *  permutation, const T toSwap1, const T toSwap2) {
//   int temps[MAXMACH]={0};
//   int job;
//
//   for (int mm = 0; mm < _nbMachines; mm++)
//     temps[mm] = 0;
//
//   for (int j = 0; j < size_d; j++) {
//     if (j == toSwap1)
//       job = permutation[toSwap2];
//     else if (j > toSwap1 && j<= toSwap2)
//       job = permutation[j-1];
//     else
//       job = permutation[j];
//
//     temps[0] = temps[0] + _tempsJob[job];
//
//     for (int m = 1; m < _nbMachines; m++)
//       temps[m] = max(temps[m], temps[m - 1]) + _tempsJob[m * size_d + job];
//   }
//
//   return temps[_nbMachines - 1];
// }
//
// //§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//
// template <typename T>
// inline __device__ int
// computeCostFast(const T* schedules_sh, const T toSwap1, const T toSwap2, const T limit1, const T limit2, const unsigned char * tempsJob, const int mat_id, const int * _tabJohnson, const int* front, const int* back, const int best) {
//   int tempsMachines[MAXMACH]={0};
//   int tempsMachinesFin[MAXMACH]={0};
//
//   int job[MAXJOBS]={0};
//   int jobFin[MAXJOBS]={0};
//
// //  int nbAffectFin = size_d - limit2;
// //  int nbAffectDebut = limit1 + 1;
//   int borneInf=0;
//
//   if (limit2 - limit1 == 1) {
//     borneInf=thread_evalSol_d(schedules_sh+mat_id*size_d, toSwap1, toSwap2);
//   } else {
// //    set_tempsMachines_retardDebut_d(tempsMachines, schedules_sh, toSwap1, toSwap2, limit1, tempsJob, mat_id);
//     set_tempsMachines_d(front, tempsMachines, schedules_sh, toSwap1, toSwap2, limit1, _tempsJob, mat_id); //front
//     set_job_d(job, schedules_sh, toSwap1, toSwap2, limit1,limit2, mat_id);
// //    set_job_jobFin_d(job, jobFin, schedules_sh, toSwap1, toSwap2, limit1,limit2, mat_id);
//
//     set_tempsMachinesFin_d(back,schedules_sh+mat_id*size_d, tempsMachinesFin, toSwap1, toSwap2, limit2, _tempsJob);
// //    set_tempsMachinesFin_tempsJobFin_d(jobFin, tempsMachinesFin, size_d - limit2, tempsJob);
//
// //    borneInf=calculBorne_d(job, tempsMachinesFin, front+ivm*_nbMachines, nbAffectDebut, nbAffectFin, tempsJob, _tabJohnson, best);
//     borneInf=calculBorne_d(job, tempsMachinesFin, tempsMachines, limit1 + 1, size_d - limit2, _tempsJob, _tabJohnson, best);
//   }
//   return borneInf;
// }
//
//
// template <typename T>
// inline __device__ int
// computeCost(const T* schedules_sh, const T toSwap1, const T toSwap2, const T limit1, const T limit2, const int * tempsJob, const int mat_id, const int * _tabJohnson, const int best) {
//   int tempsMachines[MAXMACH]={0};
//   int tempsMachinesFin[MAXMACH]={0};
//
//   int job[MAXJOBS]={0};
//   int jobFin[MAXJOBS]={0};
//
//   int nbAffectFin = size_d - limit2;
//   int nbAffectDebut = limit1 + 1;
//   int borneInf=0;
//
//   if (limit2 - limit1 == 1) {
//     borneInf=thread_evalSol_d(schedules_sh+mat_id*size_d, toSwap1, toSwap2);
//   } else {
//     set_tempsMachines_retardDebut_d(tempsMachines, schedules_sh, toSwap1,
//                                     toSwap2, limit1, _tempsJob, mat_id);
//     set_job_jobFin_d(job, jobFin, schedules_sh, toSwap1, toSwap2, limit1,
//                      limit2, mat_id);
//     set_tempsMachinesFin_tempsJobFin_d(jobFin, tempsMachinesFin, nbAffectFin,_tempsJob);
//     borneInf=calculBorne_d(job, tempsMachinesFin, tempsMachines, nbAffectDebut,
//                       nbAffectFin, _tempsJob, _tabJohnson, best);
//   }
//   return borneInf;
// }
//
// template <typename T>
// __global__ void
// __launch_bounds__(128, 8) bound(const T * schedules_d, const T * limit1s_d, const T * limit2s_d, const T * line_d,
//   int * costsBE_d, int * sums_d, const T * state_d, const int * toSwap_d,
//   const int * ivmId_d, unsigned int * bdleaves_d, unsigned int * ctrl_d, int * flagLeaf,
//   const int best){
//     /**** thread indexing ****/
//     register int tid   = blockIdx.x * blockDim.x + threadIdx.x;
//     register int BE    = tid & 1;
//     register int ivmnb = ivmId_d[(tid >> 1)]; // the ivm tid is working on
//
//     /***** shared memory declarations *****/
//     extern __shared__ unsigned char sharedArray[];
//     unsigned char * tempsJob_sh = (unsigned char *) sharedArray;
//     char * permut_sh = (char *) &tempsJob_sh[_nbMachines * size_d];
//
//     if (threadIdx.x < size_d) {
//         for (int j = 0; j < _nbMachines; j++)
//             tempsJob_sh[j * size_d + threadIdx.x] =
//               (unsigned char) _tempsJob[j * size_d + threadIdx.x];
//     }
//     if (tid < 2 * ctrl_d[toDo]) {
//         //  if (tid < 2 * ctrl_d[0]) {
//         if (tid % 2 == 0) {
//             for (int i = 0; i < size_d; i++)
//                 permut_sh[index2D(i, threadIdx.x >> 1)] =
//                   schedules_d[index2D(i, ivmnb)];
//         }
//     }
//
//     __syncthreads();
//     /*******************************************/
//     if (tid < 2 * ctrl_d[toDo]) {
//         //  if (tid < 2 * ctrl_d[0]) {
//         char limit1 = limit1s_d[ivmnb] + 1 - BE;
//         char limit2 = limit2s_d[ivmnb] - BE;
//
//         char Swap1 = toSwap_d[(tid >> 1)];
//         char Swap2 = (1 - BE) * limit1 + BE * limit2;
//
//         char jobnb = permut_sh[index2D(Swap1, threadIdx.x >> 1)];
//
//         int where = ivmnb * 2 * size_d + BE * size_d + (int) jobnb;
//
//         if (line_d[ivmnb] < (size_d - 1)) { // boundNodes
//             costsBE_d[where] = computeCost(permut_sh, Swap1, Swap2, limit1, limit2, _tempsJob, threadIdx.x>>1,_tabJohnson,best);// + BE * limit2;
//             // costsBE_d[where] = computeCost(permut_sh, Swap1, Swap2, limit1, limit2, tempsJob_sh, threadIdx.x>>1,_tabJohnson,best);// + BE * limit2;
//             atomicAdd(&sums_d[2 * ivmnb + BE], costsBE_d[where]);
//         } else if (BE == 0) { // boundLeaves
//             if (state_d[ivmnb] == 1)
//                 bdleaves_d[ivmnb]++;
//
//             flagLeaf[ivmnb] = 1;
//             atomicInc(&ctrl_d[foundLeaf], UINT_MAX);
//         }
//     }
// }
//
// //§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
// warp parallel partial makespan evaluation : front
inline __device__ void
tile_scheduleFront(thread_block_tile<TILE_SZ> g, const int* permutation, const int limit1, const int* ptm, int *front, int *remain) {
    int lane;

    if(limit1 == -1){
        for (int i = g.thread_rank(); i < _nbMachines; i+=warpSize) {
            front[i]=_minTempsArr[i];
        }
        return;
    }

    for (int i = 0; i <= (_nbMachines / warpSize); i++) {
        lane = i * warpSize + g.thread_rank();
        int mNM=mini(limit1+1,_nbMachines);

        int pt;
        int job;
        int tmp0=0;
        int tmp1=0;

        for(int i=0;i<=limit1;i++){//for jobs scheduled in front
            tmp1=g.shfl_up(tmp0,1);
            if(lane<mini(i+1,mNM)){
                job=permutation[i-lane];
                pt = ptm[lane*size_d+job];
                remain[lane] -= pt;
                tmp0=maxi(tmp0,tmp1)+pt;
            }
        }
        if(lane==0)
            front[0]=tmp0;

        if(lane<mini(_nbMachines-1,mNM)){
            job=permutation[limit1-lane];
        }

        for(int i=1;i<_nbMachines;i++){
            tmp1=g.shfl_down(tmp0,1);
            if(lane<mini(_nbMachines-i,mNM)){
                pt = ptm[(lane+i)*size_d+job];

                remain[lane+i] -= pt;
                tmp0=maxi(tmp0,tmp1)+pt;
            }
            if(lane==0)front[i]=tmp0;
        }
    }
}

inline __device__ void
tile_scheduleBack(thread_block_tile<TILE_SZ> g, const int* permutation, const int limit2, const int* ptm, int *back, int *remain)
{
    if(limit2==size_d){
        for(int i=g.thread_rank(); i<_nbMachines; i+=warpSize)
        {
            back[i]=_minTempsDep[i];
        }
        return;
    }

    int lane;
    for (int i = 0; i <= (_nbMachines / warpSize); i++) {
        lane = i * warpSize + g.thread_rank();

        if(lane<_nbMachines)back[lane]=0;

        int mNM=mini(size_d-limit2,_nbMachines);

        int pt;
        int tmp0=0;
        int tmp1=0;

        int job;
        int ma=_nbMachines-1-lane;

        for(int i=0;i<size_d-limit2;i++){//for jobs scheduled in back
            tmp1=g.shfl_up(tmp0,1);
            if(lane<mini(i+1,mNM)){
                job=permutation[(size_d-1)+lane-i];

                pt=ptm[ma*size_d+job];
                remain[ma] -= pt;
                tmp0=maxi(tmp0,tmp1)+pt;
            }
        }

        if(lane==0)
            back[_nbMachines-1]=tmp0;

        if(lane<mini(_nbMachines-1,mNM)){
            job=permutation[limit2+lane];
        }

        for(int i=1;i<_nbMachines;i++){
            ma--;
            tmp1=g.shfl_down(tmp0,1);
            if(lane<mini(_nbMachines-i,mNM)){
                pt = ptm[ma*size_d+job];
                remain[ma] -= pt;
                tmp0=maxi(tmp0,tmp1)+pt;
            }
            if(lane==0)back[_nbMachines-1-i]=tmp0;
        }
    }
}
//
// //§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
// //warp parallel makespan evaluation
// //§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
// template <typename T>
// inline __device__ void
// tile_evalSolution(thread_block_tile<TILE_SZ> g, const T* permutation, const T limit1, const int* ptm, int *front) {
//     int lane;
//
//     for (int i = 0; i <= (_nbMachines / warpSize); i++) {
//         lane = i * warpSize + g.thread_rank();
//         int mNM=mini(limit1+1,_nbMachines);
//
//         int pt;
//         int job;
//         int tmp0=0;
//         int tmp1=0;
//
//         for(int i=0;i<=limit1;i++){//for jobs scheduled in front
//             tmp1=g.shfl_up(tmp0,1);
//             if(lane<mini(i+1,mNM)){
//                 job=permutation[i-lane];
//                 pt = ptm[lane*size_d+job];
//                 // remain[lane] -= pt;
//                 tmp0=maxi(tmp0,tmp1)+pt;
//             }
//         }
//         if(lane==0)
//             front[0]=tmp0;
//
//         if(lane<mini(_nbMachines-1,mNM)){
//             job=permutation[limit1-lane];
//         }
//
//         for(int i=1;i<_nbMachines;i++){
//             tmp1=g.shfl_down(tmp0,1);
//             if(lane<mini(_nbMachines-i,mNM)){
//                 pt = ptm[(lane+i)*size_d+job];
//
//                 // remain[lane+i] -= pt;
//                 tmp0=maxi(tmp0,tmp1)+pt;
//             }
//             if(lane==0)front[i]=tmp0;
//         }
//     }
// }
//
//
// // template <typename T>
// // inline __device__ void
// // tile_evalSolution(thread_block_tile<TILE_SZ> g, const T* permutation) {
// //     int lane;
// //
// //     for (int i = 0; i <= (_nbMachines / warpSize); i++) {
// //         lane = i * warpSize + g.thread_rank();
// //         int mNM=mini(size_d,_nbMachines);
// //
// //         int pt;
// //         int job;
// //         int tmp0=0;
// //         int tmp1=0;
// //
// //         for(int i=0;i<size_d;i++){//for jobs scheduled in front
// //             tmp1=g.shfl_up(tmp0,1);
// //             if(lane<mini(i+1,mNM)){
// //                 job=permutation[i-lane];
// //                 pt = _tempsJob[lane*size_d+job];
// //                 tmp0=maxi(tmp0,tmp1)+pt;
// //             }
// //         }
// //         if(lane==0)
// //             front[0]=tmp0;
// //
// //         if(lane<mini(_nbMachines-1,mNM)){
// //             job=permutation[limit1-lane];
// //         }
// //
// //         for(int i=1;i<_nbMachines;i++){
// //             tmp1=g.shfl_down(tmp0,1);
// //             if(lane<mini(_nbMachines-i,mNM)){
// //                 pt = _tempsJob[(lane+i)*size_d+job];
// //                 tmp0=maxi(tmp0,tmp1)+pt;
// //             }
// //             if(lane==0)front[i]=tmp0;
// //         }
// //     }
// // }
//
//
// //warp_remainingWork(tile32,&prmu[l1+1],size_d-line,&remain[warpID*_nbMachines])
// template <typename T>
// inline __device__ void
// tile_remainingWork(thread_block_tile<TILE_SZ> g, const T *unscheduledJobs, const int nUn, int *remain)
// {
//     int lane = g.thread_rank();
//     int job;
//     int ptjob;
//
//     for (int i = 0; i <= nUn / warpSize; i++) {
//         //  rem+=ptm[(i*warpSize+thPos)*size_d+job];
//         for(int j=0;j<_nbMachines;j++){
//             ptjob=0;
//             if (i * warpSize + lane < nUn) {
//                 job = unscheduledJobs[i * warpSize + lane];
//                 ptjob=_tempsJob[j * size_d + job];
//             }
//
//             int rem=tile_sum(g,ptjob);
//             //remain[warpID*_nbMachines+(i*warpSize+thPos)]+=ptm[(i*warpSize+thPos)*size_d+job];
//
//             g.sync();
//
//             if(lane==0)
//                 remain[j]+=rem;
//
//             g.sync();
//         }
//     }
// }
//

inline __device__ void
addFrontAndBound(const int* back, const int* front, const int* remain, int job, const int* ptm, int &lowerb)
{
    int tmp0,tmp1;

    int lb=front[0]+remain[0]+back[0];
    tmp0=front[0]+ptm[job];

    for(int j=1;j<_nbMachines;j++){
        tmp1=max(tmp0,front[j]);
        tmp0=tmp1+ptm[j*size_d+job];
        lb=max(lb,tmp1+remain[j]+back[j]);//cmax);
    }
    lowerb=lb;
}
//
// template <typename T>
// inline __device__ void
// addBackAndBound(const int* back, const int* front, const int* remain, T job, const int* ptm, int &lowerb)
// {
//     int tmp0,tmp1;
//
//     tmp0=back[(_nbMachines-1)]+ptm[(_nbMachines-1)*size_d+job];
//     int lb=front[_nbMachines-1]+remain[_nbMachines-1]+back[_nbMachines-1];
//     //add job to back and compute max of machine-bounds;
//
//     for(int j=_nbMachines-2;j>=0;j--){
//         tmp1=max(tmp0,back[j]);//+pt;
//         tmp0=tmp1+ptm[j*size_d+job];
//         lb=max(lb,front[j]+remain[j]+tmp1);
//     }
//     lowerb=lb;
// }
//

template <unsigned tile_size>
inline __device__ void
tile_addFrontAndBound(thread_block_tile<tile_size> g, const int* back, const int* front, const int* remain, const int *unscheduledJobs, const int nUn, int *cost)
{
    for (int i = g.thread_rank(); i < nUn; i+=tile_size) {        // one thread : one job
        int job = unscheduledJobs[i]; // each thread grabs one job
        addFrontAndBound(back, front, remain, job, _tempsJob, cost[job]);
    }
    // }
}
//
//
// inline __device__ int perIVMtodo(int& ivmtodo, int* row, int line)
// {
//     ivmtodo = 0;
//     while(row[ivmtodo]>=0 && ivmtodo < size_d-line)ivmtodo++;
//
//     return ivmtodo;
// }
//
// template <typename T>
// __global__ void
//  __launch_bounds__(128, 16)
// boundOne(const T * schedules_d, const T * limit1s_d, const T * limit2s_d, const T * dir, const T * line_d,
//   int * costsBE_d, const int * toSwap_d, const int * ivmId_d, const int best, const int * front,
//   const int * back)
// {
//     // thread indexing
//     int tid   = blockIdx.x * blockDim.x + threadIdx.x;
//
//     // shared memory
//     extern __shared__ unsigned char sharedArray[];
//     unsigned char * tempsJob_sh = (unsigned char *) sharedArray;
//
//     if (threadIdx.x < size_d) {
//         for (int j = 0; j < _nbMachines; j++)
//             tempsJob_sh[j * size_d + threadIdx.x] =
//               (unsigned char) _tempsJob[j * size_d + threadIdx.x];
//     }
//     __syncthreads();
//
// //    if(tid>=nbIVM_d)return;
//     //
//     if (tid < todo) {
//         int ivmnb = ivmId_d[tid];                       // the ivm thread tid is working on
//         int BE    = dir[index2D(line_d[ivmnb], ivmnb)]; // begin/end?
//
//         int limit1 = limit1s_d[ivmnb] + 1 - BE;
//         int limit2 = limit2s_d[ivmnb] - BE;
//
//         int Swap1 = toSwap_d[tid];//index of job to place...
//         int Swap2 = (1 - BE) * limit1 + BE * limit2;//...at begin or end
//
//         int jobnb = schedules_d[index2D(Swap1, ivmnb)];
//
//         int where = ivmnb * 2 * size_d + BE * size_d + (int) jobnb;
//
//         if (line_d[ivmnb] < (size_d - 1)) { // boundNodes
//             //costsBE_d[where] = computeCostFast(schedules_d, Swap1, Swap2, limit1, limit2, tempsJob_sh, ivmnb, _tabJohnson, front + ivmnb * _nbMachines, back + ivmnb * _nbMachines, best);
//             costsBE_d[where] = computeCostFast(schedules_d, Swap1, Swap2, limit1, limit2, tempsJob_sh, ivmnb, _tabJohnson, front + ivmnb * _nbMachines, back + ivmnb * _nbMachines, best);
//         }
//     }
// } // boundOne
//
// __global__ void // __launch_bounds__(128, 16)
// boundWeak_BeginEnd(const int *limit1s_d,const int *limit2s_d, const int *line_d, const int *schedules_d, int *costsBE_d, const int *state_d,int *front_d,int *back_d,const int best,int *flagLeaf)
// {
//     thread_block_tile<32> g = tiled_partition<32>(this_thread_block());
//
//     int ivm = (blockIdx.x * blockDim.x + threadIdx.x) / warpSize; // global ivm id
//     int warpID = threadIdx.x / warpSize;
//     int lane = g.thread_rank();//laneID();
//
//     // SHARED MEMORY
//     // 5*nbMachines*(int) per warp
//     // 1*nbJob*(int) per warp
//     // 4*(int) per warp
//     // (20*nbMachines+4*nbJob+16)*(int) per block
//     // +nbJob*nbMachines per block (ptm)
//     // 20/20 : 1920 B
//     // 50/20 :
//     extern __shared__ bool sharedSet[];
//     int *front = (int*)&sharedSet;//partial schedule begin
//     int *back    = (int *)&front[4 * _nbMachines];  // partial schedule end[M]
//     int *remain  = (int *)&back[4 * _nbMachines];   // remaining work[M]
//     int *prmu = (int *)&remain[4 * _nbMachines];   // schedule[N]
//
//     // load PTM to smem
//     prmu += warpID * size_d;
//     //load schedule limits and line to smem
//     int line=line_d[ivm];
//     int l1=limit1s_d[ivm];
//     int l2=limit2s_d[ivm];
//
//     int i;
//
//     for (int i = lane; i < size_d; i+=warpSize) {
//         prmu[i]=schedules_d[ivm*size_d+i];
//     }
//     //initialize remain
//     for (int i = lane; i < _nbMachines; i+=warpSize) {
//         remain[warpID * _nbMachines + i] = _sumPT[i];
//     }
//     g.sync();
//
//     // nothing to do
//     if (state_d[ivm] == 0) return;
//
    // tile_scheduleFront(g, prmu, l1, _tempsJob, &front[warpID * _nbMachines], &remain[warpID*_nbMachines]);
//     tile_scheduleBack(g, prmu, l2, _tempsJob, &back[warpID * _nbMachines], &remain[warpID*_nbMachines]);
//     g.sync();
//
//     tile_addFrontAndBound(g,&back[warpID * _nbMachines],&front[warpID * _nbMachines],&remain[warpID * _nbMachines],&prmu[l1+1],size_d-line,&costsBE_d[2 * ivm * size_d]);
//     tile_addBackAndBound(g,&back[warpID * _nbMachines],&front[warpID * _nbMachines],&remain[warpID * _nbMachines],&prmu[l1+1],size_d-line,&costsBE_d[(2 * ivm + 1) * size_d]);
//
//     if(lane==0){
//         if (line == size_d - 1) {
//             flagLeaf[ivm] = 1;
//             atomicInc(&targetNode, UINT_MAX);
//         }
//     }
//
//     //back to main memory
//     g.sync();
//     for (i = g.thread_rank(); i<_nbMachines; i+=warpSize)
//     {
//         front_d[ivm * _nbMachines + i] = front[warpID * _nbMachines + i];
//         back_d[ivm * _nbMachines + i]  = back[warpID * _nbMachines + i];
//     }
// }
//


//warp-based LB kernel : each warp computes the LBs of all children
//[schedules_d, lim1_d] contains parent subproblems
__global__ void
boundWeak_Begin(const int *schedules_d,const int* lim1_d,int* costsBE_d, const int* state_d)
// ,int *costsBE_d, T *state_d,int *front_d,int *back_d,const int best,int *flagLeaf)
{
    thread_block_tile<32> tile32 = tiled_partition<32>(this_thread_block());
//
    int ivm = (blockIdx.x * blockDim.x + threadIdx.x) / warpSize; // global ivm id
    int warpID = threadIdx.x / warpSize;
//
//     // SHARED MEMORY
//     // 5*nbMachines*(int) per warp
//     // 1*nbJob*(int) per warp
//     // 4*(int) per warp
//     // (20*nbMachines+4*nbJob+16)*(int) per block
//     // +nbJob*nbMachines per block (ptm)
//     // 20/20 : 1920 B
//     // 50/20 :
    extern __shared__ int sharedSet[];
    int *front = sharedSet;//partial schedule begin
    int *back    = &front[4 * _nbMachines];  // partial schedule end[M]
    int *remain  = &back[4 * _nbMachines];   // remaining work[M]
    int *prmu = &remain[4 * _nbMachines];   // schedule[N]

    front += warpID*_nbMachines;
    back += warpID*_nbMachines;
    remain += warpID*_nbMachines;
    prmu += warpID*_nbJob;

//     int line=line_d[ivm];
    int l1=lim1_d[ivm];
    int l2=_nbJob; //only left to right scheduling

    // //load schedules to shared memory
    for(int i=tile32.thread_rank(); i<_nbJob; i+=warpSize)
    {
        prmu[i]=schedules_d[ivm*_nbJob+i];
    }
    //initialize remain (sum of unscheduled PTs on each machine)
    for (int i = tile32.thread_rank(); i < _nbMachines; i+=warpSize) {
        remain[i] = _sumPT[i];
    }
    tile32.sync();
    //
    // // nothing to do
    if (state_d[ivm] == 0) return;
    //
    tile_scheduleFront(tile32, prmu, l1, _tempsJob, front, remain);
    tile_scheduleBack(tile32, prmu, l2, _tempsJob, back, remain);
    tile32.sync();
    //
    tile_addFrontAndBound(tile32,back,front,remain,&prmu[l1+1],_nbJob-l1-1,&costsBE_d[2 * ivm * _nbJob]);
}

//evaluate LBs of subproblems [schedules_d,lim1_d]
__global__ void
boundWeak_Begin_One(const int *schedules_d,const int* lim1_d,int* costsBE_d, const int* state_d)
{
    int tid=blockIdx.x*blockDim.x+threadIdx.x;

    if(state_d[tid]>0)
        costsBE_d[tid]=thread_LBBegin(&schedules_d[tid*_nbJob],lim1_d[tid]);
}

//
// template < typename T >
// __global__ void // __launch_bounds__(128, 16)
// makespans(T *schedules_d,int *cmax, T *state_d)
// {
//     thread_block_tile<32> tile32 = tiled_partition<32>(this_thread_block());
//
//     int ivm = (blockIdx.x * blockDim.x + threadIdx.x) / warpSize; // global ivm id
//     int warpID = threadIdx.x / warpSize;
//
//     extern __shared__ char sharedCmax[];
//     int *front = (int*)&sharedCmax;//partial schedule begin
//     int *prmu = (int *)&front[4 * _nbMachines];   // schedule[N]
//
//     // load PTM to smem
//     prmu += warpID * size_d;
//     int i;
// 	for(i=tile32.thread_rank(); i<size_d; i+=warpSize){
//         prmu[i]=schedules_d[ivm*size_d+i];
// 	}
//     tile32.sync();
//
//     // nothing to do
//     if (state_d[ivm] == 0) return;
//
//     tile_evalSolution(tile32, prmu, _nbJob-1, _tempsJob, &front[warpID * _nbMachines]);
//     tile32.sync();
//
//     if(tile32.thread_rank()==0){
//         cmax[ivm]=front[(warpID+1) * _nbMachines - 1];
//     }
// }
//
// //single block :
// __global__ void boundRoot(int *mat, int *dir, int *line, int *costsBE_d, int *sums_d, int *bestpermut, const int best, const int branchingMode) {
// 	thread_block bl = this_thread_block();
//
//     extern __shared__ int smem1[];
//     int     *permut = smem1;
//
//     for(int l=bl.thread_rank(); l<size_d; l+=bl.size())
//     {
//         permut[l] = l;
//         mat[l] = bestpermut[l];
//     }
//     bl.sync();
//
//     for(int l=bl.thread_rank(); l<size_d; l+=bl.size()){
//         // bound begin
//         costsBE_d[l] =
//         computeCost(permut, 0, l, 0, size_d, _tempsJob, 0, _tabJohnson, 999999);
//         atomicAdd(&sums_d[0], costsBE_d[l]);
//     }
//
//     if(branchingMode>0){
//         for(int l=bl.thread_rank(); l<size_d; l+=bl.size()){
//             // bound end
//             costsBE_d[size_d + l] =
//             computeCost(permut, size_d - 1, l, -1, size_d - 1, _tempsJob, 0, _tabJohnson, 999999);
//             atomicAdd(&sums_d[1], costsBE_d[size_d + l]);
//         }
//     }
//     bl.sync();
//
//     line[0] = 0;
//     dir[index2D(0, 0)] = 0;
//
//     if(branchingMode>0)
//     {
//         if (bl.thread_rank() == 0) {
//             if (sums_d[0] <= sums_d[1]) {
//                 dir[index2D(0, 0)] = 1;
//                 //reverse
//                 int i1=0;
//                 int i2=size_d-1;
//                 while(i1<i2)
//                     swap_d(&mat[i1++],&mat[i2--]);
//             }
//         }
//     }
//     bl.sync();
//
//     for(int l=bl.thread_rank();l<size_d;l+=bl.size()){
//         int job=absolute_d(mat[l]);
//         int val = costsBE_d[index2D(job, dir[0])];
//         if (val >= best) {
//             mat[l] = negative_d(job);
//         }
//     }
//     bl.sync();
//
//     for(int l=bl.thread_rank();l<size_d;l+=bl.size()){
//         root_d[l] = mat[l];
//     }
//     if (bl.thread_rank() == 0) root_dir_d = dir[0];
// }
