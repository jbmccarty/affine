#ifndef _UTIL_H
#define _UTIL_H

#include <cmath>
#include <config.h>

inline Real root (Real base, unsigned int r)
{ 
  return std::copysign(std::pow(std::fabs(base), Real(1) / r), base);
}

#endif //_UTIL_H
