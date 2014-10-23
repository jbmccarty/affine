#include <iostream>
#include <affine/affine.h>
#include "evaluator.h"

int main (int argc, char** argv)
{
  using namespace evaluator;
  const char* input = "(a * b + c) ^ d";
  if (argc > 1)
    input = argv[1];

  std::cout << AffineH::terse;

  var_map v;
  v["a"] = 1;
  v["b"] = 2;
  v["c"] = 3;
  v["d"] = 4;
  node e(make_evaluator(input, v));
  if (!e)
    return 1;

  Affine &a(v["a"]), &b(v["b"]), &c(v["c"]), &d(v["d"]);
  for (int i = 0; i < 20; ++i)
  {
    e->reset();
    std::cout << (*e)() << "\n";
    a += 1.0;
    b += 0.01;
    c -= 1.0;
    d -= 0.05;
  }
  return 0;
}

/* 
Need:
  while parsing:
    a handle class that can point to rational, real, and affine
  after parsing:
    collapse all rational and real expressions, and convert their affine
      parents into ones that store their non-affine argument themselves
    perform CSE
  while evaluating:
   */
