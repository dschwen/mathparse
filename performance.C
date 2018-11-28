#include "SymbolicMath.h"
#include "SymbolicMathFunction.h"
#include "SymbolicMathHelpers.h"
#include "SymbolicMathJITTypes.h"

#include "performance_expression.h"

#include <iostream>
#include <chrono>

int
main(int argc, char * argv[])
{
  std::cout << "\n## with backend " << SymbolicMath::jit_backend_name << '\n';
  SymbolicMath::Parser parser;

  SymbolicMath::Real c;
  auto c_var = std::make_shared<SymbolicMath::RealReferenceData>(c, "c");
  parser.registerValueProvider(c_var);

  SymbolicMath::Real T = 500.0;
  auto T_var = std::make_shared<SymbolicMath::RealReferenceData>(T, "y");
  parser.registerValueProvider(T_var);

  parser.registerConstant("kB", 8.6173324e-5);
  parser.registerConstant("T0", 410.0);

  auto func = parser.parse(expression);
  // std::cout << func.format() << '\n';

  func.simplify();
  // std::cout << " = " << func.format() << '\n'
  //           << func->formatTree("\t") << '\n';

  func.compile();

  unsigned int n = 0;
  for (c = 0.01; c <= 0.99; c += 0.001)
    for (T = 200.0; T <= 800.0; T += 0.01)
      n++;

  // evaluate for various values of c and T
  double sum = 0.0;
  auto start = std::chrono::high_resolution_clock::now();
  for (c = 0.01; c <= 0.99; c += 0.001)
    for (T = 200.0; T <= 800.0; T += 0.01)
      sum += func.value();
  auto finish = std::chrono::high_resolution_clock::now();

  std::cout << sum << '\n';

  std::chrono::duration<double> elapsed = finish - start;
  std::cout << "Elapsed time: " << elapsed.count() << " s\n";

  return 0;
}
