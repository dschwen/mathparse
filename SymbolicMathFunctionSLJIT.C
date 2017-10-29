#include "SymbolicMathFunctionSLJIT.h"
#include <stdio.h>

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

  // determine max stack depth and allocate
  auto current_max = std::make_pair(0, 0);
  _root.stackDepth(current_max);
  state.sp = -1;

  // build function
  sljit_emit_enter(state.C, 0, 0, 4, 0, 4, 0, current_max.second * sizeof(double));

  if (current_max.first <= 0)
    fatalError("Stack depleted at function end");

  // build function from expression tree
  _root.jit(state);

  // return stack top (FR0)
  sljit_emit_return(state.C, SLJIT_MOV, SLJIT_FR0, 0);

  // generate machine code
  _jit_code = reinterpret_cast<JITFunctionPtr>(sljit_generate_code(state.C));

  // free the compiler data
  sljit_free_compiler(state.C);
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

// end namespace SymbolicMath
}
