#include "llvm/ADT/STLExtras.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/LambdaResolver.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
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
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/TargetSelect.h"

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
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  std::unique_ptr<llvm::TargetMachine> TM(llvm::EngineBuilder().selectTarget());
  auto DL = TM->createDataLayout();

  llvm::orc::RTDyldObjectLinkingLayer ObjectLayer(
      []() { return std::make_shared<llvm::SectionMemoryManager>(); });

  llvm::orc::IRCompileLayer<decltype(ObjectLayer), llvm::orc::SimpleCompiler> CompileLayer(
      ObjectLayer, llvm::orc::SimpleCompiler(*TM));

  llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);

  llvm::LLVMContext Context;
  llvm::IRBuilder<> Builder(Context);

  // Build Function and basic block
  std::unique_ptr<llvm::Module> M = llvm::make_unique<llvm::Module>("function", Context);
  M->setDataLayout(DL);

  llvm::Function * F =
      llvm::cast<llvm::Function>(M->getOrInsertFunction("F", llvm::Type::getDoubleTy(Context)));

  llvm::BasicBlock * BB = llvm::BasicBlock::Create(Context, "EntryBlock", F);
  Builder.SetInsertPoint(BB);

  // Load constant 10
  llvm::Value * Ten = llvm::ConstantFP::get(Builder.getDoubleTy(), 10.0001);

  // // load variable value
  // double c = 20.0002;
  // llvm::Constant * adr = llvm::ConstantInt::get(Builder.getInt64Ty(), (int64_t)&c);
  // llvm::Value * ptr =
  //     llvm::ConstantExpr::getIntToPtr(adr, llvm::PointerType::getUnqual(Builder.getDoubleTy()));
  // // llvm::ConstantExpr::getIntToPtr(adr, llvm::PointerType::getUnqual(Builder.getInt64Ty()));
  // llvm::Value * cval = Builder.CreateLoad(Builder.getDoubleTy(), ptr, "c");
  // // llvm::Value * cval = Builder.CreateLoad(ptr, "c");
  //
  // // sum the two values
  // llvm::Value * res = Builder.CreateFAdd(cval, Ten, "cplus10");
  //
  // // return result
  // Builder.CreateRet(res);
  Builder.CreateRet(Ten);
  std::cout << "return " << Ten << std::endl;

  // verify
  llvm::verifyFunction(*F);
  std::cout << "verified" << std::endl;

  // print module
  M->print(llvm::errs(), nullptr);
  std::cout << "print" << std::endl;

  // Build our symbol resolver:
  // Lambda 1: Look back into the JIT itself to find symbols that are part of
  //           the same "logical dylib".
  // Lambda 2: Search for external symbols in the host process.
  auto Resolver = llvm::orc::createLambdaResolver(
      [&](const std::string & Name) {
        if (auto Sym = CompileLayer.findSymbol(Name, false))
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
  auto H = CompileLayer.addModule(std::move(M), std::move(Resolver));
  std::cout << "addmodule" << std::endl;

  std::string buf;
  llvm::raw_string_ostream mangled(buf);
  llvm::Mangler::getNameWithPrefix(mangled, "F", DL);

  auto ExprSymbol = CompileLayer.findSymbol(mangled.str(), false);
  if (!ExprSymbol)
  {
    std::cout << "Function not found\n";
    return 1;
  }

  // Get the symbol's address and cast it to the right type (takes no
  // arguments, returns a double) so we can call it as a native function.
  double (*FP)() = (double (*)())(intptr_t)llvm::cantFail(ExprSymbol.getAddress());
  std::cout << "Evaluated to " << FP() << '\n';

  // CompileLayer.removeModule(H);

  return 0;
}
