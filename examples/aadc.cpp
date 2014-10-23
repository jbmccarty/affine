#include <iostream>
#include <utility>
#include <iomanip>
#include <deque>
#include <map>
#include <string>
#include <cstdlib>
#include <ctime>
#include <affine/affine.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#  define VERSION " " PACKAGE_VERSION REVISION " (Built " DATE ")"
#else
#  define VERSION ""
#endif

using std::map;
using std::string;
typedef Affine OperandType;
typedef std::deque<OperandType> OperandStack;
typedef std::pair<void (*) (OperandStack&), unsigned int> Operator;
//                function to call         number of args

typedef map<string, Operator> OperatorLUT; //operator look-up table
void init_operators (OperatorLUT&);
void call_operator (const OperatorLUT::const_iterator&, OperandStack&);

class except {};

inline void bad_input (const string& s)
{
  std::cerr << s;
  throw except();
}

int main (int argc, char** argv)
{
  using std::cin;
  using std::cerr;
  OperandStack operands;
  map<string, OperandType> variables;
  variables["pi"] = IntervalH::pi();
  variables["e"] = IntervalH::e();

  OperatorLUT operators;
  init_operators(operators);

  std::srand(std::time(0));

  string malformed("malformed input");

  if (argc <= 1 || strcmp(argv[1], "-q"))
    std::cout << "Aadc" VERSION << "\nCopyright 2002-2003 Jason McCarty\n";

  char firstchar = 0;
  while (cin >> firstchar)
  { try {
    cin.putback(firstchar);
    if (firstchar == '<')
    {
      operands.push_front(Affine());
      if (!(cin >> operands.front())) {
        operands.pop_front();
        bad_input(malformed);
      }
    }
    else if (firstchar == '[')
    {
      Interval temp;
      if (cin >> temp)
        operands.push_front(Affine(temp));
      else
        bad_input(malformed);
    }
    else if ((firstchar >= '0' && firstchar <= '9') || firstchar == '.'
        || firstchar == '_')
    {
      if (firstchar == '_')
        cin >> firstchar;
      double temp;
      if (cin >> temp)
        operands.push_front(Affine((firstchar == '_') ? -temp : temp));
      else
        bad_input(malformed);
    }
    else if (firstchar == '$') //pop into a variable
    {
      if (!operands.size())
        bad_input("stack underflow");
      char scratch;
      string temp;
      if(cin >> scratch >> temp)
      {
        variables[temp] = operands.front();
        operands.pop_front();
      }
      else
        bad_input(malformed);
    }
    else if (firstchar == '!') //push from a variable
    {
      char scratch;
      string temp;
      if(cin >> scratch >> temp)
      {
        map<string, OperandType>::const_iterator p
          = variables.find(temp);
        if (p != variables.end())
          operands.push_front(variables[temp]);
        else {
          cerr << temp;
          bad_input(" is not an initialized variable");
        }
      }
    }
    else if (firstchar == '#') //delete a variable
    {
      char scratch;
      string temp;
      if(cin >> scratch >> temp)
      {
        map<string, OperandType>::iterator p
          = variables.find(temp);
        if (p != variables.end())
          variables.erase(p);
        else {
          cerr << temp;
          bad_input(" is not an initialized variable");
        }
      }
    }
    else
    {
      string temp;
      if (cin >> temp)
      {
        const OperatorLUT::const_iterator p = operators.find(temp);
        if (p == operators.end()) {
          cerr << temp;
          bad_input(" is not a valid operator");
        }
        else {
          try {
            call_operator(p, operands);
          }
          catch (Affine::DomainError)
          {
            cerr << temp;
            bad_input(": domain error");
          }
        }
      }
    }
  }
  catch (except)
  {
    if (cin.eof())
      cerr << ". EOF received\n";
    else {
        string temp("");
        cin.clear();
        std::getline(cin, temp);
        cerr << ". Location: ";
        if (temp.size())
          cerr << "at or before `" << temp;
        else
          cerr << "at or after `" << firstchar;
        cerr << "'. Remainder of line discarded.\n";
      }
    }
  }
  return 0;
}

void collapse (OperandStack& operands)
{ operands.front().collapse(); }

void clear (OperandStack& operands)
{ operands.clear(); }

void dup (OperandStack& operands)
{ operands.push_front(operands.front()); }

void printall (OperandStack& operands)
{
  for (OperandStack::const_iterator i = operands.begin();
      i != operands.end(); ++i)
    std::cout << *i << "\n";
}

void printtop (OperandStack& operands)
{ std::cout << operands.front() << "\n"; }

void quit (OperandStack&)
{ std::exit(0); }

