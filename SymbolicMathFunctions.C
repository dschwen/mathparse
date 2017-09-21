#include "SymbolicMathFunctions.h"

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

// end namespace SymbolicMath
}
