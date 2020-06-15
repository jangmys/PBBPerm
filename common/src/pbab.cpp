// ==========================================================================================
#include <pthread.h>
#include <semaphore.h>
#include <sys/sysinfo.h>

#include "../include/solution.h"
#include "../include/ttime.h"
#include "../include/pbab.h"
#include "../include/log.h"

pbab::pbab()
{
    // printf("===== pbab constr; ============\n"); fflush(stdout);
    stats.totDecomposed = ATOMIC_VAR_INIT(0);
    stats.johnsonBounds = ATOMIC_VAR_INIT(0);
    stats.simpleBounds  = ATOMIC_VAR_INIT(0);
    stats.leaves        = ATOMIC_VAR_INIT(0);

    foundSolution=false;

    this->ttm = new ttime();

    pthread_mutex_init(&mutex_instance, NULL);

    set_instance(arguments::problem, arguments::inst_name);
    size     = instance->size;

    sltn = new solution(this);
    sltn->bestcost = arguments::initial_ub;

    root_sltn = new solution(this);
    root_sltn->bestcost = arguments::initial_ub;

    if (arguments::problem[0] == 'n')
        sltn->bestcost = 0;

    ils=new IG(instance);
}

void
pbab::printStats()
{
    if(arguments::singleNode){
        printf("=================\n");
        printf("Exploration stats\n");
        printf("=================\n");

        // printf("\t######Total %d\n", M);
        std::cout << "TOTdecomposed:\t " << stats.totDecomposed << std::endl;
        std::cout << "TOTjohnson LBs:\t " << stats.johnsonBounds << std::endl;
        std::cout << "TOTsimple LBs:\t " << stats.simpleBounds << std::endl;
        std::cout << "TOTleaves:\t " << stats.leaves << std::endl;

        if (arguments::branchingMode > 0) {
            printf("AvgBranchingFactor:\t %f\n", (double) (stats.simpleBounds / 2) / stats.totDecomposed);
            printf("LB1-PruningRate:\t %f\n", 1.0 - (double) stats.totDecomposed / (stats.simpleBounds / 2));

            if (arguments::boundMode == 2)
                printf("LB2-SuccessRate:\t %f\n",
                  (double) (stats.johnsonBounds - stats.totDecomposed) / stats.totDecomposed);
        }

		std::cout<<"\n";
        if(foundSolution)
        {
            std::cout<<"Found optimal solution."<<std::endl;
			std::cout<<*(sltn);
        }else{
        	std::cout<<"Not improved..."<<std::endl;
        	std::cout<<"Optimal makespan is >= "<<sltn->bestcost<<" (initial solution) "<<std::endl;
        }
    }else{
        printf("shutting down\n");
    }
}

pbab::~pbab()
{
    delete instance;
    delete ils;
}

void pbab::reset()
{
    stats.totDecomposed = 0;
    stats.johnsonBounds = 0;
    stats.simpleBounds  = 0;
    stats.leaves        = 0;

    foundSolution=false;
}

void pbab::set_instance(char problem[],char inst_name[])
{
    switch(problem[0])//DIFFERENT PROBLEMS...
    {
        case 'f': //FLOWSHOP
        {
            switch (inst_name[0]) {//DIFFERENT INSTANCES...
                case 't': {
                    instance = new instance_flowshop(inst_name);
                    break;
                }
                case 'V': {
                    instance = new instance_vrf(inst_name);
                    break;
                }
            }
            break;
        }
        case 'n': //NQUEENS
        {
            instance = new instance_nqueens(inst_name);
            break;
        }
        // add other problems...
        case 't': //TEST
        {
            instance = new instance_nqueens(inst_name);
            break;
        }
    }
}

void pbab::set_heuristic()
{

}


void *
ig_thread(void * _pbb)
{
    pbab * pbb = (pbab *) _pbb;

    pthread_mutex_lock(&pbb->mutex_instance);
    IG* ils=new IG(pbb->instance);
    pthread_mutex_unlock(&pbb->mutex_instance);

    subproblem *s=new subproblem(pbb->size);
    helper::shuffle(s->schedule,pbb->size);

    //check that parallel RNG generates different subpbs
    // std::cout<<"SUBPR ]]]]]]]]] "<<*s<<std::endl;

    s->limit1=-1;
    s->limit2=pbb->size;

    ils->igiter=arguments::initial_heuristic_iters;
    ils->neh->runNEH(s->schedule, s->cost);
    // std::cout<<"NEH ]]]]]]]]] "<<*s<<std::endl;

    int c=ils->runIG(s);
    // std::cout<<"IG ]]]]]]]]] "<<*s<<std::endl;

    pbb->sltn->update(s->schedule,c);

    delete s;
    return NULL;
}


void
pbab::buildInitialUB(){
    sltn->bestcost=9999999; //temporary...

    unsigned int N=get_nprocs_conf();
    pthread_t *thds=(pthread_t*)malloc(N*sizeof(pthread_t));
	//[N];

    FILE_LOG(logINFO) << "Run initial heuristic with " << N << " threads";

    for (unsigned int i = 0; i < N; i++)
        pthread_create(&thds[i], NULL, ig_thread, (void *) this);

    for (unsigned int i = 0; i < N; i++)
    {
        int err = pthread_join(thds[i], NULL);

        if (err)
        {
            FILE_LOG(logERROR) << "Failed to join Thread : " << strerror(err);
        }
    }

    // subproblem *s=new subproblem(size);
    // s->limit1=-1;
    // s->limit2=size;
    //
    // ils->igiter=arguments::initial_heuristic_iters;
    //
    // sltn->bestcost = 999999;
    // sltn->update(s->schedule,999999);
    //
    // ils->neh->runNEH(s->schedule, s->cost);
    //
    // int c=ils->runIG(s);
    //
    // if (c<sltn->bestcost){
    //     sltn->update(s->schedule,c);
    // }
    //
    // delete s;

	free(thds);
}
