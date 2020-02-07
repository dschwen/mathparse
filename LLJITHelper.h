#pragma once

#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Triple.h>
#include <llvm/ExecutionEngine/JITSymbol.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Error.h>

#include <functional>
#include <memory>

class LLJITHelper
{
public:
  LLJITHelper();

  // Not a value type.
  LLJITHelper(const LLJITHelper &) = delete;
  LLJITHelper & operator=(const LLJITHelper &) = delete;
  LLJITHelper(LLJITHelper &&) = delete;
  LLJITHelper & operator=(LLJITHelper &&) = delete;

  llvm::DataLayout getDataLayout() const { return LLJIT->getDataLayout(); }

  // const llvm::Triple & getTargetTriple() const { return TT; }

  llvm::Error submitModule(std::unique_ptr<llvm::Module> M, std::unique_ptr<llvm::LLVMContext> C);

  template <class Signature_t>
  llvm::Expected<std::function<Signature_t>> getFunction(llvm::StringRef Name)
  {
    if (auto A = getFunctionAddr(Name))
      return std::function<Signature_t>(llvm::jitTargetAddressToPointer<Signature_t *>(*A));
    else
      return A.takeError();
  }

  llvm::Expected<llvm::JITTargetAddress> getFunctionAddr(llvm::StringRef Name);

private:
  std::unique_ptr<llvm::orc::LLJIT> LLJIT;
  // llvm::Triple TT;

  llvm::orc::JITDylib::GeneratorFunction createHostProcessResolver(llvm::DataLayout DL);
};
