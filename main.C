#include "SymbolicMath.h"
#include "SymbolicMathFunction.h"
#include "SymbolicMathHelpers.h"
#include "SymbolicMathJITTypes.h"

#include <iostream>

int
main(int argc, char * argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " mathexpression\n\n";
    return 100;
  }

  std::cout << "SymbolicMath with backend " << SymbolicMath::jit_backend_name << '\n';
  SymbolicMath::Parser parser;

  auto a_var = SymbolicMath::symbol("a");
  parser.registerValueProvider(a_var);

  // auto b_var = parser.registerValueProvider("b");
  //
  // auto c_var = std::make_shared<SymbolicMath::SymbolData>("c");
  // parser.registerValueProvider(c_var);

  SymbolicMath::Real c;
  auto c_var = std::make_shared<SymbolicMath::RealReferenceData>(c, "c");
  parser.registerValueProvider(c_var);

  auto func = SymbolicMath::Function(parser.parse(argv[1]));
  std::cout << func.format() << '\n' << func.formatTree("\t") << '\n';

  func.simplify();
  std::cout << " = " << func.format() << '\n'; // << func->formatTree("\t") << '\n';

  // evaluate for various values of c
  for (c = -1.0; c <= 1.0; c += 0.3)
    std::cout << func.value() << ' ';
  std::cout << '\n';

  func.compile();

  // evaluate for various values of c
  for (c = -1.0; c <= 1.0; c += 0.3)
    std::cout << func.value() << ' ';
  std::cout << '\n';

  auto deriv = func.D(c_var);
  std::cout << "D(F) = " << deriv.format() << '\n'; // << deriv->formatTree("\t") << '\n';

  deriv.simplify();
  std::cout << "D(F) = " << deriv.format() << '\n'; // << deriv->formatTree("\t") << '\n';

  // evaluate for various values of c
  for (c = -1.0; c <= 1.0; c += 0.3)
    std::cout << deriv.value() << ' ';
  std::cout << '\n';

  deriv.compile();

  // evaluate for various values of c
  for (c = -1.0; c <= 1.0; c += 0.3)
    std::cout << deriv.value() << ' ';
  std::cout << '\n';

  // finite differencing
  auto dc = 0.00001;
  for (c = -1.0; c <= 1.0; c += 0.3)
  {
    auto a = func.value();
    c += dc;
    auto b = func.value();
    c -= dc;
    std::cout << (b - a) / dc << ' ';
  }
  std::cout << '\n';

  return 0;
}
