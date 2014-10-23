// Inlined member functions ----------------------------------------------------

inline const Real& Interval::lo () const { return lower(); }

inline const Real& Interval::hi () const { return upper(); }

inline void IntervalException::operator() ()
{ throw Interval::DomainError(); }

// Inlined global functions ----------------------------------------------------

inline Interval inverse (const Interval& a)
{ return multiplicative_inverse(a); }

inline Interval ldexp (const Interval& a, int b)
{ return Interval(std::ldexp(a.lo(), b), std::ldexp(a.hi(), b)); }

inline bool check_periods (const Interval& I, const Interval& offset,
    const Interval& period)
{
  const Interval mod((I - offset) / period);
  Real lon = std::floor(mod.lo()), hin = std::floor(mod.hi());
  if ((lon != hin) || (lon == mod.lo()) || (hin == mod.hi()))
    return true; //I straddles an interval
  else
    return false;
}

inline std::istream& operator>> (std::istream& is, Interval& r)
{
  Real l, u;
  char c = 0;
  is >> c;
  if (c == '[') {
    is >> l >> c;
    if (c == ',')
      is >> u >> c;
    if (c != ']')
      is.setstate(is.failbit);
  } else {
    is.putback(c);
    is >> l;
    u = l;
  }
  if (is)
    r.assign(l, u);
  return is;
}

inline std::ostream& operator<< (std::ostream& o, const Interval& i)
{
  return o << '[' << i.lo() << ", " << i.hi() << ']';
}
