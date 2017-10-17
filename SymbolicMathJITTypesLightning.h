#ifndef SYMBOLICMATHTYPESLIGHTNING_H
#define SYMBOLICMATHTYPESLIGHTNING_H

extern "C" {
#include <lightning.h>
}

namespace SymbolicMath
{

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

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHTYPESLIGHTNING_H
