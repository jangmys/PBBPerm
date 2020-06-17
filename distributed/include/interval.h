#ifndef INTERVAL_H
#define INTERVAL_H

#include <vector>
#include <memory>

#include "gmp.h"
#include "gmpxx.h"

class weights;

class interval {
public:
  interval();
  interval(mpz_class begin, mpz_class end,int id);
  interval(std::string b,std::string e, std::string _id);
  interval(const interval& i);

  //an INTERVAL:
  int id;
  mpz_class begin;
  mpz_class end;

  bool operator < (const interval& in) const
  { return (begin < in.begin);};

  mpz_class length() const;

  bool disjoint(interval *i) const;

  void operator=(interval& i);
  bool operator==(interval& i) const;
};

#endif
