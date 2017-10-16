#ifndef SYMBOLICMATHTYPESSLJIT_H
#define SYMBOLICMATHTYPESSLJIT_H

#include "contrib/sljit_src/sljitLir.h"

namespace SymbolicMath
{

using JITReturnValue = void;

struct JITStateValue
{
  sljit_f64 * stack;
  struct sljit_compiler * C;
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHTYPESSLJIT_H
