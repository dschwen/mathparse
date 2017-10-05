#ifndef SYMBOLICMATHUTILS_H
#define SYMBOLICMATHUTILS_H

#include <string>
#include <cstdlib>
#include <iostream>

namespace SymbolicMath
{

// replace this with mooseError in the future
_Noreturn void
fatalError(const std::string & error)
{
  std::cerr << error << '\n';
  std::exit(1);
}

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHUTILS_H
