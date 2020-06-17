# PBBPerm
Parallel, distributed, GPU-accelerated Branch-and-Bound (B&B) for Permutation Problems

## General description

B&B proceeds by implicit enumeration...


Data Structure
--------
The B&B algorithms in this repository are based on the Integer-Vector-Matrix (IVM) data structure [TODO: ref], dedicated to permutation problems.

Compared to conventional data structures for B&B (stacks, priority queues, ...) the main advatage of IVM is its constant memory footprint.

With IVM it is not necessary to maintain pools of dynamically allocated search tree nodes - all memory required is allocated at initialization.


Parallelization
---------
PBBPerm uses parallel tree exploration with work stealing between exploration agents.

the GPU version



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
