#include <iostream>
#include <set>
#include <vector>
#include <affine/affine.h>

// calculate product [lo .. hi]
inline Real product (unsigned lo, unsigned hi)
{
  Real p = 1.0;
  for (unsigned i = lo; i <= hi; ++i)
    p *= i;
  return p;
}

// calculate n choose r
inline Real nCr (unsigned n, unsigned r)
{
  return (n > 2*r) ?
    product(n - r + 1, n) / product(2, r) :
    product(r + 1, n) / product(2, n - r);
}

// Calculate integer powers of "x" as specified by "which", and return
// them in "results".
void powers (const Affine& x, const std::set<unsigned>& which,
    std::vector<Affine>& results)
{
  results.clear();
  results.reserve(which.size());
  if (which.count(0))
    results.push_back(1.0);
  if (which.count(1))
    results.push_back(x);

  if ((which.begin() != which.end()) && (*which.rbegin() > 1)) {{
    // approximate x by x0 + x1e1 and use a binomial expansion
    unsigned upper_bound = *which.rbegin(); // highest power
    std::vector<Real> x0_p(upper_bound + 1); // powers of x0
    x0_p[1] = 0.5 * (x.hi() + x.lo()); // this can be anything you want
    std::vector<Affine> x1e1_p(upper_bound + 1, x - x0_p[1]); // powers of x1e1
    for (unsigned i = 2; i <= upper_bound; ++i) {
      x0_p[i] = x0_p[1] * x0_p[i - 1];
      powA(x1e1_p[i], i);
    }
    for (std::set<unsigned>::iterator n = which.lower_bound(2);
        n != which.end(); ++n) {
      results.push_back(x0_p[*n]);
      for (unsigned r = 1; r < *n; ++r)
        results.back() += nCr(*n, r) * x0_p[*n - r] * x1e1_p[r];
      results.back() += x1e1_p[*n];
      Interval range(pow(Interval(x), *n));
      results.back().calc(0, 0, 0, &range);
    }
  }
  results.back().collapse();
  // only the last one has the possibility of being collapsed
  }
}

int main ()
{
  Affine x(3, 0.05);
  std::set<unsigned> which;
  which.insert(1);
  which.insert(5);
  which.insert(6);
  which.insert(25);
  std::vector<Affine> res;
  powers(x, which, res);

  int j = 0;
  for (std::set<unsigned>::iterator i = which.begin(); i != which.end();
      ++i, ++j)
    std::cout << "x^" << *i << "\t= " << res[j] << "\n\t= " << pow(x, *i)
      << "\n";
  std::cout << "x^6 - x^5 \t= " << res[2] - res[1] << "\n"
    << "\t\t= " << pow(x, 6) - pow(x, 5) << "\n"
    << "\t\t= " << pow(x, 5) * (x - 1.0) << "\n";
  return 0;
}
