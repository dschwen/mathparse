///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include "SymbolicMath.h"
#include "SMFunction.h"
#include "SMTransformSimplify.h"
#include "SMHelpers.h"

#include "SMCSourceGenerator.h"
#include "SMCompiledCCode.h"

#include "SMCompiledByteCode.h"
#include "SMCompiledCCode.h"
#include "SMCompiledSLJIT.h"
#include "SMCompiledLibJIT.h"
#include "SMCompiledLightning.h"
#ifdef SYMBOLICMATH_USE_LLVMIR
#include "SMCompiledLLVM.h"
#endif
#include <iostream>

// #include "performance_expression.h"

int
main(int argc, char * argv[])
{
  SymbolicMath::Parser<SymbolicMath::Real> parser;

  SymbolicMath::Real c, T;
  auto c_var = std::make_shared<SymbolicMath::RealReferenceData<SymbolicMath::Real>>(c, "c");
  auto T_var = std::make_shared<SymbolicMath::RealReferenceData<SymbolicMath::Real>>(T, "y");
  parser.registerValueProvider(c_var);
  parser.registerValueProvider(T_var);

  parser.registerConstant("kB", 8.6173324e-5);
  parser.registerConstant("T0", 410.0);

  // auto func = parser.parse("a := c*c; b := 5; sqrt(a+b)");
  // func.simplify();

  // auto func = parser.parse("(c + 2) / 1 - 0 / (c -2)");
  auto func = parser.parse("if(c<-0.5, 10, if(c>0.2, 20, 30))");
  // auto func = parser.parse(expression);
  std::cout << func.format() << '\n';

  {
    SymbolicMath::CSourceGenerator<SymbolicMath::Real> source(func);
    std::cout << '{' << source() << "}\n";
  }

  std::cout << func.formatTree() << '\n';

  SymbolicMath::Simplify<SymbolicMath::Real> simplify(func);

  std::cout << func.format() << '\n';
  std::cout << func.formatTree() << '\n';

  {
    SymbolicMath::CSourceGenerator<SymbolicMath::Real> source(func);
    std::cout << '{' << source() << "}\n";
  }

  c = -1;
  T = 300.0;
  std::cout << "c = " << c << "; Value = " << func() << '\n';

  SymbolicMath::CompiledByteCode<SymbolicMath::Real> vm(func);
  vm.print();
  std::cout << "vm value = " << vm() << '\n';

  SymbolicMath::CompiledCCode<SymbolicMath::Real> ccode(func);
  std::cout << "ccode value = " << ccode() << '\n';

  SymbolicMath::CompiledSLJIT<SymbolicMath::Real> sljit(func);
  std::cout << "sljit value = " << sljit() << '\n';

  SymbolicMath::CompiledLibJIT<SymbolicMath::Real> libjit(func);
  std::cout << "libjit value = " << libjit() << '\n';

  SymbolicMath::CompiledLightning<SymbolicMath::Real> lightning(func);
  std::cout << "lightning value = " << lightning() << '\n';

#ifdef SYMBOLICMATH_USE_LLVMIR
  SymbolicMath::CompiledLLVM<SymbolicMath::Real> llvm(func);
  std::cout << "llvm value = " << llvm() << '\n';
#endif

  // func.compile();
  //
  // auto diff = func.D(c_var);
  //
  // diff.simplify();
  // diff.compile();
}
