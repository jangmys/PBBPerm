#ifndef SOLUTION_H
#define SOLUTION_H

#include <iostream>
#include <climits>
#include <pthread.h>

/*
this class and subproblem are somewhat redundant....
*/

class pbab;

class solution{
public:
    solution(pbab* pbb);

    pbab*pbb;

    int size;

    int cost;
    int* perm;

    pthread_mutex_t mutex_sol;
    bool newBest;

    int update(const int* candidate, const int cost);
    int updateCost(const int cost);

    void getBestSolution(int* permut, int &cost);

    int getBest();
    void getBest(int& cost);
    void print();

    void save();
    void random();

    solution& operator=(solution& s);
};

std::ostream&  operator<<(std::ostream& stream,const solution& s);
std::istream& operator>>(std::istream& stream, solution& s);
#endif
