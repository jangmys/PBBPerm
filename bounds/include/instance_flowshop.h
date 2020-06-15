#ifndef INSTANCE_FLOWSHOP_H
#define INSTANCE_FLOWSHOP_H

#include "instance_abstract.h"

struct instance_flowshop : public instance_abstract {
    instance_flowshop(const char* inst);

    int get_job_number(int id);
    int get_machine_number(int id);
    long unif(long * seed, long low, long high);
    void generate_instance(int id, std::ostream& stream);
};

#endif
