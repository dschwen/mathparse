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
  if (_jit_closure)
    return;

  _jit_context = jit_context_create();
  jit_context_build_start(_jit_context);

  jit_type_t signature = jit_type_create_signature(jit_abi_cdecl, jit_type_float64, nullptr, 0, 1);
  jit_function_t function = jit_function_create(_jit_context, signature);
  jit_type_free(signature);

  jit_insn_return(function, _root.jit(function));
  // jit_dump_function(stdout, function, 0);

  jit_function_compile(function);
  // jit_dump_function(stdout, function, 0);

  jit_context_build_end(_jit_context);

  _jit_closure = reinterpret_cast<JITFunctionPtr>(jit_function_to_closure(function));
}

Real
Function::value()
{
  if (_jit_closure)
    return _jit_closure();
  else
    return _root.value();
}

// end namespace SymbolicMath
}
