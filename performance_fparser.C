#include "fparser.hh"

#include "performance_expression.h"

#include <iostream>
#include <chrono>

void
bench(FunctionParser & fparser)
{
  double p[2];
  double & c = p[0];
  double & T = p[1];

  // evaluate for various values of c and T
  double sum = 0.0;
  auto start = std::chrono::high_resolution_clock::now();
  for (c = 0.01; c <= 0.99; c += 0.001)
    for (T = 200.0; T <= 800.0; T += 0.01)
      sum += fparser.Eval(p);
  auto finish = std::chrono::high_resolution_clock::now();

  std::cout << sum << '\n';

  std::chrono::duration<double> elapsed = finish - start;
  std::cout << "Elapsed time: " << elapsed.count() << " s\n";
}

int
main(int argc, char * argv[])
{
  std::cout << "\n# FParser\n";

  FunctionParser fparser;
  fparser.AddConstant("kB", 8.6173324e-5);
  fparser.AddConstant("T0", 410.0);
  fparser.Parse(expression, "c,y");

  std::cout << "\n## Bytecode\n";
  bench(fparser);

  // optimize
  fparser.Optimize();
  std::cout << "\n## Bytecode with Optimizer\n";
  bench(fparser);

  // if (!fparser.JITCompile())
  //   exit(1);

  return 0;
}
