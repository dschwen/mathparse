#ifndef SYMBOLICMATHTYPESLLVMIR_H
#define SYMBOLICMATHTYPESLLVMIR_H

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"

namespace SymbolicMath
{

const std::string jit_backend_name = "LLVMIR";

using JITReturnValue = llvm::Value *;

// basic block?!
using JITStateValue = llvm::IRBuilder<> &;

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHTYPESLLVMIR_H
