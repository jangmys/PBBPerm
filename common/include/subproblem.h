#ifndef SUBPROBLEM_H
#define SUBPROBLEM_H

#include <iostream>

class subproblem {
public:
    subproblem(int size);
    subproblem(const subproblem& s);
    subproblem(const subproblem& father, int indice, int begin_end);

    ~subproblem();

    int limit1;
    int limit2;
    int * schedule;

    int size;
    int cost;
    int ub;
    int depth;

    int
    locate(const int job);

    bool
    simple()  const;
    bool
    leaf()  const;

    // int intRand(const int & min, const int & max);

    void
    print();
    void
    shuffle();

    void
    permutation_set(const subproblem& father, int indice, int begin_end);
    void
    limites_set(const subproblem& father, int begin_end);
    void
    copy(subproblem * p);

    subproblem&
    operator = (const subproblem& s);
};

std::ostream&
operator << (std::ostream& stream, const subproblem& s);
// std::istream& operator>>(std::istream& stream, sumproblem& s);

#endif // ifndef SUBPROBLEM_H
