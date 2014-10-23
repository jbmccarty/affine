#include <cmath>
#include <affine/affine.h>
#include "devlist.h"
#include "util.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

DevAllocator DevList::allocator_;

namespace AffineH
{
  const int Verbosity::fmt_index_ = std::ios_base::xalloc();
  const Verbosity verbose(false);
  const Verbosity terse(true);
  
  std::ostream& operator<< (std::ostream& o, const Verbosity& v)
  {
    o.iword(Verbosity::fmt_index_) = v.setting_;
    return o;
  }
}

void Affine::calc (const Real* slope, const Real* intercept, const Real* error,
    const Interval* range)
{
  if (slope) *this *= *slope;
  if (intercept) *this += *intercept;
  if (error) {
    Real abserror = std::fabs(*error);
    devs_.new_dev(abserror);
    interval_ += Interval(-abserror, abserror);
  }
  if (range) interval_ = intersect(interval_, *range);
}

Affine::Affine (Real val) : center_(val), interval_(val), devs_(*(new DevList))
{}

Affine::Affine (Real c, Real dev) : center_(c),
    interval_(c - fabs(dev), c + fabs(dev)), devs_(*(new DevList(dev)))
{}

Affine::Affine (const Interval& I) : center_((I.lo() + I.hi()) / 2),
    interval_(I), devs_(*(new DevList(width(I) / 2)))
{}

Affine::Affine (const Affine& a) : center_(a.center_), interval_(a.interval_),
    devs_(*(new DevList(a.devs_)))
{}

Affine::~Affine ()
{
  delete &devs_; //Memory management could be better
}

Affine& Affine::operator= (const Affine& a)
{
  center_ = a.center_;
  interval_ = a.interval_;
  devs_ = a.devs_;
  return *this;
}

Affine& Affine::operator+= (const Affine& a)
{
//  collapse(); //XXX: make runtime flag
	center_ += a.center_;
	DevList::iterator i1 = devs_.begin();
	DevList::const_iterator i2 = a.devs_.begin();
	while ((i2 != a.devs_.end()) && (i1 != devs_.end()))
	{
		if (i2->first < i1->first)
			i1 = devs_.insert(i1, *i2++);
      //careful--palist iterators are more volatile than in std::list
    else if (i2->first == i1->first) {
			i1->second += i2->second; //XXX: round away from zero
			++i2;
		}
    ++i1;
  }
  for (; i2 != a.devs_.end(); ++i2)
		devs_.push_back(*i2);

	Real H = radius();
  Interval range(-H, +H); //XXX: could put in loop for efficiency
  range += center_;
	interval_ += a.interval_;
	interval_ = intersect(interval_, range);
	return *this;
}

Affine& Affine::operator-= (const Affine& a)
{
//  collapse(); //XXX: Make runtime flag
	center_ -= a.center_;
	DevList::iterator i1 = devs_.begin();
	DevList::const_iterator i2 = a.devs_.begin();
	while ((i2 != a.devs_.end()) && (i1 != devs_.end()))
	{
		if (i2->first < i1->first)
		{
			i1 = devs_.insert(i1, std::make_pair(i2->first, -i2->second));
			++i2;
		}
		else if (i2->first == i1->first)
		{
			i1->second -= i2->second; //XXX: round away from zero
			++i2;
		}
		++i1;
	}
  for (;i2 != a.devs_.end(); ++i2)
		devs_.push_back(std::make_pair(i2->first, -i2->second));

	Real H = radius();
  Interval range(-H, +H);
  range += center_;
	interval_ -= a.interval_;
	interval_ = intersect(interval_, range);
	return *this;
}

Affine& Affine::operator/= (const Affine& a)
{
//	collapse(); //XXX: should I put this in DevList?
	*this *= inverse(a);
	return *this;
}

Affine& Affine::operator= (Real a)
{
  center_ = a;
  interval_ = a;
  devs_.clear();
  return *this;
}

Affine& Affine::neg ()
{
  center_ = -center_;
  interval_ = -interval_;
  for (DevList::iterator i = devs_.begin(); i != devs_.end(); ++i)
    i->second = -i->second;
  return *this;
}

Affine& Affine::ldexp (int b)
{
  center_ = std::ldexp(center_, b);
  interval_ = ::ldexp(interval_, b);
  for (DevList::iterator i = devs_.begin(); i != devs_.end(); ++i)
    i->second = std::ldexp(i->second, b);
  return *this;
}

