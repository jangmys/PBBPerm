#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "instance_abstract.h"
#include "instance_vrf.h"

instance_vrf::instance_vrf(const char * inst_name)
{
    data = new std::stringstream();

    const char vrfdirname[] = "./parameters/vrf_parameters/";
    const char ext[]        = "_Gap.txt";

    file = (char *) malloc(strlen(inst_name) + strlen(vrfdirname) + strlen(ext) + 1); /* make space for the new string*/

    strcpy(file, vrfdirname);/* copy dirname into the new var */
    strcat(file, inst_name); /* add the instance name */
    strcat(file, ext);       /* add the extension */

    generate_instance(file, *data);

    free(file);
}

instance_vrf::~instance_vrf()
{
    delete data;
}

void
instance_vrf::generate_instance(const char * _file, std::ostream& stream)
{
    std::ifstream infile(_file);

    int nbMachines = 0;

    if (infile.is_open()) {
        infile.seekg(0);
        if (!infile.eof()) infile >> size;
        if (!infile.eof()) infile >> nbMachines;

        if (nbMachines){
            stream << size << " " << nbMachines << " ";
        } else {
            perror("infile read error"); exit(1);
        }

        int tmp[size * nbMachines];
        int c = 0;

        while (1) {
            int m;
            infile >> m >> tmp[c++];
            if (infile.eof()) break;
        }

        // transpose
        for (int i = 0; i < nbMachines; i++) {
            for (int j = 0; j < size; j++) {
                stream << tmp[j * nbMachines + i] << " ";
            }
        }

    }else {
        std::cout << "Error opening file: " << std::string(_file) <<"\n";
        exit(1);
    }
}
