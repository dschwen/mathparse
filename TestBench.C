///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include "SymbolicMath.h"
#include "SMFunction.h"
#include "SMHelpers.h"
#include "SMJITTypes.h"
#include "SMTransformSimplify.h"

#include <iostream>

int
main(int argc, char * argv[])
{
  SymbolicMath::Parser parser;

  SymbolicMath::Real c;
  auto c_var = std::make_shared<SymbolicMath::RealReferenceData>(c, "c");
  parser.registerValueProvider(c_var);

  // auto func = parser.parse("a := c*c; b := 5; sqrt(a+b)");
  // func.simplify();

  // auto func = parser.parse("(c + 2) / 1 - 0 / (c -2)");
  auto func = parser.parse("1 *c*2*3*sin(4)");
  std::cout << func.format() << '\n';

  auto simplify = SymbolicMath::Simplify(func);
  func.apply(simplify);

  std::cout << func.format() << '\n';
  std::cout << func.formatTree() << '\n';

  c = 2.0;
  std::cout << "c = " << c << "; Value = " << func.value() << '\n';

  // func.compile();
  //
  // auto diff = func.D(c_var);
  //
  // diff.simplify();
  // diff.compile();
}
