#include "SymbolicMathSymbols.h"

namespace SymbolicMath
{

FunctionType
identifyFunction(const std::string & func)
{
  std::size_t i = 0;
  const auto last = static_cast<std::size_t>(FunctionType::INVALID);
  for (; i < last; ++i)
    if (_functions[i]._form == func)
      break;

  return static_cast<FunctionType>(i);
}

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
