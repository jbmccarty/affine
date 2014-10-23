// Inlined member functions ----------------------------------------------------

inline Affine& Affine::operator+= (Real a)
{
  center_ += a;
  interval_ += a;
  return *this;
}

inline Affine& Affine::operator-= (Real a)
{
  center_ -= a;
  interval_ -= a;
  return *this;
}

inline Affine& Affine::operator/= (Real a)
{
  if (a == 0.0)
    throw DomainError();
  return *this *= 1.0 / a;
}

inline Affine::operator const Interval& () const
{ return interval_; }

inline Real Affine::lo () const
{ return interval_.lo(); }

inline Real Affine::hi () const
{ return interval_.hi(); }

// Inlined global functions ----------------------------------------------------

#define MAKEOP(op) \
inline Affine operator op (const Affine& a, const Affine& b) \
{ \
  Affine ret(a); \
  ret op##= b; \
  return ret; \
} \
\
inline Affine operator op (const Affine& a, Real b) \
{ \
  Affine ret(a); \
  ret op##= b; \
  return ret; \
}

#define MAKEOPCOM(op) \
inline Affine operator op (Real a, const Affine& b) \
{ \
  Affine ret(b); \
  ret op##= a; \
  return ret; \
}

#define MAKE1(name) \
inline Affine name (const Affine& a) \
{ \
  Affine ret(a); \
  name##A(ret); \
  return ret; \
}

#define MAKE2(name, type2) \
inline Affine name (const Affine& a, type2 b) \
{ \
  Affine ret(a); \
  name##A(ret, b); \
  return ret; \
}

inline Affine operator- (const Affine& a)
{
  Affine ret(a);
  negA(ret);
  return ret;
}

inline Affine operator+ (const Affine& a)
{ return a; }

inline Affine operator- (Real a, const Affine& b)
{
  Affine ret(b);
  negA(ret) += a;
  return ret;
}

inline Affine operator/ (Real a, const Affine& b)
{
  Affine ret(b);
  inverseA(ret) *= a;
  return ret;
}

MAKEOP(+)
MAKEOPCOM(+)
MAKEOP(-)
MAKEOP(*)
MAKEOPCOM(*)
MAKEOP(/)
MAKE1(abs)
MAKE1(inverse)
MAKE1(sqr)
MAKE2(ldexp, int)
MAKE2(pow, int)
inline Affine pow (const Affine& base, int numerator, unsigned int denominator)
{
  Affine ret(base);
  powA(ret, numerator, denominator);
  return ret;
}
MAKE2(pow, const Affine&)
MAKE2(root, unsigned int)
MAKE1(sqrt)
MAKE1(exp)
MAKE1(log)
MAKE1(sin)
MAKE1(cos)
MAKE1(tan)
MAKE1(asin)
MAKE1(acos)
MAKE1(atan)

inline Affine& negA (Affine& a)
{ return a.neg(); }

inline Affine& ldexpA (Affine& a, int b)
{ return a.ldexp(b); }

inline Affine& powA (Affine& base, int exponent)
{
  if (exponent < 0) {
    inverseA(base);
    exponent = -exponent;
  }
  return powA(base, static_cast<unsigned int> (exponent));
}

inline Affine& powA (Affine& base, const Affine& exponent)
{ return expA(logA(base) *= exponent); }

inline Affine& sinA (Affine& a)
{
  sincos(a, &a, 0);
  a.collapse();
  return a;
}

inline Affine& cosA (Affine& a)
{
  sincos(a, 0, &a);
  a.collapse();
  return a;
}

inline Affine& acosA (Affine& a)
{
  negA(asinA(a)) += ldexp(IntervalH::pi(), -1);
  a.collapse();
  return a;
}

inline Affine min (const Affine& a, const Affine& b)
{
  Affine res(a);
  absA(res -= b);
  (res -= a) -= b;
  ldexpA(negA(res), -1);
  return res;
}

inline Affine max (const Affine& a, const Affine& b)
{
  Affine res(a);
  absA(res -= b);
  (res += a) += b;
  ldexpA(res, -1);
  return res;
}

inline Real width (const Affine& a)
{ return width(Interval(a)); }

inline std::ostream& operator<< (std::ostream& o, const Affine& a)
{ return a.print(o); }

#undef MAKEOP
#undef MAKEOPCOM
#undef MAKE1
#undef MAKE2
