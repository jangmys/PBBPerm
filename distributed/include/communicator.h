#include <string.h>
#include <mpi.h>

#include <memory>

#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

class pbab;
class work;
class solution;

class communicator{
public:
    pbab* pbb;
    communicator(int,pbab*);
    ~communicator();

    int rank;
    int M;
    int size;

    std::shared_ptr<work> dwork_buf;
    solution * best_buf;

    void send_fwork(int dest, int tag);
    void recv_fwork(int src, int tag, MPI_Status* status);

    void send_work(std::shared_ptr<work> src_wrk, int dest, int tag);
    void recv_work(std::shared_ptr<work> dst_wrk, int src, int tag, MPI_Status* status);

    void send_sol(solution* sol, int dest, int tag);
    void recv_sol(solution* sol, int dest, int tag, MPI_Status* status);
};

#endif
