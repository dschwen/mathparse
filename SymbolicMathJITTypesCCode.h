#ifndef SYMBOLICMATHTYPESCCODE_H
#define SYMBOLICMATHTYPESCCODE_H

namespace SymbolicMath
{

const std::string jit_backend_name = "C Code";

typedef Real (*JITFunctionPtr)();

using JITReturnValue = std::string;

using JITStateValue = bool;

} // namespace SymbolicMath

#endif // SYMBOLICMATHTYPESCCODE_H
