#include <iostream>
#include <cstdio>
#include <cstdlib>

extern "C" {
#include <time.h>
}

#include <affine/affine.h>

int main ()
{
  int number, interval;
  std::cout << "Number of loops, collapse interval: ";
  std::cin >> number >> interval;

  Affine a(1.5, 0.4), b(3, 0.1);

  int i = number;
  clock_t start, end;
  start = clock();
  for (; i; --i)
  {
    //a -= (i & 1)? sin(a) : cos(a);
    (i & 1) ? logA(a) : expA(a);
    {
      Affine c(b);
      b += inverseA(sqrtA(c));
    }
    if (!(i % interval)) {
      a.collapse();
      b.collapse();
    }
  }
  end = clock();
  double secs = unsigned(end - start) / double(CLOCKS_PER_SEC);

  char mhz[15];
  std::FILE* proc_cpu
    = popen("egrep -i '^cpu.MHz' /proc/cpuinfo | cut -d: -f 2", "r");
  std::fgets(mhz, 15, proc_cpu);
  pclose(proc_cpu);
  
  std::cout << secs * std::strtod(mhz, 0) * 1000000 / number
    << " cycles per operation\n";
  return 0;
}
