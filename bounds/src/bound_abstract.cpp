#include "bound_abstract.h"

bound_abstract::~bound_abstract()
{ }

void
bound_abstract::set_instance(instance_abstract * _instance)
{
    instance = _instance;
}
