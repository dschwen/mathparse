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
  auto func = parser.parse(argv[1]);

  std::cout << func->format() << '\n';

  func->foldConstants();

  std::cout << " = " << func->format() << '\n';

  return 0;
}
