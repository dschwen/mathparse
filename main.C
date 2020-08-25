#include "SymbolicMath.h"
#include "SMFunction.h"
#include "SMHelpers.h"

#include <iostream>

int
main(int argc, char * argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " mathexpression\n\n";
    return 100;
  }

  SymbolicMath::Parser<SymbolicMath::Real> parser;

  auto a_var = SymbolicMath::symbol<SymbolicMath::Real>("a");
  parser.registerValueProvider(a_var);

  // auto b_var = parser.registerValueProvider("b");
  //
  // auto c_var = std::make_shared<SymbolicMath::SymbolData>("c");
  // parser.registerValueProvider(c_var);

  SymbolicMath::Real c;
  auto c_var = std::make_shared<SymbolicMath::RealReferenceData<SymbolicMath::Real>>(c, "c");
  parser.registerValueProvider(c_var);

  auto func = SymbolicMath::Function<SymbolicMath::Real>(parser.parse(argv[1]));
  std::cout << func.format() << '\n' << func.formatTree() << '\n';

  // func.simplify();
  std::cout << " = " << func.format() << '\n'; // << func->formatTree() << '\n';

  // evaluate for various values of c
  std::vector<SymbolicMath::Real> reference;
  for (c = -1.0; c <= 1.0; c += 0.3)
  {
    auto val = func();
    reference.push_back(val);
    std::cout << val << ' ';
  }
  std::cout << '\n';

  // func.compile();

  // evaluate for various values of c
  unsigned int index = 0;
  SymbolicMath::Real norm = 0;
  for (c = -1.0; c <= 1.0; c += 0.3)
  {
    auto val = func();
    std::cout << val << ' ';
    auto diff = reference[index++] - val;
    // auto diff = c + std::sin(1.1) - val;
    norm += std::abs(diff);
  }
  std::cout << '\n' << "norm = " << norm << '\n';

  auto deriv = func.D(c_var);
  std::cout << "D(F) = " << deriv.format() << '\n'; // << deriv->formatTree() << '\n';

  // deriv.simplify();
  std::cout << "D(F) = " << deriv.format() << '\n'; // << deriv->formatTree() << '\n';

  // evaluate for various values of c
  for (c = -1.0; c <= 1.0; c += 0.3)
    std::cout << deriv() << ' ';
  std::cout << '\n';

  // deriv.compile();

  // evaluate for various values of c
  for (c = -1.0; c <= 1.0; c += 0.3)
    std::cout << deriv() << ' ';
  std::cout << '\n';

  // finite differencing
  auto dc = 0.00001;
  for (c = -1.0; c <= 1.0; c += 0.3)
  {
    auto a = func();
    c += dc;
    auto b = func();
    c -= dc;
    std::cout << (b - a) / dc << ' ';
  }
  std::cout << '\n';

  return 0;
}
