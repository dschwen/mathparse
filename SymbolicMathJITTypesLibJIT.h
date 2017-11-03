#ifndef SYMBOLICMATHTYPESLIBJIT_H
#define SYMBOLICMATHTYPESLIBJIT_H

#include <jit/jit.h>

namespace SymbolicMath
{

const std::string jit_backend_name = "LibJIT";

typedef Real (*JITFunctionPtr)();

using JITReturnValue = jit_value_t;

using JITStateValue = jit_function_t;

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHTYPESLIBJIT_H
