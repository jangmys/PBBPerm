#include <sys/stat.h>

#include "../include/arguments.h"

int arguments::heuristic_threads = 1;
int arguments::heuristic_iters   = 1;
int arguments::initial_heuristic_iters = 100;
char arguments::heuristic_type = 'n';

int arguments::boundMode     = 2;
int arguments::branchingMode = 3;

int arguments::instancev;
int arguments::checkpointv = 1;
int arguments::balancingv  = 1;
bool arguments::mc_timeout = false;

int arguments::initial_ub;

int arguments::init_mode = 1;

int arguments::nbivms_mc  = -1;
int arguments::nbivms_gpu = 4096;

char arguments::inst_name[50];
char arguments::problem[50];
char arguments::inifile[50] = "./bbconfig.ini";

char arguments::type = 'c';

int arguments::sortNodes         = 1;
bool arguments::earlyStopJohnson = true;
int arguments::johnsonPairs      = 1;

int arguments::nodePriority = 1;

bool arguments::findAll        = false;
bool arguments::printSolutions = true;

bool arguments::localSearchOnLeaves     = false;
bool arguments::intermediateLocalSearch = false;

bool arguments::singleNode = !true;// false;

char arguments::gpu_ws_strategy = 'z';
char arguments::mc_ws_select    = 'o';

int arguments::truncateDepth   = 0;
bool arguments::truncateSearch = false;
int arguments::cut_top         = 99999;
int arguments::cut_bottom      = -1;


int arguments::initial_work = 3;

/***********************************************************************/
/***********************************************************************/
/***********************************************************************/
/***********************************************************************/
/***********************************************************************/

std::string
sections(INIReader &reader)
{
    std::stringstream ss;
    std::set<std::string> sections = reader.Sections();
    for (std::set<std::string>::iterator it = sections.begin(); it != sections.end(); ++it)
        ss << *it << ",";
    return ss.str();
}

void
arguments::readIniFile()
{
    std::string str(inifile);
    std::cout << "reading config from:\t" << str << std::endl;

    INIReader reader(str);
    if (reader.ParseError() < 0) {
        std::cout << "Can't load " << str << "\n";
        return;
    }
    // read strings
    strncpy(problem, reader.Get("problem", "problem", "UNKNOWN").c_str(), 50);
    strncpy(inst_name, reader.Get("problem", "instance", "UNKNOWN").c_str(), 50);
    printf("%s / %s\n", problem, inst_name);

    // read integers
    checkpointv = reader.GetInteger("time", "checkpoint", 1);// default values;
    mc_timeout  = reader.GetBoolean("time", "timeout", false);// default values;
    balancingv  = reader.GetInteger("time", "balance", 1);

    // if(!mc_timeout){ //timeout only for singleNode
    //   balancingv=INT_MAX;
    // }

    init_mode = reader.GetInteger("initial", "ub", -1);

    // nb IVMs
    // nbivm_mc   = reader.GetInteger("multicore", "threads", -1);
    nbivms_mc  = reader.GetInteger("multicore", "threads", -1);
    nbivms_gpu = reader.GetInteger("gpu", "nbIVMs", 4096);

    // sorting
    sortNodes    = reader.GetInteger("bb", "sortedDFS", 1);
    nodePriority = reader.GetInteger("bb", "sortingCriterion", 1);

    earlyStopJohnson = reader.GetBoolean("bb", "earlyStopJohnson", true);
    boundMode        = reader.GetInteger("bb", "boundingMode", 2);
    johnsonPairs     = reader.GetInteger("bb", "JohnsonMode", 1);

    findAll    = reader.GetBoolean("bb", "findAll", false);
    singleNode = reader.GetBoolean("bb", "singleNode", false);
    if (singleNode)
        std::cout << "Single-node mode" << std::endl;

    branchingMode = reader.GetInteger("bb", "adaptiveBranchingMode", 3);

    printSolutions = reader.GetBoolean("verbose", "printSolutions", false);
    // if(printSolutions)
    //     std::cout<<"Printing Solutions..."<<std::endl;

    mc_ws_select = *(reader.Get("multicore", "worksteal", "r").c_str());
    type         = reader.Get("bb", "type", "c")[0];

    truncateDepth  = reader.GetInteger("truncate", "truncDepth", 0);
    truncateSearch = reader.GetBoolean("truncate", "truncSearch", false);
    cut_bottom     = reader.GetInteger("truncate", "cutoff_bottom", -1);
    cut_top        = reader.GetInteger("truncate", "cutoff_top", INT_MAX);

    heuristic_threads       = reader.GetInteger("heuristic", "heuristic_threads", 1);
    initial_heuristic_iters = reader.GetInteger("heuristic", "initial_heuristic_iters", 1000);
    heuristic_iters         = reader.GetInteger("heuristic", "heuristic_iters", 1000);
    heuristic_type         = reader.Get("heuristic", "heuristic_type", "n")[0];

    initial_work = reader.GetInteger("distributed", "initialWork", 3);

} // arguments::readIniFile

