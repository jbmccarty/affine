#ifndef _PARSER_H
#define _PARSER_H

#include <vector>
#include <deque> //std::stack isn't usable, as boost's append() doesn't work
#include <map>
#include <boost/spirit/core.hpp>
#include <boost/spirit/symbols/symbols.hpp>
#include "expressions.h"
#include "optimizer.h"

namespace expr_parser {
using namespace boost::spirit;
using evaluator::node;
using evaluator::var_map;

class expr_parser : public grammar<expr_parser>
{
public:
  expr_parser (node& expr, const var_map& vars)
    : expr_(expr), vars_(vars)
  { init_funcs(); }

  template <typename ScannerT>
  struct definition
  {
    typedef definition<ScannerT>* selftype;
    struct functor
    {
      functor (selftype d) : this_(d) {}
      selftype this_;
    };

    struct return_expr : functor
    {
      return_expr (selftype d) : functor(d) {}
      void operator() (const char*, const char*) const
      {
        functor::this_->self_.expr_ = functor::this_->expr_stack.back();
      }
    };

    struct push_assign : functor
    {
      push_assign (selftype d) : functor(d) {}
      void operator() (const char*, const char*) const
      {
        functor::this_->vars[functor::this_->sym_stack.back()] =
          functor::this_->expr_stack.back();
        functor::this_->sym_stack.pop_back();
        functor::this_->expr_stack.pop_back();
      }
    };

    struct push_op : functor
    {
      push_op (selftype d, char sym) : functor(d), sym_(sym) {}
      void operator() (const char*, const char*) const
      {
        node e;
        node right = functor::this_->expr_stack.back();
        functor::this_->expr_stack.pop_back();
        switch(sym_) {
          case '=': case '+': case '-': case '*': case '/': case '^':
            {
              node left = functor::this_->expr_stack.back();
              functor::this_->expr_stack.pop_back();
              e.reset(new
                evaluator::binary_op(std::string(&sym_, 1), left, right,
                  functor::this_->self_.binary_funcs.find(
                    std::string(&sym_, 1))->second));
            }
            break;
          case '_':
            e.reset(new evaluator::unary_op("neg", right,
                  functor::this_->self_.unary_funcs.find("neg")->second));
            break;
        }
        functor::this_->expr_stack.push_back(e);
      }
      char sym_;
    };

    struct push_real : functor
    {
      push_real (selftype d) : functor(d) {}
      void operator() (Real val) const
      {
        node e;
        int int_val = static_cast<int>(val);
        if (int_val == val)
          e.reset(new evaluator::rational(int_val));
        else
          e.reset(new evaluator::real(val));
        functor::this_->expr_stack.push_back(e);
      }
    };

    struct push_affine : functor
    {
      push_affine (selftype d) : functor(d) {}
      void operator() (const char*, const char*) const
      {
        node e(new evaluator::immediate(
              Affine(functor::this_->reals[0], functor::this_->reals[1])));
        functor::this_->expr_stack.push_back(e);
        functor::this_->reals.clear();
      }
    };

    struct new_arg_count : functor
    {
      new_arg_count (selftype d) : functor(d) {}
      void operator() (const char*, const char*) const
      { functor::this_->arg_count.push_back(0); }
    };
    
    struct inc_arg_count : functor
    {
      inc_arg_count (selftype d) : functor(d) {}
      void operator() (const char*, const char*) const
      { ++functor::this_->arg_count.back(); }
    };

    struct push_func : functor
    {
      push_func (selftype d) : functor(d) {}
      void operator() (const char*, const char*) const
      {
        const std::string& sym = functor::this_->sym_stack.back();
        node e;
        switch (functor::this_->arg_count.back()) {
          case 0:
            {
              /*zero_ary_func_map::const_iterator i
                = functor::this_->self.zero_ary_funcs.find(sym);
              if (i != functor::this_->self.zero_ary_funcs.end())
                e.reset(new evaluator::zero_ary_op(sym, i->second));
              else*/ {
                std::map<std::string, node>::const_iterator i
                  = functor::this_->vars.find(sym);
                if (i != functor::this_->vars.end())
                  e = i->second;
                else
                  throw(int(3));
              }
            }
            break;
          case 1:
            {
              node arg = functor::this_->expr_stack.back();
              functor::this_->expr_stack.pop_back();
              unary_func_map::const_iterator i
                = functor::this_->self_.unary_funcs.find(sym);
              if (i != functor::this_->self_.unary_funcs.end())
                e.reset(new evaluator::unary_op(sym, arg, i->second));
              else
                throw int(4);
            }
            break;
          case 2:
            {
              node right = functor::this_->expr_stack.back();
              functor::this_->expr_stack.pop_back();
              node left = functor::this_->expr_stack.back();
              functor::this_->expr_stack.pop_back();
              binary_func_map::const_iterator i
                = functor::this_->self_.binary_funcs.find(sym);
              if (i != functor::this_->self_.binary_funcs.end())
                e.reset(new evaluator::binary_op(sym, left, right, i->second));
              else
                throw int(5);
            }
            break;
          default:
            throw int(6);
        }
        functor::this_->expr_stack.push_back(e);
        functor::this_->sym_stack.pop_back();
        functor::this_->arg_count.pop_back();
      }
    };

