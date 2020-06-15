#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <climits>
#include <getopt.h>

#include "../inih/INIReader.h"

class arguments
{
public:
    static int heuristic_threads;
    static int heuristic_iters;
    static int initial_heuristic_iters;

    static int branchingMode;
    static int boundMode;

    static char type;       //'g'PU or 'c'PU
    static bool singleNode; //no MPI = 1 ; distributed = 0

    static int instancev;//obsolete : instance as char*
    static int checkpointv;
    static int balancingv;
    static bool mc_timeout;

    static int init_mode;
    static int initial_ub;

    static int nbivms_mc;//la m^me ...
    static int nbivms_gpu;//chose ...

    static char problem[50];
    static char inst_name[50];
    static char inifile[50];

    static int sortNodes;
    static bool earlyStopJohnson;
    static int johnsonPairs;
    static int nodePriority;

    static int singleNodeDS;

    static bool findAll;
    static bool printSolutions;
    static bool localSearchOnLeaves;
    static bool intermediateLocalSearch;

    static bool truncateSearch;
    static int truncateDepth;

    static int cut_bottom;
    static int cut_top;

    static void readIniFile();
    static bool parse_arguments(int argc, char **argv);
    static void initialize();

    static char mc_ws_select;
    static char gpu_ws_strategy;

};

#endif
