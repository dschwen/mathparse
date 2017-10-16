#include "SymbolicMathFunctionSLJIT.h"
#include <stdio.h>

#include <jit/jit-dump.h>

namespace SymbolicMath
{

// lightning uses cascading macros to reduce the number of passed in arguments
#define _jit state.C

Function::~Function()
{
  // destroy context only at function end of life
  if (_jit_code)
    sljit_free_code(reinterpret_cast<void *>(_jit_code));
}

void
Function::compile()
{
  if (_jit_code)
    return;

  // global shit (TODO: move into singleton)
  init_jit(nullptr);

  // build and lock context
  JITStateValue state;
  state.C = jit_new_state();

  // build function
  jit_prolog();

  // determine max stack depth and allocate
  auto current_max = std::make_pair(0, 0);
  _root.stackDepth(current_max);
  state.stack_base = jit_allocai(current_max.second * sizeof(double));

  // start of stack in the empty. Actual top of stack is register F0 and at program start
  // that register does not contain anything to be pushed into the stack.
  state.sp = -1;

  if (current_max.first <= 0)
    fatalError("Stack depleted at function end");

  // build function from expression tree
  _root.jit(state);

  // return top of stack (register F0)
  jit_retr_d(JIT_F0);
  jit_epilog();

  // generate machine code
  _jit_code = jit_emit();

  // free the compiler data
  jit_clear_state();
}

Real
Function::value()
{
  if (_jit_code)
    // if a JIT compiled version exists evaluate it
    return _jit_code();
  else
    // otherwise recursively walk the expression tree (slow)
    return _root.value();
}

void
Function::invalidateJIT()
{
  if (_jit_code)
  {
    sljit_free_code(reinterpret_cast<void *>(_jit_code));
    _jit_code = nullptr;
  }
};

// undefine the lightning helper macro
#undef _jit

// end namespace SymbolicMath
}
