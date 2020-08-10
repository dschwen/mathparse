///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include <libgccjit.h>

namespace SymbolicMath
{

const std::string jit_backend_name = "GCCJIT";

typedef Real (*JITFunctionPtr)();

using JITReturnValue = gcc_jit_rvalue *;

struct JITStateValue
{
  gcc_jit_context * ctxt;
  gcc_jit_function * func;
  gcc_jit_block * block;
};

} // namespace SymbolicMath
