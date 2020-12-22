///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include "SMTransform.h"
#include "SMEvaluable.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Verifier.h>
#include "llvm/IR/Type.h"
#include <llvm/Support/Format.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Triple.h>
#include <llvm/ExecutionEngine/JITSymbol.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Error.h>

#include <memory>

namespace SymbolicMath
{

/**
 * LibJIT compiler transform
 */
template <typename T>
class CompiledLLVM : public Transform<T>, public Evaluable<T>
{
  using Transform<T>::apply;

public:
  CompiledLLVM(Function<T> &);
  ~CompiledLLVM() override;

  void operator()(Node<T> &, SymbolData<T> &) override;

  void operator()(Node<T> &, UnaryOperatorData<T> &) override;
  void operator()(Node<T> &, BinaryOperatorData<T> &) override;
  void operator()(Node<T> &, MultinaryOperatorData<T> &) override;

  void operator()(Node<T> &, UnaryFunctionData<T> &) override;
  void operator()(Node<T> &, BinaryFunctionData<T> &) override;

  void operator()(Node<T> &, RealNumberData<T> &) override;
  void operator()(Node<T> &, RealReferenceData<T> &) override;
  void operator()(Node<T> &, RealArrayReferenceData<T> &) override;
  void operator()(Node<T> &, LocalVariableData<T> &) override;

  void operator()(Node<T> &, ConditionalData<T> &) override;
  void operator()(Node<T> &, IntegerPowerData<T> &) override;

  T operator()() override { return _jit_function(); }

protected:
  class Helper;
  std::unique_ptr<Helper> _lljit;

  enum class Native
  {
    acos,
    acosh,
    asin,
    asinh,
    atan,
    atanh,
    cbrt,
    cosh,
    cot,
    csc,
    erf,
    erfc,
    sec,
    sinh,
    tan,
    tanh,
    atan2,
    plog
  };
  std::map<Native, llvm::Function *> _native;

  typedef Real (*JITFunctionPtr)();

  llvm::Value * _value;

  struct JITStateValue
  {
    JITStateValue(llvm::BasicBlock * BB, llvm::Module * M_) : builder(BB), M(M_) {}
    llvm::IRBuilder<> builder;
    llvm::Module * M;
  };
  std::unique_ptr<JITStateValue> _state;

  JITFunctionPtr _jit_function;
};

template <typename T>
class CompiledLLVM<T>::Helper
{
public:
  Helper();

  // Not a value type.
  Helper(const Helper &) = delete;
  Helper & operator=(const Helper &) = delete;
  Helper(Helper &&) = delete;
  Helper & operator=(Helper &&) = delete;

  llvm::DataLayout getDataLayout() const { return _lljit->getDataLayout(); }

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
  std::unique_ptr<llvm::orc::LLJIT> _lljit;

  llvm::orc::JITDylib::GeneratorFunction createHostProcessResolver(llvm::DataLayout DL);
};

} // namespace SymbolicMath
