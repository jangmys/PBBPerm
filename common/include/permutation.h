#ifndef PERMUTATION_H_
#define PERMUTATION_H_

class permutation{
public:
    int N;
    int *arr;

    permutation(int N);
    permutation(const int* a,int N);
    ~permutation();

    void id();
    void random();
    void print();

    bool isValid();

    void copy(permutation *p);

    int& operator[](int);

};

#endif
