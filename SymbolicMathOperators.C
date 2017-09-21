#include "SymbolicMathOperators.h"

namespace SymbolicMath
{

OperatorType
identifyOperator(const std::string & op)
{
  // we search from the back to avoid matching with the unary operators
  int i = static_cast<int>(OperatorType::INVALID) - 1;
  for (; i >= 0; --i)
    if (_operators[i]._form == op)
      return static_cast<OperatorType>(i);

  return OperatorType::INVALID;
}

// end namespace SymbolicMath
}
