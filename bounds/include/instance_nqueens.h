#include <stdlib.h>
#include <fstream>
#include <string.h>

#include "instance_abstract.h"


#ifndef INSTANCE_NQUEENS_H
# define INSTANCE_NQUEENS_H

struct instance_nqueens : public instance_abstract {
    instance_nqueens(const char * id);
    void
    generate_instance(int id, std::ostream& stream);
};

#endif
