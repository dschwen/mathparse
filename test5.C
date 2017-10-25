#include "llvm/ADT/STLExtras.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/Orc/JITSymbol.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/LambdaResolver.h"
// #include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Mangler.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

#include "llvm/Support/TargetRegistry.h"

// using ModuleHandle = decltype(CompileLayer)::ModuleHandleT;

int
main()
{
  // move into singleton
  LLVMInitializeNativeTarget();

  std::unique_ptr<llvm::TargetMachine> TM(llvm::EngineBuilder().selectTarget());
  auto DL = TM->createDataLayout();

  // LLVM 5.0.0
  // RTDyldObjectLinkingLayer ObjectLayer;
  // LLVM 3.9.0
  llvm::orc::ObjectLinkingLayer<> ObjectLayer;

  // LLVM 5.0.0
  // llvm::orc::IRCompileLayer<decltype(ObjectLayer), llvm::orc::SimpleCompiler> CompileLayer(
  //     ObjectLayer, llvm::orc::SimpleCompiler(*TM));
  // LLVM 3.9.0
  llvm::orc::IRCompileLayer<decltype(ObjectLayer)> CompileLayer(ObjectLayer,
                                                                llvm::orc::SimpleCompiler(*TM));

  llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);

  llvm::LLVMContext Context;

  llvm::sys::Process::PreventCoreFiles();

  // Build IR
  std::unique_ptr<llvm::Module> M = llvm::make_unique<llvm::Module>("function", Context);
  llvm::Function * F = llvm::cast<llvm::Function>(
      M->getOrInsertFunction("F", llvm::Type::getInt32Ty(Context), (llvm::Type *)0));

  llvm::BasicBlock * BB = llvm::BasicBlock::Create(Context, "EntryBlock", F);
  llvm::IRBuilder<> Builder(BB);

  // return 10
  llvm::Value * Ten = Builder.getInt32(10);
  Builder.CreateRet(Ten);

  // JIT stuff

  std::vector<std::unique_ptr<llvm::Module>> Ms;
  Ms.push_back(std::move(M));

  // Build our symbol resolver:
  // Lambda 1: Look back into the JIT itself to find symbols that are part of
  //           the same "logical dylib".
  // Lambda 2: Search for external symbols in the host process.
  auto Resolver = llvm::orc::createLambdaResolver(
      [&](const std::string & Name) {
        if (auto Sym = CompileLayer.findSymbol(Name, false))
          return Sym.toRuntimeDyldSymbol();
        return llvm::RuntimeDyld::SymbolInfo(nullptr);
      },
      [](const std::string & Name) {
        if (auto SymAddr = llvm::RTDyldMemoryManager::getSymbolAddressInProcess(Name))
          return llvm::RuntimeDyld::SymbolInfo(SymAddr, llvm::JITSymbolFlags::Exported);
        return llvm::RuntimeDyld::SymbolInfo(nullptr);
      });

  // Add the set to the JIT with the resolver we created above and a newly
  // created SectionMemoryManager.
  auto CL = CompileLayer.addModuleSet(
      std::move(Ms), llvm::make_unique<llvm::SectionMemoryManager>(), std::move(Resolver));

  return 0;
}
