# PBBPerm
Parallel, distributed and GPU-accelerated Branch-and-Bound (B&B) for Permutation-based optimization problems

### General description

B&B is an exact algorithm which solves combinatorial optimization problems by dynamically constructing and exploring a search tree that implicitly enumerates all possible solutions.
Due to the combinatorial explosion of the search space, this requires massively parallel processing for all but the smallest problem instances.
PBBPerm provides parallel B&B algorithms for solving permutation-based optimization problems on **multi-core** processors, **GPUs** and large-scale **GPU-accelerated HPC systems**.

##### Data Structure
All algorithms are based on the **Integer-Vector-Matrix ([IVM](link)) data structure**, dedicated to permutation problems.
Compared to conventional data structures for B&B (stacks, priority queues, ...) the main advatage of IVM is its constant memory footprint.
With IVM it is not necessary to maintain pools of dynamically allocated search tree nodes - all memory required is allocated at initialization.

##### Load balancing

PBBPerm achieves parallelization mainly by exploring independent parts of the dynamically generated search tree in parallel.
However, as B&B uses pruning to reduce the size of the search space, the size and shape of the generated trees are highly irregular and unpredictable.
Therefore, naive approaches that statically assign different parts of the tree to workers are inefficient.
PBBPerm solves this problem by using the work stealing paradigm on different levels (CPU local, GPU local and inter-node). Work units exchanged between workers are integer-intervals instead of sets of nodes, which is particularly useful for efficient intra-GPU load balancing.

##### Performance

- [This figure](https://github.com/jangmys/PBBPerm/blob/master/figures/Ta21_timeline.pdf)
illustrates the evolution of the workload during a short (8 second) run of PBBPerm solving Taillards Flow-Shop instance Ta21 on 4 [Grid'5000](https://www.grid5000.fr) equipped with 2 P100 GPUs each.
The vertical axis represents the number of active explorers (each GPU-worker uses 16384 IVM-explorers). The small spikes correspond to work stealing operations inside the GPUs and large spikes occur when a worker run out of local work and requests new work from the master process.
(*Experiments were carried out using the [Grid'5000](https://www.grid5000.fr) testbed, supported by a scientific interest group hosted by Inria and including CNRS, RENATER and several Universities as well as other organizations.*)

- [This figure](https://github.com/jangmys/PBBPerm/blob/master/figures/ScalingOnJeanZay.pdf)
illustrates the scalability of PBBPerm on the [Jean Zay supercomputer](http://www.idris.fr/jean-zay/) with up to 384 V100 GPUs.
The resolution time for three 30-job FLow-Shop instances corresponding to different workloads (tree sizes from 122G to 3.7T decomposed nodes). For the larger instance the execution time is reduced from 26 hours on a single GPU to 8 minutes (note the log-scale). (*This work was granted access to the HPC resources of IDRIS under the allocation 2019-A0070611107 made by GENCI*)

### Components

The following describes the most important components and features of PBBPerm.

##### Bounding functions
The `./bounds` folder contains bounding functions for different problems.

Running `make` in that folder builds a library of lower bounds which can be tested independently in the `./bounds/test` folder.

PBBPerm allows to use combinations of two bounds (a weak and a strong one), as in [this work](https://hal.inria.fr/hal-02421229/) on the Flow-Shop scheduling problem (FSP).

For the moment, lower bounds for the FSP and n-queens (for testing purposes) are available. We plan to add more problems in the future.

## Sequential and Multi-core (pthreads)
The `./multicore` folder contains the CPU-only IVM-based B&B. It can be run in
- single-threaded mode (no pthreads)
- multi-threaded standalone mode
- multi-core worker mode (as part of a distributed execution)



## GPU (CUDA)
recompilation for different problems necessary (constant device memory)


## Distributed (MPI)

initmodes

checkpoints





## Other options...

#### Initial upper bound
Initialize at
- 0 : best-known cost (from file) / perm=id
- 1 : run heuristic (uses all available CPU cores)
- 2 : infinity / perm=id

This is done in pbab::buildInitialUB.

#### Sorting sibling nodes


#### Heuristic
Currently only for FSP

In addition to initial heuristic, one may run heuristic searches in parallel to the exploration
1: none
2: treeSearch
3: Iterated Local
