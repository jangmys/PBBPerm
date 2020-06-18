# PBBPerm
Parallel, distributed, GPU-accelerated Branch-and-Bound (B&B) for Permutation Problems

## General description

B&B is an exact algorithm which solves combinatorial optimization problems by dynamically constructing and exploring a search tree that implicitly enumerates all possible solutions.
Due to the combinatorial explosion of the search space, this requires massively parallel processing for all but the smallest problem instances.
PBBPerm provides parallel B&B algorithms for solving permutation-based optimization problems on multi-core processors, GPUs and large-scale GPU-accelerated HPC systems.

##### Data Structure
All algorithms are based on the **Integer-Vector-Matrix ([IVM](link)) data structure**, dedicated to permutation problems.
Compared to conventional data structures for B&B (stacks, priority queues, ...) the main advatage of IVM is its constant memory footprint.
With IVM it is not necessary to maintain pools of dynamically allocated search tree nodes - all memory required is allocated at initialization.

##### Load balancing

PBBPerm achieves parallelization mainly by exploring independent parts of the dynamically generated search tree in parallel.
However, as B&B uses pruning to reduce the size of the search space, the size and shape of the generated trees are highly irregular and unpredictable.
Therefore, naive approaches that statically assign different parts of the tree to workers are inefficient.
PBBPerm solves this problem by using the work stealing paradigm on different levels (CPU local, GPU local and inter-node). Work units exchanged between workers are integer-intervals instead of sets of nodes, which is particularly useful for efficient intra-GPU load balancing.

[PBBPerm running with 7 P100 worker GPUs](
https://github.com/jangmys/PBBPerm/blob/master/figures/Ta21_timeline.pdf)

The following describes the different components (reflected by the folder structure)

## Bounding functions
The `./bounds` folder contains bounding functions for different problems.

Running `make` in that folder builds a library of lower bounds which can be tested in the `./bounds/test` folder.

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
