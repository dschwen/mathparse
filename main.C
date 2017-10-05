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
  parser.registerValueProvider("a");
  parser.registerValueProvider("b");
  parser.registerValueProvider("c");

  auto func = parser.parse(argv[1]);

  std::cout << func->format() << '\n' << func->formatTree("\t") << '\n';

  SymbolicMath::simplify(func);

  std::cout << " = " << func->format() << '\n' << func->formatTree("\t") << '\n';

  return 0;
}