Affine& Affine::operator*= (Real a)
{
	center_ *= a;
	interval_ *= a;
	for (DevList::iterator i = devs_.begin(); i != devs_.end(); ++i)
		i->second *= a; //XXX: round away from zero
//	RemoveEmptyDevs();
	return *this;
}

Affine& Affine::operator*= (const Affine& a)
{
  Affine temp(a);
  Real old_center = center_;
  Real slope = a.center_, intercept = -center_ * a.center_, error = radius()
    * a.radius(); //round up
  Interval range(interval_ * a.interval_);
  calc(&slope, &intercept, &error, 0);
  temp *= old_center;
  *this += temp;
  interval_ = intersect(interval_, range);
  return *this;
}

const DevList& Affine::get_devs () const
{ return devs_; }

Affine& inverseA (Affine& a)
{
  if ((a.lo() <= 0.0) && (a.hi() >= 0.0))
    throw Affine::DomainError();
  if (a.lo() == a.hi())
    a = 1.0 / a.lo();
  else { //mathematical simplification of straightforward method
    Interval range(inverse(Interval(a)));
    Real lohi = range.lo() * range.hi(),
    slope = -lohi,
    f_0_2 = std::copysign(std::sqrt(lohi), a.lo()),
    r_0_2 = 0.5 * (range.lo() + range.hi()),
    intercept = f_0_2 + r_0_2,
    error = f_0_2 - r_0_2;
    a.calc(&slope, &intercept, &error, &range);
  }
  return a;
}

Real Affine::radius () const
{
	Real sum = 0.0; //XXX: possibly add interval Center width
	for (DevList::const_iterator i = devs_.begin(); i != devs_.end(); ++i)
		sum += std::fabs(i->second);
	return sum;
}

void Affine::collapse ()
{
	DevList::iterator i = devs_.begin();
	for (; i != devs_.end(); ++i)
	{
		if (devs_.get_refcount(i) == 1)
			break;
	}
	if (i != devs_.end())
	{
		DevList::iterator sum = i;
		sum->second = std::fabs(sum->second);
		for (++i; i != devs_.end();)
		{
			if (devs_.get_refcount(i) == 1)
			{
				sum->second += std::fabs(i->second);
				i = devs_.erase(i);
			}
			else
				++i;
		}
	}
}

Affine& expA (Affine& a)
{
  if (a.lo() == a.hi())
    a = std::exp(a.lo());
  else {
    Interval range = exp(Interval(a));
    Real slope = width(range) / width(a),
    f_0 = slope * (1.0 - std::log(slope)),
    r_0 = range.lo() - slope * a.lo(),
    intercept = 0.5 * (r_0 + f_0),
    error = 0.5 * (r_0 - f_0);

    a.calc(&slope, &intercept, &error, &range);
  }
  return a;
} 

Affine& logA (Affine& a)
{
  if (a.lo() <= 0.0)
    throw Affine::DomainError();
	if (a.lo() == a.hi())
		a = std::log(a.lo());
  else {
  	Interval range(log(Interval(a)));
  	Real slope = width(range) / width(a),
    f_0 = -1.0 - std::log(slope),
    r_0 = range.lo() - slope * a.lo(),
    intercept = 0.5 * (f_0 + r_0),
    error = 0.5 * (f_0 - r_0);
  	a.calc(&slope, &intercept, &error, &range);
  }
	return a;
}

Affine& sqrtA (Affine& a)
{
  if (a.lo() < 0.0)
    throw Affine::DomainError();
  if (a.lo() == a.hi())
    a = std::sqrt(a.lo());
  else {
    Interval range(sqrt(Interval(a)));
  	Real lph = range.lo() + range.hi();
  	Real slope = 1.0 / lph;
  	Real intercept = 0.125 * lph + 0.5 * range.lo() * range.hi() * slope;
  	Real error = 0.125 * std::pow(width(range), 2) * slope;
  	a.calc(&slope, &intercept, &error, &range);
  }
	return a;
}

Affine& sqrA (Affine& a)
{
  if (a.lo() == a.hi())
    a = a.lo() * a.lo();
  else {
    Interval range(pow(Interval(a), 2));
    Real slope = a.lo() + a.hi(),
    f_0_2 = -0.125 * std::pow(slope, 2),
    r_0_2 = -0.5 * a.lo() * a.hi(),
    intercept = f_0_2 + r_0_2,
    error = f_0_2 - r_0_2;
  
    a.calc(&slope, &intercept, &error, &range);
  }
  return a;
}

