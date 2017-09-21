#include "SymbolicMath.h"

#include <vector>
#include <iostream>
#include <cmath>

typedef double Real;

SymbolicMath::SymbolicMath(const std::string expression)
  : SymbolicMathParser(expression), _expression(expression)
{
}
