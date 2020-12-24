///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include "SymbolicMath.h"
#include "SMFunction.h"
#include "SMTransformSimplify.h"
#include "SMHelpers.h"

#include <iostream>

#include "SMCompilerFactory.h"

#include "performance_expression.h"

int
main(int argc, char * argv[])
{
  // check the factory system
  auto compilers = SymbolicMath::CompilerFactory<SymbolicMath::Real>::listCompilers();
  std::cout << "Registerd compilers:";
  for (auto & c : compilers)
    std::cout << ' ' << c;
  std::cout << '\n';

  std::cout << "Best compiler:" << SymbolicMath::CompilerFactory<SymbolicMath::Real>::bestCompiler()
            << '\n';

  // expression building
  {
    using RealNode = SymbolicMath::Node<SymbolicMath::Real>;
    auto n1 = RealNode::fromReal(0.0) + RealNode::fromReal(1.0);
    std::cout << "n1 = " << n1.format() << '\n';
    auto n2 = n1 + RealNode::fromReal(2.0);
    std::cout << "n2 = " << n2.format() << '\n';

    // simplify n1
    SymbolicMath::Simplify<SymbolicMath::Real> simplify(n1);

    std::cout << "n1 = " << n1.format() << '\n';
    std::cout << "n2 = " << n2.format() << '\n';
  }

  // parse example expression
  SymbolicMath::Parser<SymbolicMath::Real> parser;

  SymbolicMath::Real c = 2.0, T;
  auto c_var = std::make_shared<SymbolicMath::RealReferenceData<SymbolicMath::Real>>(c, "c");
  auto T_var = std::make_shared<SymbolicMath::RealReferenceData<SymbolicMath::Real>>(T, "y");
  parser.registerValueProvider(c_var);
  parser.registerValueProvider(T_var);

  parser.registerConstant("kB", 8.6173324e-5);
  parser.registerConstant("T0", 410.0);

  auto func = parser.parse("log10(c)");
  std::cout << func.format() << '\n';

  std::cout << func.formatTree() << '\n';

  SymbolicMath::Simplify<SymbolicMath::Real> simplify(func);

  std::cout << func.format() << '\n';
  std::cout << func.formatTree() << '\n';

  auto diff = func.D(c_var);

  std::cout << diff.format() << '\n';
  std::cout << diff.formatTree() << '\n';

  auto best_comp = SymbolicMath::CompilerFactory<SymbolicMath::Real>::buildBestCompiler(func);
  std::cout << "[best compiler] = " << (*best_comp)() << '\n';
}
