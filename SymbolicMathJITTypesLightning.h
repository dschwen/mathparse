///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

extern "C"
{
#include <lightning.h>
}

namespace SymbolicMath
{

const std::string jit_backend_name = "GNU Lightning";

typedef Real (*JITFunctionPtr)();

using JITReturnValue = void;

struct JITStateValue
{
  /// current stack entry (as array index)
  int sp;

  /// base offset of the stack from the frame pointer in bytes
  int stack_base;

  /// lightning compiler state
  jit_state_t * C;
};

} // namespace SymbolicMath
