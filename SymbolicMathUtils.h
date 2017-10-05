#ifndef SYMBOLICMATHUTILS_H
#define SYMBOLICMATHUTILS_H

#include <string>
#include <cstdlib>
#include <iostream>

namespace SymbolicMath
{

typedef double Real;

_Noreturn void fatalError(const std::string & error);

std::string stringify(Real number);

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHUTILS_H
