#include <string>
#include <iostream>
#include <sstream>

#include "../../common/headers/pbab.h"

#include "../headers/weights.h"

//0 (N-1)!  2*(N-1)!    3*(N-1)!    ... (N-1)*(N-1)! N*(N-1)!
//...
//24
//6
//2
//1
//0 1   2   3   4   5   ... N-1 N
weights::weights(pbab* pbb)
{
    depth[pbb->size]     = 1;
    depth[pbb->size - 1] = 1;
    for (int i = pbb->size - 2, j = 2; i >= 0; i--, j++) {
        depth[i]  = depth[i + 1];
        depth[i] *= j;
    }
    // std::cout<<depth[0]<<"\n";
    for (int i = 0; i <= pbb->size; i++) {
        for (int j = 0; j <= pbb->size; j++) {
            W[i][j] = j * depth[i];
           // std::cout<<W[i][j]<<" ";
        }
       // std::cout<<std::endl;
    }
}
