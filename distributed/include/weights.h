#include <string>
#include <iostream>
#include <sstream>

#include "gmp.h"
#include "gmpxx.h"

#define MAX_JOBS 800

#ifndef weightS_H
#define weightS_H
class pbab;

class weights
{
public:
    mpz_class depth[MAX_JOBS+1];
    mpz_class W[MAX_JOBS+1][MAX_JOBS+1];
    weights(pbab* pbb);
};
#endif
