#ifndef _AFFINE_H
#define _AFFINE_H
#include <istream>
#include <ostream>
#include <affine/interval.h>
#include <affine/config.h>

class DevList;

class Affine
{
public:
	Affine (Real center = 0.0);
	Affine (Real center, Real width);
  Affine (const Interval&);
  Affine (const Affine&);
  ~Affine ();

  Affine& operator= (const Affine&);
  Affine& neg ();
  Affine& ldexp (int);
	Affine& operator+= (const Affine&);
	Affine& operator-= (const Affine&);
	Affine& operator*= (const Affine&);
	Affine& operator/= (const Affine&);

  Affine& operator= (Real);
	Affine& operator+= (Real);
	Affine& operator-= (Real);
	Affine& operator*= (Real);
	Affine& operator/= (Real);

	operator const Interval& () const;
	void calc (const Real*, const Real*, const Real*, const Interval*);
	Real radius () const; //half the width of the variable
	Real lo () const;
	Real hi () const;
	void collapse (); //condense independent deviations into one
  const DevList& get_devs () const; //don't use this

  std::ostream& print (std::ostream&) const;
	typedef Interval::DomainError DomainError;
private:
	Real center_;
	Interval interval_; //Interval approximation
	DevList& devs_; //Deviations
};

namespace AffineH //FIXME rename once everything is in a namespace
{
  class Verbosity //iostream manipulator class to choose output verbosity
  {
  private:
    Verbosity ();
    Verbosity (const Verbosity&);
    Verbosity& operator= (const Verbosity&);
    const bool setting_; // false: verbose. true: terse
    static const int fmt_index_; // index of our data in streams
  public:
    explicit Verbosity (bool s) : setting_(s) {}
    friend std::ostream& operator<< (std::ostream&, const Verbosity&);
    friend std::ostream& Affine::print (std::ostream&) const;
  };

  extern const Verbosity verbose; // cout << verbose; //sets verbosity = yes
  extern const Verbosity terse; // cout << terse; //sets verbosity = no
  
  std::ostream& operator<< (std::ostream&, const Verbosity&);
}

// Global function declarations ------------------------------------------------
Affine operator- (const Affine&);
Affine operator+ (const Affine&);
Affine operator+ (const Affine&, const Affine&);
Affine operator- (const Affine&, const Affine&);
Affine operator* (const Affine&, const Affine&);
Affine operator/ (const Affine&, const Affine&);
Affine operator+ (const Affine&, Real);
Affine operator- (const Affine&, Real);
Affine operator* (const Affine&, Real);
Affine operator/ (const Affine&, Real);
Affine operator+ (Real, const Affine&);
Affine operator- (Real, const Affine&);
Affine operator* (Real, const Affine&);
Affine operator/ (Real, const Affine&);

Affine ldexp (const Affine& a, int b); //a*(2^b)
Affine abs (const Affine&);
Affine inverse (const Affine&);
Affine sqr (const Affine&);
Affine pow (const Affine& base, int exponent);
Affine pow (const Affine& base, int numerator, unsigned int denominator);
Affine pow (const Affine& base, const Affine& exponent);
Affine root (const Affine& base, unsigned int root);
Affine sqrt (const Affine&);
Affine exp (const Affine&);
Affine log (const Affine&);
Affine sin (const Affine&);
Affine cos (const Affine&);
void sincos (const Affine& a, Affine* sinres, Affine* cosres);
//put sin(a) in sinres and cos(a) in cosres--better than separate calls
Affine tan (const Affine&);
Affine asin (const Affine&);
Affine acos (const Affine&);
Affine atan (const Affine&);

// *A functions modify their first parameter instead of copying it.
Affine& negA (Affine&); //XXX: redundant?
Affine& ldexpA (Affine&, int);
Affine& absA (Affine&);
Affine& inverseA (Affine&);
Affine& sqrA (Affine&);
Affine& powA (Affine& base, int exponent);
Affine& powA (Affine& base, unsigned int exponent);
Affine& powA (Affine& base, int numerator, unsigned int denominator);
Affine& powA (Affine& base, const Affine& exponent);
Affine& rootA (Affine& base, unsigned int root);
Affine& sqrtA (Affine&);
Affine& expA (Affine&);
Affine& logA (Affine&);
Affine& sinA (Affine&);
Affine& cosA (Affine&);
Affine& tanA (Affine&);
Affine& asinA (Affine&);
Affine& acosA (Affine&);
Affine& atanA (Affine&);

//Miscellaneous functions

Affine min (const Affine&, const Affine&);
Affine max (const Affine&, const Affine&);

void solve_linear (const Affine& fx, Real& a, const Affine& x);
//solve fx = a*x + ... for a

void solve_linear (const Affine& fxy, Real& a, const Affine& x,
    Real& b, const Affine& y);
//solve fxy = a*x + b*y + ... for a and b

void solve_linear (const Affine& fxyz, Real& a, const Affine& x,
    Real& b, const Affine& y, Real& c, const Affine& z);
//solve fxyz = a*x + b*y + c*z + ... for a, b, and c

Real width(const Affine&); //hi() - lo();

std::ostream& operator<< (std::ostream&, const Affine&);
std::istream& operator>> (std::istream&, Affine&);

#include <affine/affine_inline.h>
#endif //_AFFINE_H
