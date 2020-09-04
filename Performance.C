///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include "SymbolicMath.h"
#include "SMFunction.h"
#include "SMHelpers.h"
#include "SMTransformSimplify.h"

#include "SMCompiledByteCode.h"
#include "SMCompiledCCode.h"
#include "SMCompiledSLJIT.h"
#include "SMCompiledLibJIT.h"
#include "SMCompiledLightning.h"
#ifdef SYMBOLICMATH_USE_LLVMIR
#include "SMCompiledLLVM.h"
#endif

#include "performance_expression.h"

#include <iostream>
#include <chrono>

template <class C>
void
test()
{
  SymbolicMath::Parser<SymbolicMath::Real> parser;

  SymbolicMath::Real c;
  auto c_var = std::make_shared<SymbolicMath::RealReferenceData<SymbolicMath::Real>>(c, "c");
  parser.registerValueProvider(c_var);

  SymbolicMath::Real T = 500.0;
  auto T_var = std::make_shared<SymbolicMath::RealReferenceData<SymbolicMath::Real>>(T, "y");
  parser.registerValueProvider(T_var);

  parser.registerConstant("kB", 8.6173324e-5);
  parser.registerConstant("T0", 410.0);

  auto func = parser.parse(expression);
  auto diff = func.D(c_var);
  SymbolicMath::Simplify<SymbolicMath::Real> simplify2(diff);
  C compiled(diff);

  // SymbolicMath::Simplify<SymbolicMath::Real> simplify(func);
  // C compiled(func);

  unsigned int n = 0;
  for (c = 0.01; c <= 0.99; c += 0.001)
    for (T = 200.0; T <= 800.0; T += 0.01)
      n++;

  // evaluate for various values of c and T
  double sum = 0.0;
  auto start = std::chrono::high_resolution_clock::now();
  for (c = 0.01; c <= 0.99; c += 0.001)
    for (T = 200.0; T <= 800.0; T += 0.01)
      sum += compiled();
  auto finish = std::chrono::high_resolution_clock::now();

  std::cout << sum << '\n';

  std::chrono::duration<double> elapsed = finish - start;
  std::cout << "Elapsed time: " << elapsed.count() << " s\n";
}

int
main(int argc, char * argv[])
{

  // test various compilers
  std::cout << "## SymbolicMath::Function...\n";
  test<SymbolicMath::Function<SymbolicMath::Real>>();
  std::cout << "\n## SymbolicMath::CompiledByteCode...\n";
  test<SymbolicMath::CompiledByteCode<SymbolicMath::Real>>();
  std::cout << "\n## SymbolicMath::CompiledCCode...\n";
  test<SymbolicMath::CompiledCCode<SymbolicMath::Real>>();
  std::cout << "\n## SymbolicMath::CompiledSLJIT...\n";
  test<SymbolicMath::CompiledSLJIT<SymbolicMath::Real>>();
  std::cout << "\n## SymbolicMath::CompiledLibJIT...\n";
  test<SymbolicMath::CompiledLibJIT<SymbolicMath::Real>>();
  std::cout << "\n## SymbolicMath::CompiledLightning...\n";
  test<SymbolicMath::CompiledLightning<SymbolicMath::Real>>();
#ifdef SYMBOLICMATH_USE_LLVMIR
  std::cout << "\n## SymbolicMath::CompiledLLVM...\n";
  test<SymbolicMath::CompiledLLVM<SymbolicMath::Real>>();
#endif

  return 0;
}
