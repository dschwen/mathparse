///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include "contrib/sljit_src/sljitLir.h"

namespace SymbolicMath
{

const std::string jit_backend_name = "SLJIT";

using JITFunctionPtr = Real SLJIT_FUNC (*)();

using JITReturnValue = void;

struct JITStateValue
{
  /// current stack entry (as array index)
  int sp;

  /// SLJIT compiler context
  struct sljit_compiler * C;
};

} // namespace SymbolicMath
