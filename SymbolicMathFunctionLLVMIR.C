#include "SymbolicMathFunctionLLVMIR.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
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
  llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
}

Function::Function(const Node & root)
  : FunctionBase((initialize(), root)),
    _llvm_target_machine(llvm::EngineBuilder().selectTarget()),
    _llvm_data_layout(_llvm_target_machine->createDataLayout()),
    _llvm_object_layer([]() { return std::make_shared<llvm::SectionMemoryManager>(); }),
    _llvm_compile_layer(_llvm_object_layer, llvm::orc::SimpleCompiler(*_llvm_target_machine)),
    _llvm_optimize_layer(_llvm_compile_layer, [this](std::shared_ptr<llvm::Module> M) {
      return optimizeModule(std::move(M));
    })
{
}

Function::Function(const Function & F)
  : FunctionBase(F._root),
    _llvm_target_machine(llvm::EngineBuilder().selectTarget()),
    _llvm_data_layout(_llvm_target_machine->createDataLayout()),
    _llvm_object_layer([]() { return std::make_shared<llvm::SectionMemoryManager>(); }),
    _llvm_compile_layer(_llvm_object_layer, llvm::orc::SimpleCompiler(*_llvm_target_machine)),
    _llvm_optimize_layer(_llvm_compile_layer, [this](std::shared_ptr<llvm::Module> M) {
      return optimizeModule(std::move(M));
    })
{
}

Function::~Function()
{
  // CompileLayer.removeModuleSet(_H);
}

void
Function::compile()
{
  if (_jit_code)
    return;

  llvm::LLVMContext context;

  // Build Function and basic block
  std::unique_ptr<llvm::Module> M = llvm::make_unique<llvm::Module>("function", context);
  M->setDataLayout(_llvm_data_layout);

  llvm::Function * F =
      llvm::cast<llvm::Function>(M->getOrInsertFunction("F", llvm::Type::getDoubleTy(context)));

  llvm::BasicBlock * BB = llvm::BasicBlock::Create(context, "EntryBlock", F);
  JITStateValue state(BB, M.get());

  // return result
  state.builder.CreateRet(_root.jit(state));

  // print module (debug)
  // M->print(llvm::errs(), nullptr);

  // Build our symbol resolver:
  // Lambda 1: Look back into the JIT itself to find symbols that are part of
  //           the same "logical dylib".
  // Lambda 2: Search for external symbols in the host process.
  auto Resolver = llvm::orc::createLambdaResolver(
      [&](const std::string & Name) {
        if (auto Sym = _llvm_optimize_layer.findSymbol(Name, false))
          return Sym;
        return llvm::JITSymbol(nullptr);
      },
      [](const std::string & Name) {
        if (auto SymAddr = llvm::RTDyldMemoryManager::getSymbolAddressInProcess(Name))
          return llvm::JITSymbol(SymAddr, llvm::JITSymbolFlags::Exported);
        return llvm::JITSymbol(nullptr);
      });

  // Add the set to the JIT with the resolver we created above and a newly
  // created SectionMemoryManager.
  _module_handle =
      llvm::cantFail(_llvm_optimize_layer.addModule(std::move(M), std::move(Resolver)));

  std::string buf;
  llvm::raw_string_ostream mangled(buf);
  llvm::Mangler::getNameWithPrefix(mangled, "F", _llvm_data_layout);

  // std::cout << mangled.str() << '\n';
  auto ExprSymbol = _llvm_optimize_layer.findSymbol(mangled.str(), false);
  if (!ExprSymbol)
    fatalError("Function not found\n");

  // Get the symbol's address and cast it to the right type (takes no
  // arguments, returns a double) so we can call it as a native function.
  _jit_code = reinterpret_cast<JITFunctionPtr>(llvm::cantFail(ExprSymbol.getAddress()));
  // _jit_code = (double (*)())(intptr_t)(ExprSymbol.getAddress());

  // std::cout << "EVALUATES to " << _jit_code() << '\n';
}

std::shared_ptr<llvm::Module>
Function::optimizeModule(std::shared_ptr<llvm::Module> M)
{
  // Create a function pass manager.
  auto FPM = llvm::make_unique<llvm::legacy::FunctionPassManager>(M.get());

  // Add some optimizations.
  FPM->add(llvm::createInstructionCombiningPass());
  FPM->add(llvm::createReassociatePass());
  FPM->add(llvm::createGVNPass());
  FPM->add(llvm::createCFGSimplificationPass());
  FPM->doInitialization();

  // Run the optimizations over all functions in the module being added to
  // the JIT.
  for (auto & F : *M)
    FPM->run(F);

  return M;
}

void
Function::invalidateJIT()
{
  if (_jit_code)
  {
    llvm::cantFail(_llvm_optimize_layer.removeModule(_module_handle));
    _jit_code = nullptr;
  }
};

// end namespace SymbolicMath
}
