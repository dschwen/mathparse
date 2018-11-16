#include "fparser.hh"

#include "performance_expression.h"

#include <iostream>
#include <chrono>

int
main(int argc, char * argv[])
{
  std::cout << "FParser\n";

  FunctionParser fparser;
  fparser.AddConstant("kB", 8.6173324e-5);
  fparser.AddConstant("T0", 410.0);
  fparser.Parse(expression, "c,y");

  // optimize
  fparser.Optimize();

  // if (!fparser.JITCompile())
  //   exit(1);

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

  return 0;
}
