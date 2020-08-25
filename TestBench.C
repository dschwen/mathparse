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

#include <iostream>

int
main(int argc, char * argv[])
{
  SymbolicMath::Parser<SymbolicMath::Real> parser;

  SymbolicMath::Real c;
  auto c_var = std::make_shared<SymbolicMath::RealReferenceData<SymbolicMath::Real>>(c, "c");
  parser.registerValueProvider(c_var);

  // auto func = parser.parse("a := c*c; b := 5; sqrt(a+b)");
  // func.simplify();

  // auto func = parser.parse("(c + 2) / 1 - 0 / (c -2)");
  auto func = parser.parse("1 *c*2*3*sin(4)");
  // auto func = parser.parse("sin(c/4)");
  std::cout << func.format() << '\n';

  {
    SymbolicMath::CompiledCCode<SymbolicMath::Real>::Source source(func);
    std::cout << '{' << source() << "}\n";
  }

  std::cout << func.formatTree() << '\n';

  SymbolicMath::Simplify<SymbolicMath::Real> simplify(func);

  std::cout << func.format() << '\n';
  std::cout << func.formatTree() << '\n';

  {
    SymbolicMath::CompiledCCode<SymbolicMath::Real>::Source source(func);
    std::cout << '{' << source() << "}\n";
  }

  c = 2.0;
  std::cout << "c = " << c << "; Value = " << func() << '\n';

  SymbolicMath::CompiledByteCode<SymbolicMath::Real> vm(func);
  std::cout << "vm value = " << vm() << '\n';

  SymbolicMath::CompiledCCode<SymbolicMath::Real> ccode(func);
  std::cout << "ccode value = " << ccode() << '\n';

  SymbolicMath::CompiledSLJIT<SymbolicMath::Real> sljit(func);
  std::cout << "sljit value = " << sljit() << '\n';

  SymbolicMath::CompiledLibJIT<SymbolicMath::Real> libjit(func);
  std::cout << "libjit value = " << libjit() << '\n';

  SymbolicMath::CompiledLightning<SymbolicMath::Real> lightning(func);
  std::cout << "lightning value = " << lightning() << '\n';

  // func.compile();
  //
  // auto diff = func.D(c_var);
  //
  // diff.simplify();
  // diff.compile();
}
