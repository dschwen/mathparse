#include "SymbolicMathFunction.h"
#include <stdio.h>

#include <jit/jit-dump.h>

namespace SymbolicMath
{

Function::~Function()
{
  if (_jit_context)
    jit_context_destroy(_jit_context);
}

void
Function::compile()
{
  if (_jit_function)
    return;

  _jit_context = jit_context_create();
  jit_context_build_start(_jit_context);

  jit_type_t signature = jit_type_create_signature(jit_abi_cdecl, jit_type_float64, nullptr, 0, 1);
  _jit_function = jit_function_create(_jit_context, signature);
  jit_type_free(signature);

  jit_insn_return(_jit_function, _root.jit(_jit_function));
  jit_dump_function(stdout, _jit_function, 0);

  jit_function_compile(_jit_function);
  // jit_dump_function(stdout, _jit_function, 0);

  jit_context_build_end(_jit_context);
}

Real
Function::value()
{
  if (_jit_function)
  {
    Real result = 123.456;
    jit_function_apply(_jit_function, nullptr, &result);
    return result;
  }
  else
    return _root.value();
}

// end namespace SymbolicMath
}