Affine& powA (Affine& base, unsigned int exponent)
{
  switch (exponent) {
    case 0: base = 1.0; break;
    case 1: break;
  default:
    if (base.lo() == base.hi())
      base = std::pow(base.lo(), (int)exponent);
    else {
      Interval range(pow(Interval(base), (int)exponent)); //XXX: annoying duplication
      Real f_lo = std::pow(base.lo(), (int)exponent),
           f_hi = std::pow(base.hi(), (int)exponent),
          slope = (f_hi - f_lo) / width(base),
              u = root(slope / exponent, exponent - 1),
            f_0 = std::pow(u, (int)exponent) - slope * u;

      if ((exponent & 1) && (-u > base.lo()) && (u < base.hi()))
        base.calc(&slope, 0, &f_0, &range);
        //tangent at 2 points--no solution to n*u^(n - 1) = (x^n - u^n)/(x - u),
        //although it has the form u = x*f(n), where f(n) ~= -n^(1/(1 - n))
      else { //tangent at one point
        if ((exponent & 1) && (u > base.hi())) //we chose the wrong u
          f_0 = -f_0;
        Real r_0 = f_lo - slope * base.lo(),
             intercept = std::ldexp(f_0 + r_0, -1),
             error = std::ldexp(f_0 - r_0, -1);
        base.calc(&slope, &intercept, &error, &range);
      }
    }
  }
  return base;
}

Affine& rootA (Affine& base, unsigned int r)
{
  if (!(r & 1) && (base.lo() < 0.0))
    throw Affine::DomainError();
  if (base.lo() == base.hi())
    base = Affine(root(base.lo(), r));
  else {
    Interval range(root(Interval(base), r));
    Real slope = width(range) / width(base),
             u = std::pow(r * slope, r / (Real(1) - r)),
           f_0 = root(u, r) - slope * u;

    if ((-u > base.lo()) && (u < base.hi()))
      base.calc(&slope, 0, &f_0, &range); //same approximation as with powA
    else { //tangent at one point
      if (-u > base.lo())
        f_0 = -f_0;
      Real r_0 = range.lo() - slope * base.lo(),
           intercept = std::ldexp(f_0 + r_0, -1),
           error = std::ldexp(f_0 - r_0, -1);
      base.calc(&slope, &intercept, &error, &range);
    }
  }
  return base;
}

inline unsigned int gcd (unsigned int a, unsigned int b)
{
  while (b != 0) {
    unsigned int c = a % b;
    a = b;
    b = c;
  }
  return a;
}

Affine& powA (Affine& base, int numerator, unsigned int denominator)
{
  if (denominator == 0)
    throw Affine::DomainError();
  unsigned int g = gcd(numerator, denominator);
  numerator /= g;
  denominator /= g;
  rootA(base, denominator);
  return powA(base, numerator);
}

Affine& absA (Affine& a)
{
  if (a.hi() <= 0.0)
    negA(a);
  else if (a.lo() < 0.0) {
    const Interval range(abs(Interval(a)));
    Real slope = (a.hi() + a.lo()) / width(a);
    Real intercept = 0.5 * a.hi() * (1.0 - slope); //same as error
    a.calc(&slope, &intercept, &intercept, &range);
  }
  return a;
}

void symsincos (const Affine& a, Affine& sinres, Affine& cosres)
//sine and cosine of a zero-centered form; a, sinres, and cosres must
//not be at the same address. (a) must have non-zero width.
{
  Real sinh, cosh;
  mysincos(a.hi(), &sinh, &cosh);

  cosres = Affine((1 + cosh) / 2, (1 - cosh) / 2);
  //FIXME: write sincos for interval and use it.

  //There doesn't seem to be an explicit solution to the equation
  //  slope * (acos(slope) + a.hi()) == sqrt(1 - slope^2) + sin(a.hi()),
  //but the equation   slope = 0.5(cos(2*a.hi()/3) + 1)   comes very
  //close. The additional error it introduces is very roughly
  //  -0.001 * a.hi() * (a.hi() - 3pi/2) * (a.hi() + 4.1), or less than
  //0.016 * a.hi().
  Interval range(sin(Interval(a)));
  Real slope = 0.5 * (std::cos(2 * a.hi() / 3) + 1),
  //slope is an overestimate, so error must overshoot too
  error = slope * a.hi() - sinh;
  sinres = a;
  sinres.calc(&slope, 0, &error, &range);
}

