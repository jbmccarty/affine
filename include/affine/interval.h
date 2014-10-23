#ifndef _INTERVAL_H
#define _INTERVAL_H

#include <istream>
#include <ostream>
#include <boost/numeric/interval.hpp>
#include <affine/config.h>

class IntervalException
{
public:
  void operator() ();
};

namespace bl = boost::numeric::interval_lib;

typedef
#ifdef USE_ROUNDING
  bl::save_state<bl::rounded_transc_opp<
#else
  bl::save_state_nothing<bl::rounded_transc_exact<
#endif
  Real> > round_type;

typedef boost::numeric::interval<Real, bl::policies<round_type,
  bl::checking_catch_nan<Real, bl::checking_no_empty<Real,
  bl::checking_base<Real>, IntervalException>, IntervalException> > >
  b_interval;

class Interval : public b_interval
{
public:
  Interval (Real a = Real()) : b_interval(a) {}
  Interval (Real a, Real b) : b_interval(a, b) {}
  Interval (const b_interval& a) : b_interval(a) {}
  const Real& lo () const;
  const Real& hi () const;
  class DomainError {};
};

namespace IntervalH {
  const Interval& pi (); //return an interval containing Pi
  const Interval& e (); //return an interval containing e (Euler's number)
}

// Global function declarations ------------------------------------------------

Interval inverse (const Interval&);
Interval root (const Interval&, unsigned int);

Interval ldexp (const Interval& a, int b); //a * 2^b

bool check_periods (const Interval& I, const Interval& offset,
    const Interval& period); //check whether I straddles any value of the form
    //(offset + n * period), n an integer

std::istream& operator>> (std::istream&, Interval&);
std::ostream& operator<< (std::ostream&, const Interval&);

#include <affine/interval_inline.h>
#endif //_INTERVAL_H
