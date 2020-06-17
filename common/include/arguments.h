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
    //heuristic
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

    //UB initialization
    static int init_mode;
    static int initial_ub;

    static int initial_work;

    static int sortNodes;
    static int nodePriority;

    static int nbivms_mc;//la m^me ...
    static int nbivms_gpu;//chose ...

    //problem
    static char problem[50];
    static char inst_name[50];

    static char inifile[50];

    //FSP - johnson bound
    static bool earlyStopJohnson;
    static int johnsonPairs;

    static int singleNodeDS;

    static bool findAll;
    static bool localSearchOnLeaves;
    static bool intermediateLocalSearch;

    //verbosity
    static bool printSolutions;

    //truncate...
    static bool truncateSearch;
    static int truncateDepth;
    static int cut_bottom;
    static int cut_top;

    //work stealing
    static char mc_ws_select;
    static char gpu_ws_strategy;

    static void readIniFile();
    static bool parse_arguments(int argc, char **argv);
    static void initialize();
};

#endif
