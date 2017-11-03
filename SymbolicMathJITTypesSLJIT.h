#ifndef SYMBOLICMATHTYPESSLJIT_H
#define SYMBOLICMATHTYPESSLJIT_H

#include "contrib/sljit_src/sljitLir.h"

namespace SymbolicMath
{

const std::string jit_backend_name = "SLJIT";

using JITFunctionPtr = long SLJIT_CALL (*)();

using JITReturnValue = void;

struct JITStateValue
{
  sljit_f64 * stack;
  struct sljit_compiler * C;
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHTYPESSLJIT_H
