#ifndef _EXPRESSIONS_H
#define _EXPRESSIONS_H

#include <boost/rational.hpp>
#include "evaluator.h"

namespace evaluator {

struct variable : expression
{ //variable is a bit of a misnomer: only the main program is allowed to
  //"vary" it; the expression may not.
  variable (rstring symbol, const Affine* p)
    : expression("variable"), symbol_(symbol)
    { value = const_cast<Affine*>(p); }
  cstring symbol_;
};

struct immediate : expression
{
  immediate (const Affine& a) : expression("immediate"), val(a)
  { value = const_cast<Affine*>(&val); }
  const Affine val;
};

struct copy_holder : expression
{
  copy_holder (node original) : expression("copy")
  { const_child = original; value = &copy; }
  void reset () { expression::reset(); copy = Affine(); }
  //XXX: not sure if I need to reset original when (*this) is reset
  void evaluate ()
  { const_child->safe_evaluate(); copy = *const_child->value; }
  Affine copy;
};

struct unary_op : expression
{
  typedef Affine& (*unary_func) (Affine&);

  unary_op (rstring name, node arg, unary_func func)
    : expression("unary " + name), func_(func)
    { mod_child = arg; }
  void evaluate ()
  {
    mod_child->safe_evaluate();
    func_(*value);
  }

  unary_func func_;
};

struct binary_op : expression
{
  typedef Affine& (*binary_func) (Affine&, const Affine&);

  binary_op (rstring name, node left, node right, binary_func func)
    : expression("binary " + name), func_(func)
    { mod_child = left; const_child = right; }
  void evaluate ()
  {
    mod_child->safe_evaluate();
    const_child->safe_evaluate();
    func_(*value, *const_child->value);
  }

  binary_func func_;
};

template <Affine& (Affine::*f) ()>
  Affine& make_unary_func (Affine& x) { return (x.*f)(); }

template <Affine& (Affine::*f) (const Affine&)>
  Affine& make_binary_func (Affine& l, const Affine& r) { return (l.*f)(r); }

struct real : expression
{
  real (Real v) : expression("real"), val(v), aval(v)
  { value = const_cast<Affine*>(&aval); }
  Real val;
  const Affine aval;
};

struct rational : expression
{
  rational (int v) : expression("rational"), val(v), aval(Real(v))
  { value = const_cast<Affine*>(&aval); }
  boost::rational<int> val;
  const Affine aval;
};

} //namespace evaluator

#endif //_EXPRESSIONS_H
