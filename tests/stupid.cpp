#include <iostream>
#include <affine/affine.h>

//This program exposes a memory problem with the old list implementation.

int main ()
{
  int times = 0;
  std::cin >> times;
  while (times--)
  {
  //[-0.390625, -0.351562] $x [-0.741172, -0.73418] $y !x d sqr * sincos !y sqr sin log 3 !x sin * !y cos * - * + p #x #y c
    Affine x(-0.8, 0.2), y(-0.7377, 0.0035);
    std::cout << (sin(sqr(sqr(x))) + cos(x) + sin(x) + sin(y) + sin(x) + cos(y))
      << "\n";
  }
  return 0;
}
