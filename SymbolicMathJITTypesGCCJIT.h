#ifndef SYMBOLICMATHTYPESGCCJIT_H
#define SYMBOLICMATHTYPESGCCJIT_H

#include <libgccjit.h>

namespace SymbolicMath
{

const std::string jit_backend_name = "GCCJIT";

using JITReturnValue = gcc_jit_rvalue *;

using JITStateValue = gcc_jit_context *;

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHTYPESGCCJIT_H
