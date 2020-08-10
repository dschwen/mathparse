///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include <jit/jit.h>

namespace SymbolicMath
{

const std::string jit_backend_name = "LibJIT";

typedef Real (*JITFunctionPtr)();

using JITReturnValue = jit_value_t;

using JITStateValue = jit_function_t;

} // namespace SymbolicMath
