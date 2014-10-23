#include "optimizer.h"
#include "expressions.h"

namespace optimizer {
  using namespace evaluator;

  inline bool is_writable (const node& expr)
  {
    return (expr.unique() && (expr->name_ != "variable")
        && (expr->name_ != "immediate") && (expr->name_ != "real")
        && (expr->name_ != "rational"));
  }

  inline void copy_on_write (node expr)
  {
    if (expr->mod_child) {
      if (!is_writable(expr->mod_child))
        if (((expr->name_ == "binary +") || (expr->name_ == "binary *"))
            //XXX: use properties for commutivity
            && is_writable(expr->const_child))
          //copy can be avoided by commuting
          expr->mod_child.swap(expr->const_child);
        else
          expr->mod_child.reset(new copy_holder(expr->mod_child));
      copy_on_write(expr->mod_child);
    }
    if (expr->const_child)
      copy_on_write(expr->const_child);
  }

  inline Affine* propagate_values (node expr)
  {
    if (expr->mod_child)
      expr->value = propagate_values(expr->mod_child);
    if (expr->const_child)
      propagate_values(expr->const_child);
    return expr->value;
  }

  void optimize (node expr)
  {
    //do tree rearrangement/simplification
    //  collapse immediate expressions
    //  store rational/real arguments in the function itself
    //  if values get promoted, store the promotion, not the original
    //  common subexpression elimination
    //  call collapse() at certain points?
    copy_on_write(expr);
    //delete unnecessary copies by checking evaluation order (the last
    //modifier doesn't have to copy unless mod_child is a var/immed)
    propagate_values(expr);
  }
}
