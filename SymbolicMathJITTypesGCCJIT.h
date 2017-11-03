#ifndef SYMBOLICMATHTYPESGCCJIT_H
#define SYMBOLICMATHTYPESGCCJIT_H

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

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHTYPESGCCJIT_H
