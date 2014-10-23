#ifndef _EVALUATOR_H
#define _EVALUATOR_H

#include <string>
#include <map>
#include <boost/shared_ptr.hpp>
#include <affine/affine.h>

namespace evaluator {
typedef const std::string& rstring;
typedef const std::string  cstring;
typedef std::map<std::string, Affine> var_map;
struct expression;
typedef boost::shared_ptr<expression> node;

struct expression
{
  expression (rstring name) : name_(name), is_evaluated(false), value(0) {}
  virtual ~expression () {};
  virtual void reset () {
    is_evaluated = false;
    if (mod_child)
      mod_child->reset();
    if (const_child)
      const_child->reset();
  }
  virtual void evaluate () {};
  void safe_evaluate ()
  {
    if (!is_evaluated) {
      evaluate();
      is_evaluated = true;
    }
  }
  const Affine& operator() ()
  {
    safe_evaluate();
    return *value;
  }

  cstring name_;
  bool is_evaluated;
  node mod_child; //modifiable child, if any
  node const_child; //unmodifiable, if any
  Affine* value; //set by optimizer
};

node make_evaluator (const char* input, const var_map& vars);

} //namespace evaluator

#endif //_EVALUATOR_H
