#include "SymbolicMathFunctionBase.h"

namespace SymbolicMath
{

Real
FunctionBase::value()
{
  if (_jit_code)
  {
    // if a JIT compiled version exists evaluate it
    return _jit_code();
  }
  else
    // otherwise recursively walk the expression tree (slow)
    return _root.value();
}

} // namespace SymbolicMath