    definition(const expr_parser& self) : self_(self)
    {
      init_vars();

      input      = (*assignment >> equality)[return_expr(this)] ;
      assignment = (identifier >> ':' >> expression >> ';')[push_assign(this)] ;
      equality   = expression >> !('=' >> expression)[push_op(this, '=')] ;
      identifier = lexeme_d[alpha_p >> *(alnum_p | '_')][append(sym_stack)] ;
      expression = term >> *(
                   ('+' >> term)[push_op(this, '+')]
                 | ('-' >> term)[push_op(this, '-')]
                 );
      term       = factor >> *(
                   ('*' >> factor)[push_op(this, '*')]
                 | ('/' >> factor)[push_op(this, '/')]
                 );
      factor     = (atom >> !('^' >> factor)[push_op(this, '^')])
                 | ('-' >> factor)[push_op(this, '_')] ;
      atom       = number | function | '(' >> expression >> ')' ;
      number     = real_p[push_real(this)] | affine ;
      affine     = ('<' >> real_p[append(reals)] >> ','
                >> real_p[append(reals)] >> '>')[push_affine(this)] ;
      function   = (identifier[new_arg_count(this)]
                   >> !( '(' >> expression[inc_arg_count(this)]
                   >> *( ',' >> expression[inc_arg_count(this)])
                   >> ')' ))[push_func(this)] ;
    }

    void init_vars ()
    {
      for (var_map::const_iterator i = self_.vars_.begin();
          i != self_.vars_.end(); ++i) {
        node e(new evaluator::variable(i->first, &(i->second)));
        vars[i->first] = e;
      }
    }

    rule<ScannerT> input, assignment, equality, identifier, expression,
      term, factor, atom, number, affine, function;
    const rule<ScannerT>& start () const { return input; }
    std::map<std::string, node> vars;
    std::deque<node> expr_stack;
    std::deque<std::string> sym_stack;
    std::deque<int> arg_count;
    std::vector<double> reals;
    const expr_parser& self_;
  };

  void init_funcs () {
    unary_funcs["neg"] = evaluator::make_unary_func<&Affine::neg>;
#define DECLARE1(x) unary_funcs[#x] = x##A;
    DECLARE1(abs)
    DECLARE1(inverse)
    DECLARE1(sqr)
    DECLARE1(sqrt)
    DECLARE1(exp)
    DECLARE1(log)
    DECLARE1(sin)
    DECLARE1(cos)
    DECLARE1(tan)
    DECLARE1(asin)
    DECLARE1(acos)
    DECLARE1(atan)
    //more...
#undef DECLARE1
    
    binary_funcs["^"] = powA;
#define DECLARE2(x) binary_funcs[#x] = \
      evaluator::make_binary_func<&Affine::operator x##= >;
    DECLARE2(+)
    DECLARE2(-)
    DECLARE2(*)
    DECLARE2(/)
#undef DECLARE2
    binary_funcs["="] = binary_funcs["-"];
  }

  node& expr_;
  const var_map& vars_;

  typedef std::map<std::string, Affine& (*) ()> zero_ary_func_map;
  typedef std::map<std::string, Affine& (*) (Affine&)> unary_func_map;
  typedef std::map<std::string, Affine& (*) (Affine&, const Affine&)>
    binary_func_map;
  zero_ary_func_map zero_ary_funcs;
  unary_func_map unary_funcs;
  binary_func_map binary_funcs;
};
} // namespace expr_parser

namespace evaluator {

node make_evaluator (const char* input, const var_map& vars)
{
  using namespace boost::spirit;
  node ret;
  if (parse(input, expr_parser::expr_parser(ret, vars), space_p).full)
  {
    optimizer::optimize(ret);
  } else
    throw(17);
  return ret;
}
} // namespace evaluator

#endif // _PARSER_H
