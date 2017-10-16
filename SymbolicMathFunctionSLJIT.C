#include "SymbolicMathFunctionSLJIT.h"
#include <stdio.h>

#include <jit/jit-dump.h>

namespace SymbolicMath
{

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

  // build and lock context
  JITStateValue state;
  state.C = sljit_create_compiler(NULL);

  // build function
  sljit_emit_enter(state.C, 0, 0, 0, 0, 0, 0, 0);

  // determine max stack depth and allocate
  _stack.resize(_root.stackDepth(_final_stack_ptr));
  state.stack = _stack.data();

  // build function from expression tree
  _root.jit(state);

  // return nothing (result is on the stack at position _final_stack_ptr)
  sljit_emit_return(state.C, SLJIT_MOV, SLJIT_R0, 0);

  // generate machine code
  _jit_code = reinterpret_cast<JITFunction>(sljit_generate_code(state.C));

  // free the compiler data
  sljit_free_compiler(state.C);
}

Real
Function::value()
{
  if (_jit_code)
  {
    // if a JIT compiled version exists evaluate it
    _jit_code();
    return _stack[_final_stack_ptr];
  }
  else
    // otherwise recursively walk the expression tree (slow)
    return _root.value();
}

// end namespace SymbolicMath
}