void sincos (const Affine& a, Affine* sinres, Affine* cosres)
{ //sine and cosine of a, using trig identities
  //XXX: should investigate when simply returning [-1..1] has better accuracy
  if ((width(a) / 2) >= IntervalH::pi().lo()) {
    if (sinres) *sinres = Affine(0, 1);
    if (cosres) *cosres = Affine(0, 1);
  } else {
    Real center = (a.lo() + a.hi()) / 2;
    Real sinc, cosc;
    mysincos(center, &sinc, &cosc);
    if (a.lo() == a.hi()) {
      if (sinres) *sinres = Affine(sinc);
      if (cosres) *cosres = Affine(cosc);
    } else {
      Interval old_range(a); //save in case &a == sinres or cosres
      Affine sincomp, coscomp;
      symsincos(a - center, sincomp, coscomp);

      if (sinres) {
        //sin(center + range) = sin(center)*cos(range) + cos(center)*sin(range)
        *sinres = sinc * coscomp + cosc * sincomp;
        Interval sin_range(sin(old_range));
        sinres->calc(0, 0, 0, &sin_range);
      }
      if (cosres) {
        //cos(center + range) = cos(center)*cos(range) - sin(center)*sin(range)
        *cosres = cosc * coscomp - sinc * sincomp;
        Interval cos_range(cos(old_range));
        cosres->calc(0, 0, 0, &cos_range);
      }
    }
  }
}

Affine& tanA (Affine& a)
{ //XXX: write a better version
  Interval range(tan(Interval(a)));
  {
    Affine cos;
    sincos(a, &a, &cos);
    a /= cos;
  }
  a.collapse();
  a.calc(0, 0, 0, &range);
  return a;
}

Affine& asinA (Affine& a)
{
  if ((a.lo() < -1.0) || (a.hi() > 1.0))
    throw Affine::DomainError();
  if (a.lo() == a.hi())
    a = std::asin(a.lo());
  else {
    Interval range = asin(Interval(a));
    Real slope = width(range) / width(a),
    u = std::sqrt(1.0 - 1.0 / (slope*slope)),
    f_0 = std::asin(u) - slope*u;
    if ((-u >= a.lo()) && (u <= a.hi()))
      a.calc(&slope, 0, &f_0, &range);
    else {
      if (u > a.hi())
        f_0 = -f_0;
      Real r_0 = range.lo() - slope*a.lo(),
      intercept = std::ldexp(r_0 + f_0, -1),
      error = std::ldexp(r_0 - f_0, -1);
      a.calc(&slope, &intercept, &error, &range);
    }
  }
  return a;
}

Affine& atanA (Affine& a)
{
  if (a.lo() == a.hi())
    a = std::atan(a.lo());
  else {
    Interval range = atan(Interval(a));
    Real slope = width(range) / width(a),
    u = std::sqrt(1.0 / slope - 1.0),
    f_0 = std::atan(u) - slope*u;
    if ((-u >= a.lo()) && (u <= a.hi()))
      a.calc(&slope, 0, &f_0, &range);
    else {
      if (u > a.hi())
        f_0 = -f_0;
      Real r_0 = range.lo() - slope*a.lo(),
      intercept = std::ldexp(r_0 + f_0, -1),
      error = std::ldexp(r_0 - f_0, -1);
      a.calc(&slope, &intercept, &error, &range);
    }
  }
  return a;
}

void solve_linear (const Affine& fx, Real& a, const Affine& x)
{
  DevList::const_iterator fx_pos = fx.get_devs().begin();
  DevList::const_iterator fx_end = fx.get_devs().end();
  DevList::const_iterator x_pos = x.get_devs().begin();
  DevList::const_iterator x_end = x.get_devs().end();

  while ((x_pos != x_end) && (x_pos->second == 0.0))
    ++x_pos; //find a non-zero deviation
  if (x_pos == x_end)
    throw Affine::DomainError(); //indeterminate result, probably user error

  while ((fx_pos != fx_end) && (fx_pos->first < x_pos->first))
    ++fx_pos; //find matching deviation in fx
  if ((fx_pos == fx_end) || (fx_pos->first != x_pos->first))
    a = 0.0; //deviation not found; assumed equal to 0
  else
    a = fx_pos->second / x_pos->second;
}

