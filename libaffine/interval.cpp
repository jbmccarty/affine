#include <cmath>
#include <affine/interval.h>
#include "util.h"

Interval MakeConstant (const unsigned long* digits, int shift)
{
  Interval ret(0.0);
  const int precision = 64; //Result is correct up to this many bits.
  //Increase this if Real is more precise than long double.

  for (int i = (precision + 10) / 24; i >= 0; --i)
  //use 10 extra bits to be sure it will round up
    ret += Interval(std::ldexp(Real(digits[i]), shift - 24 * i));
  return ret;
}

const Interval& IntervalH::pi ()
{
  //Values taken from http://www.jjj.de/hfloat/hfloatpage.html
  static const unsigned long pidigits[] = { //high to low
    0xC90FDA, 0xA22168, 0xC234C4, 0xC6628B, 0x80DC1C, 0xD12902, 0x4E088A,
    0x67CC74, 0x020BBE, 0xA63B13};
  static const Interval pi = MakeConstant(pidigits, -22);
  return pi;
}

const Interval& IntervalH::e ()
{
  static const unsigned long edigits[] = { //high to low
    0xADF854, 0x58A2BB, 0x4A9AAF, 0xDC5620, 0x273D3C, 0xF1D8B9, 0xC583CE,
    0x2D3695, 0xA9E136, 0x411464};
  static const Interval e = MakeConstant(edigits, -22);
  return e;
}

Interval root (const Interval& base, unsigned int r)
{ //XXX: Doesn't fit in well with boost, no rounding
  if (!(r & 1) && (base.lo() < 0.0))
    throw Interval::DomainError();
  return Interval(root(base.lo(), r), root(base.hi(), r));
}
