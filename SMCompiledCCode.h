///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include <stack>

#include "SMEvaluable.h"
#include "SMFunction.h"

namespace SymbolicMath
{

/**
 * C source code compilation, dynamic object laoding, evaluation
 */
template <typename T>
class CompiledCCode : public Evaluable<T>
{
public:
  CompiledCCode(Function<T> &);

  T operator()() override { return _jit_function(); }

protected:
  const std::string typeHeader();

  typedef Real (*JITFunctionPtr)();

  JITFunctionPtr _jit_function;
};

} // namespace SymbolicMath
