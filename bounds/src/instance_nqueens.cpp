#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>

#include "instance_abstract.h"
#include "instance_nqueens.h"

void instance_nqueens::generate_instance(int id, std::ostream& stream)
{
	long N=id;

	stream << N << " ";

	std::cout<<"id="<<id<<std::endl;
}

instance_nqueens::instance_nqueens(const char* id)
{
	size=atoi(id);
	data=new std::stringstream();

	generate_instance(size,*data);
}
