#include "SymbolicMath.h"
#include <iostream>

int
main(int argc, char * argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " mathexpression\n\n";
    return 100;
  }

  SymbolicMath::Parser parser;
  auto a_var = parser.registerValueProvider("a");
  auto b_var = parser.registerValueProvider("b");
  auto c_var = parser.registerValueProvider("c");

  auto func = parser.parse(argv[1]);

  std::cout << func->format() << '\n'; // << func->formatTree("\t") << '\n';

  SymbolicMath::simplify(func);

  std::cout << " = " << func->format() << '\n'; // << func->formatTree("\t") << '\n';

  SymbolicMath::NodePtr deriv(func->D(a_var));

  std::cout << "D(F) = " << deriv->format() << '\n'; // << deriv->formatTree("\t") << '\n';

  SymbolicMath::simplify(deriv);

  std::cout << "D(F) = " << deriv->format() << '\n'; // << deriv->formatTree("\t") << '\n';

  return 0;
}