void pushdepth (OperandStack& operands)
{ operands.push_front(OperandType(operands.size())); }

void setterse (OperandStack&)
{ std::cout << AffineH::terse; }

void setverbose (OperandStack&)
{ std::cout << AffineH::verbose; }

void abs (OperandStack& operands)
{ absA(operands.front()); }

void cos (OperandStack& operands)
{ cosA(operands.front()); }

void asin (OperandStack& operands)
{ asinA(operands.front()); }

void acos (OperandStack& operands)
{ acosA(operands.front()); }

void atan (OperandStack& operands)
{ atanA(operands.front()); }

void exp (OperandStack& operands)
{ expA(operands.front()); }

void setprecision (OperandStack& operands)
{
  std::cout << std::setprecision(static_cast<int> (operands.front().lo()));
  operands.pop_front();
}

void log (OperandStack& operands)
{ logA(operands.front()); }

void pop (OperandStack& operands)
{ operands.pop_front(); }

void rand (OperandStack& operands)
{
  Affine& f = operands.front();
  f = Affine(f.lo() + std::rand() * width(f) / RAND_MAX);
}

void sin (OperandStack& operands)
{ sinA(operands.front()); }

void sincos (OperandStack& operands)
{
  operands.push_front(Affine());
  sincos(operands[1], &operands[1], &operands.front());
}

void sqr (OperandStack& operands)
{ sqrA(operands.front()); }

void sqrt (OperandStack& operands)
{ sqrtA(operands.front()); }

void tan (OperandStack& operands)
{ tanA(operands.front()); }

void add (OperandStack& operands)
{
  operands[1] += operands.front();
  operands.pop_front();
} 

void sub (OperandStack& operands)
{
  operands[1] -= operands.front();
  operands.pop_front();
} 

void mul (OperandStack& operands)
{
  operands[1] *= operands.front();
  operands.pop_front();
} 

void div (OperandStack& operands)
{
  operands[1] /= operands.front();
  operands.pop_front();
} 

void pow (OperandStack& operands)
{
  powA(operands[1], operands.front());
  operands.pop_front();
} 

void swap (OperandStack& operands)
{
  Affine temp(operands[1]);
  operands[1] = operands.front();
  operands.front() = temp;
}

void affine (OperandStack& operands)
{
  operands[1] = Affine(operands[1].lo(), operands.front().lo());
  operands.pop_front();
}

void call_operator (const OperatorLUT::const_iterator& function,
    OperandStack& operands)
{
  if (operands.size() < function->second.second)
    bad_input(function->first + ": stack underflow");
  else
    function->second.first(operands);
}

inline Operator make_pair (void (*f) (OperandStack&), unsigned int n)
{ return std::make_pair<void (*) (OperandStack&), unsigned int>(f, n); }

void init_operators (OperatorLUT& operators)
{
  //operators which take 0 operands
  operators["c"]        = make_pair(clear, 0);
  operators["collapse"] = make_pair(collapse, 0);
  operators["f"]        = make_pair(printall, 0);
  operators["q"]        = make_pair(quit, 0);
  operators["verbose"]  = make_pair(setverbose, 0);
  operators["terse"]    = make_pair(setterse, 0);
  operators["z"]        = make_pair(pushdepth, 0);

  //operators which take 1 operand
  operators["abs"]    = make_pair(abs, 1);
  operators["acos"]   = make_pair(acos, 1);
  operators["asin"]   = make_pair(asin, 1);
  operators["atan"]   = make_pair(atan, 1);
  operators["cos"]    = make_pair(cos, 1);
  operators["d"]      = make_pair(dup, 1);
  operators["exp"]    = make_pair(exp, 1);
  operators["k"]      = make_pair(setprecision, 1);
  operators["log"]    = make_pair(log, 1);
  operators["p"]      = make_pair(printtop, 1);
  operators["pop"]    = make_pair(pop, 1);
  operators["rand"]   = make_pair(rand, 1);
  operators["sin"]    = make_pair(sin, 1);
  operators["sincos"] = make_pair(sincos, 1);
  operators["sqr"]    = make_pair(sqr, 1);
  operators["sqrt"]   = make_pair(sqrt, 1);
  operators["tan"]    = make_pair(tan, 1);
  operators["v"]      = make_pair(sqrt, 1);

  //operators which take 2 operands
  operators["+"] = make_pair(add, 2);
  operators["-"] = make_pair(sub, 2);
  operators["*"] = make_pair(mul, 2);
  operators["/"] = make_pair(div, 2);
  operators["^"] = make_pair(pow, 2);
  operators["r"] = make_pair(swap, 2);
  operators["affine"] = make_pair(affine, 2); //make an affine form from top two
}