inline bool
fexists(const std::string& name)
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

void
arguments::initialize()
{
    std::stringstream rubrique;
    std::string tmp;
    int jobs, machines, valeur, no, lb;

    std::ifstream infile;

    initial_ub = INT_MAX;

    // flowshop Taillard w/ BKSol initialization
    if (inst_name[0] == 't' && init_mode == 0) {
        if (fexists("./parameters/instances.data")){
            infile = std::ifstream("./parameters/instances.data");
        }else if (fexists("../parameters/instances.data")){
            infile = std::ifstream("../parameters/instances.data");
        }else{
            std::cout<<"Trying to read best-known solution from ./parameters/instances.data failed\n";
        }

        int id=atoi(&inst_name[2]);
        rubrique << "instance" << id << "i";

        while (!infile.eof()) {
            string str;
            getline(infile, str);

            if (str.substr(0, rubrique.str().length()).compare(rubrique.str()) == 0) {
                std::stringstream buffer;
                buffer << str << std::endl;
                buffer >> tmp >> jobs >> machines >> valeur;
                break;
            }
        }
        infile.close();
        initial_ub=valeur;
    }

    // flowshop VRF
    if (inst_name[0] == 'V') {
        tmp      = "";
        jobs     = 0;
        machines = 0;
        valeur   = 0;
        rubrique << inst_name;// ance;

        //        std::cout<<rubrique.str()<<std::endl;
        std::ifstream infile("./parameters/instancesVRF.data");

        while (!infile.eof()) {
            string str;
            getline(infile, str);

            // std::cout<<str<<std::endl;
            //            std::cout<<str.substr(0, rubrique.str().length())<<std::endl;
            //            break;

            if (str.substr(0, rubrique.str().length()).compare(rubrique.str()) == 0) {
                std::stringstream buffer;
                buffer << str << std::endl;
                buffer >> tmp >> jobs >> machines >> no >> valeur >> lb;
                //                std::cout<<tmp<<" "<<jobs<<" "<<machines<<" "<<valeur<<std::endl;
                break;
            }
        }
        infile.close();

        initial_ub = (init_mode == 0) ? valeur : INT_MAX;
    }
} // arguments::initialize

#define OPTIONS "z:ftm" // vrtnqbiowcdugmsfh"
bool
arguments::parse_arguments(int argc, char ** argv)
{
    bool ok = false;

    char * subopts, * value;

    enum { PROBLEM = 0, INST, OPT };
    char * const problem_opts[] = {
        [PROBLEM] = (char *) "p",
        [INST]    = (char *) "i",
        [OPT]     = (char *) "o",
        NULL
    };

    int c = getopt_long(argc, argv, OPTIONS, NULL, NULL);

    while (c != -1) {
        switch (c) {
            case 'z': {
                subopts = optarg;
                while (*subopts != '\0')
                    switch (getsubopt(&subopts, problem_opts, &value)) {
                        case PROBLEM:
                            strcpy(problem, value);
                            break;
                        case INST:
                            strcpy(inst_name, value);
                            break;
                        case OPT:
                            init_mode = 0;
                            break;
                    }
                ok = true;
                break;
            }
            case 'f': {
                // strcpy(inifile, argv[optind]);
                // std::cout<<"inifile:\t"<<inifile<<std::endl;
                break;
            }
            case 't': {
                type = argv[optind][0];
                // printf("Type %c\n",type);
                break;
            }
            case 'm': {
                singleNode = true;
                break;
            }
        }
        c = getopt_long(argc, argv, OPTIONS, NULL, NULL);
    }

    return ok;
}