void solve_linear (const Affine& fxy, Real& a, const Affine& x,
    Real& b, const Affine& y)
{
  DevList::const_iterator fxy_pos = fxy.get_devs().begin();
  DevList::const_iterator fxy_end = fxy.get_devs().end();
  DevList::const_iterator x_pos = x.get_devs().begin();
  DevList::const_iterator x_end = x.get_devs().end();
  DevList::const_iterator y_pos = y.get_devs().begin();
  DevList::const_iterator y_end = y.get_devs().end();

  Real fxy_val[2] = {0.0, 0.0}, x_val[2] = {0.0, 0.0}, y_val[2] = {0.0, 0.0};
  unsigned int dev_index[2];

  //These two blocks could be generalized into a for loop, with some
  //efficiency loss and extra obfuscation
  while ((x_pos != x_end) && (x_pos->second == 0.0))   //
    ++x_pos;                                           //
  while ((y_pos != y_end) && (y_pos->second == 0.0))   //
    ++y_pos;                                           //
  if ((x_pos == x_end) || (y_pos == y_end))            //
    throw Affine::DomainError();                       //
  dev_index[0] = std::min(x_pos->first, y_pos->first); // Get first set of
  if (x_pos->first == dev_index[0])                    // coefficients
    x_val[0] = x_pos->second;                          //
  if (y_pos->first == dev_index[0])                    //
    y_val[0] = y_pos->second;                          //

  if (x_pos->first == dev_index[0]) {                    //
    do { ++x_pos; }                                      //
    while ((x_pos != x_end) && (x_pos->second == 0.0));  //
  }                                                      //
  if (y_pos->first == dev_index[0]) {                    //
    do { ++y_pos; }                                      //
    while ((y_pos != y_end) && (y_pos->second == 0.0));  //
  }                                                      //
  if ((x_pos == x_end) && (y_pos == y_end))              //
    throw Affine::DomainError();                         //
  if (y_pos == y_end) {                                  // Get second set of
    dev_index[1] = x_pos->first;                         // coefficients
    x_val[1] = x_pos->second;                            //
  } else if (x_pos == x_end) {                           //
    dev_index[1] = y_pos->first;                         //
    y_val[1] = y_pos->second;                            //
  } else {                                               //
    dev_index[1] = std::min(x_pos->first, y_pos->first); //
    if (x_pos->first == dev_index[1])                    //
      x_val[1] = x_pos->second;                          //
    if (y_pos->first == dev_index[1])                    //
      y_val[1] = y_pos->second;                          //
  }

  for (int i = 0; i < 2; ++i) { //find matching coefficients in fxy
    while ((fxy_pos != fxy_end) && (fxy_pos->first < dev_index[i]))
      ++fxy_pos;
    if ((fxy_pos != fxy_end) && (fxy_pos->first == dev_index[i]))
      fxy_val[i] = fxy_pos->second;
  }

  Real divisor = x_val[1] * y_val[0] - x_val[0] * y_val[1];
  if (divisor == 0.0)
    throw Affine::DomainError();
  a = (y_val[0] * fxy_val[1] - y_val[1] * fxy_val[0]) / divisor;
  b = (x_val[1] * fxy_val[0] - x_val[0] * fxy_val[1]) / divisor;
}

std::ostream& Affine::print (std::ostream& o) const
{
  if (o.iword(AffineH::Verbosity::fmt_index_) == 0)
  { //print devs if verbosity == yes
  	o << center_;
  	for (DevList::const_iterator i = devs_.begin();
  			i != devs_.end(); i++)
  		o << " + " << i->second << "(e" << i->first << ")";

  	o << " == ";
  }

	o << interval_;
	return o;
}
  
std::istream& operator>> (std::istream& i, Affine& a)
{
//Format is: <center, radius>
	Real center, radius;
	char scratch = 0;
	if (!(i >> scratch) || (scratch != '<') || !(i >> center >> scratch)
			|| (scratch != ',') || !(i >> radius >> scratch)
			|| (scratch != '>'))
	//rediculous use of early-out
		i.setstate(::std::ios_base::badbit);
	else
		a = Affine(center, radius);
	return i;
}
