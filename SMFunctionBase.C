///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include "SMFunctionBase.h"

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

void
FunctionBase::apply(Transform & transform)
{
  _root.apply(transform);
}

} // namespace SymbolicMath
