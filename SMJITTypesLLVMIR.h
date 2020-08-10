///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"

namespace SymbolicMath
{

const std::string jit_backend_name = "LLVMIR";

typedef Real (*JITFunctionPtr)();

using JITReturnValue = llvm::Value *;

// basic block?!
struct JITStateValue
{
  JITStateValue(llvm::BasicBlock * BB, llvm::Module * M_) : builder(BB), M(M_) {}
  llvm::IRBuilder<> builder;
  llvm::Module * M;
};

} // namespace SymbolicMath
