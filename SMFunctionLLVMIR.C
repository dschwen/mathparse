///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include "SMFunctionLLVMIR.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/LambdaResolver.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/Mangler.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "llvm/Support/TargetRegistry.h"

namespace SymbolicMath
{

FunctionJITInitialization::FunctionJITInitialization()
{
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();
}

Function::Function(const Node & root) : FunctionBase((initialize(), root)) {}

Function::Function(const Function & F) : FunctionBase(F._root) {}

Function::~Function() { invalidateJIT(); }

void
Function::compile()
{
  if (_jit_code)
    return;

  // std::cout << _lljit.getTargetTriple().normalize() << "\n\n";

  auto C = llvm::make_unique<llvm::LLVMContext>();
  auto M = llvm::make_unique<llvm::Module>("LLJIT", *C);
  M->setDataLayout(_lljit.getDataLayout());

  auto & ctx = M->getContext();
  auto * FT = llvm::FunctionType::get(llvm::Type::getDoubleTy(ctx), false);
  auto * F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "F", M.get());

  auto * BB = llvm::BasicBlock::Create(ctx, "EntryBlock", F);
  JITStateValue state(BB, M.get());

  // return result
  state.builder.CreateRet(_root.jit(state));

  std::string buffer;
  llvm::raw_string_ostream es(buffer);

  if (verifyFunction(*F, &es))
  {
    std::cerr << "Function verification failed: %s" << es.str() << '\n';
    std::exit(1);
  }

  if (verifyModule(*M, &es))
  {
    std::cerr << "Module verification failed: " << es.str() << '\n';
    std::exit(1);
  }

  if (_lljit.submitModule(std::move(M), std::move(C)))
  {
    std::cerr << "Greeeeeet saak-seeeesss!\n";
  }

  // Request function; this compiles to machine code and links.
  // _jit_code = _lljit.getFunction<double *()>("F").get();
  _jit_code = llvm::jitTargetAddressToPointer<JITFunctionPtr>(*(_lljit.getFunctionAddr("F")));
}

void
Function::invalidateJIT()
{
  if (_jit_code)
  {
    // llvm::cantFail(_llvm_optimize_layer.removeModule(_module_handle));
    _jit_code = nullptr;
  }
};

} // namespace SymbolicMath
