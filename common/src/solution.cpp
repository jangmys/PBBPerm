#include "../include/pbab.h"
#include "../include/solution.h"
#include "../include/macros.h"

#include "../include/log.h"

solution::solution(pbab * _pbb)
{
    pbb  = _pbb;
    size = pbb->size;

    bestpermut = (int *)calloc(pbb->size,sizeof(int));
    for(int i=0;i<size;i++)
    {
        bestpermut[i]=i;
    }

    bestcost   = INT_MAX;
    newBest    = false;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&mutex_sol, &attr);
}

int
solution::update(int * candidate, int cost)
{
    int ret = 0;

    pthread_mutex_lock_check(&mutex_sol);
    if (cost < bestcost) {
        ret = 1;
        newBest  = true;
        bestcost = cost;
        if (candidate) {
            for (int i = 0; i < pbb->size; i++) bestpermut[i] = candidate[i];
        }
    }
    pthread_mutex_unlock(&mutex_sol);
    // print();
    return ret;
}

int
solution::updateCost(int cost)
{
    int ret = 0;

    pthread_mutex_lock_check(&mutex_sol);
    if (cost < bestcost) {
        ret = 1;
        bestcost = cost;
    }
    pthread_mutex_unlock(&mutex_sol);
    // print();
    return ret;
}

void
solution::getBestSolution(int *perm, int &cost)
{
    pthread_mutex_lock_check(&mutex_sol);
    cost = bestcost;
    for (int i = 0; i < pbb->size; i++)
        perm[i] = bestpermut[i];
    pthread_mutex_unlock(&mutex_sol);
}

int
solution::getBest()
{
    int ret;
    pthread_mutex_lock_check(&mutex_sol);
    ret=bestcost;
    pthread_mutex_unlock(&mutex_sol);
    return ret;
}

void
solution::getBest(int& cost)
{
    // printf("bestcost: %d\n",bestcost);
    if (bestcost == cost) return;

    if (bestcost < cost) {
        cost = bestcost;
        return;
    }
}

void
solution::print()
{
    pthread_mutex_lock_check(&mutex_sol);
    printf("Cost: %d\t\t", bestcost);
    for (int i = 0; i < pbb->size; i++) {
        printf("%3d\t", bestpermut[i]);
    }
    printf("\n");
    fflush(stdout);
    pthread_mutex_unlock(&mutex_sol);
}

void
solution::save()
{
    FILE_LOG(logINFO) << "SAVE SOLUTION " << this->bestcost;
    std::ofstream stream(("./bbworks/sol" + std::string(arguments::inst_name) + ".save").c_str());

    stream << *this <<std::endl;

    // stream << *(pbb->sltn);
    // stream << *this;
    // std::cout<<"SAVE "<<totalNodes<<std::endl;
    stream.close();
}

solution&
solution::operator=(solution& s)
{
    pbb  = s.pbb;
    size = s.size;

    bestcost   = s.bestcost;
    newBest    = s.newBest;

    for(int i=0;i<size;i++)
    {
        bestpermut[i]=s.bestpermut[i];
    }
    return *this;
}



// write solution to stream
std::ostream&
operator << (std::ostream& stream, const solution& s)
{
    stream << s.size << std::endl;
    stream << s.bestcost << std::endl;
    for (int i = 0; i < s.size; i++) {
        stream << s.bestpermut[i] << " ";
    }
    stream << std::endl;

    return stream;
}

// read solution from stream
std::istream&
operator >> (std::istream& stream, solution& s)
{
    stream >> s.size;
    stream >> s.bestcost;
    for (int i = 0; i < s.size; i++) {
        stream >> s.bestpermut[i];
    }
    return stream;
}
