#ifndef SYMBOLICMATH_H
#define SYMBOLICMATH_H

#include "SymbolicMathParser.h"

#include <string>

class SymbolicMath : public SymbolicMathParser
{
public:
  SymbolicMath(const std::string expression);

protected:
  std::string _expression;
};

#endif // SYMBOLICMATH_H
