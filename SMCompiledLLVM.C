///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include "SMFunction.h"
#include "SMCompiledLLVM.h"

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

#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/Support/Debug.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "llvm/Support/TargetRegistry.h"

using namespace llvm;
using namespace llvm::orc;

namespace SymbolicMath
{

template <typename T>
CompiledLLVM<T>::CompiledLLVM(Function<T> & fb) : Transform<T>(fb), _jit_function(nullptr)
{
  // global one time initialization
  static struct InitializationSingleton
  {
    InitializationSingleton()
    {
      llvm::InitializeNativeTarget();
      llvm::InitializeNativeTargetAsmPrinter();
      llvm::InitializeNativeTargetAsmParser();
    }
  } initialize;

  _lljit = std::unique_ptr<Helper>(new Helper);

  auto C = llvm::make_unique<llvm::LLVMContext>();
  auto M = llvm::make_unique<llvm::Module>("LLJIT", *C);
  M->setDataLayout(_lljit->getDataLayout());

  auto & ctx = M->getContext();
  auto * FT = llvm::FunctionType::get(llvm::Type::getDoubleTy(ctx), false);
  auto * F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "F", M.get());

  auto * BB = llvm::BasicBlock::Create(ctx, "EntryBlock", F);
  _state = std::unique_ptr<JITStateValue>(new JITStateValue(BB, M.get()));

  // return result
  apply();
  _state->builder.CreateRet(_value);

  std::string buffer;
  llvm::raw_string_ostream es(buffer);

  if (verifyFunction(*F, &es))
    throw std::runtime_error("Function verification failed: " + es.str());

  if (verifyModule(*M, &es))
    throw std::runtime_error("Module verification failed: " + es.str());

  if (_lljit->submitModule(std::move(M), std::move(C)))
    throw std::runtime_error("Module submission failed");

  // Request function; this compiles to machine code and links.
  _jit_function = llvm::jitTargetAddressToPointer<JITFunctionPtr>(*(_lljit->getFunctionAddr("F")));
}

template <typename T>
CompiledLLVM<T>::~CompiledLLVM()
{
}

// Visitor operators

template <typename T>
void
CompiledLLVM<T>::operator()(SymbolData<T> * n)
{
  fatalError("Symbol in compiled function");
}

template <typename T>
void
CompiledLLVM<T>::operator()(UnaryOperatorData<T> * n)
{
  n->_args[0].apply(*this);

  switch (n->_type)
  {
    case UnaryOperatorType::PLUS:
      return;

    case UnaryOperatorType::MINUS:
      _value = _state->builder.CreateFNeg(_value);
      return;

    default:
      fatalError("Unknown operator");
  }
}

template <typename T>
void
CompiledLLVM<T>::operator()(BinaryOperatorData<T> * n)
{
  n->_args[0].apply(*this);
  const auto A = _value;
  n->_args[1].apply(*this);
  const auto B = _value;

  switch (n->_type)
  {
    case BinaryOperatorType::SUBTRACTION:
      _value = _state->builder.CreateFSub(A, B);
      return;

    case BinaryOperatorType::DIVISION:
      _value = _state->builder.CreateFDiv(A, B);
      return;

    case BinaryOperatorType::MODULO:
      fatalError("Operator not implemented yet");

    case BinaryOperatorType::POWER:
      _value = _state->builder.CreateCall(
          Intrinsic::getDeclaration(
              _state->M, llvm::Intrinsic::pow, {_state->builder.getDoubleTy()}),
          {A, B});
      return;

    case BinaryOperatorType::LOGICAL_OR:
      _value = _state->builder.CreateOr(A, B); //?
      return;

    case BinaryOperatorType::LOGICAL_AND:
      _value = _state->builder.CreateAnd(A, B); //?
      return;

    case BinaryOperatorType::LESS_THAN:
      _value = _state->builder.CreateFCmpOLT(A, B);
      return;

    case BinaryOperatorType::GREATER_THAN:
      _value = _state->builder.CreateFCmpOGT(A, B);
      return;

    case BinaryOperatorType::LESS_EQUAL:
      _value = _state->builder.CreateFCmpOLE(A, B);
      return;

    case BinaryOperatorType::GREATER_EQUAL:
      _value = _state->builder.CreateFCmpOGE(A, B);
      return;

    case BinaryOperatorType::EQUAL:
      _value = _state->builder.CreateFCmpOEQ(A, B);
      return;

    case BinaryOperatorType::NOT_EQUAL:
      _value = _state->builder.CreateFCmpONE(A, B);
      return;

    default:
      fatalError("Unknown operator");
  }
}

template <typename T>
void
CompiledLLVM<T>::operator()(MultinaryOperatorData<T> * n)
{
  if (n->_args.size() == 0)
    fatalError("No child nodes in multinary operator");

  n->_args[0].apply(*this);
  if (n->_args.size() == 1)
    return;

  auto tmp = _value;
  for (std::size_t i = 1; i < n->_args.size(); ++i)
  {
    n->_args[i].apply(*this);
    switch (n->_type)
    {
      case MultinaryOperatorType::ADDITION:
        tmp = _state->builder.CreateFAdd(tmp, _value);
        break;

      case MultinaryOperatorType::MULTIPLICATION:
        tmp = _state->builder.CreateFMul(tmp, _value);
        break;

      default:
        fatalError("Unknown operator");
    }
  }
  _value = tmp;
}

template <>
void
CompiledLLVM<Real>::operator()(UnaryFunctionData<Real> * n)
{
  llvm::Intrinsic::ID func;

  switch (n->_type)
  {
    case UnaryFunctionType::ABS:
      func = llvm::Intrinsic::fabs;
      break;

      // case UnaryFunctionType::ACOS:
      //
      // case UnaryFunctionType::ACOSH:
      //
      // case UnaryFunctionType::ARG:
      //
      // case UnaryFunctionType::ASIN:
      //
      // case UnaryFunctionType::ASINH:
      //
      // case UnaryFunctionType::ATAN:
      //
      // case UnaryFunctionType::ATANH:
      //
      // case UnaryFunctionType::CBRT:

    case UnaryFunctionType::CEIL:
      func = llvm::Intrinsic::ceil;
      break;

      // case UnaryFunctionType::CONJ:

    case UnaryFunctionType::COS:
      func = llvm::Intrinsic::cos;
      break;

      // case UnaryFunctionType::COSH:
      //
      // case UnaryFunctionType::COT:
      //
      // case UnaryFunctionType::CSC:
      //
      // case UnaryFunctionType::ERF:

    case UnaryFunctionType::EXP:
      func = llvm::Intrinsic::exp;
      break;

    case UnaryFunctionType::EXP2:
      func = llvm::Intrinsic::exp2;
      break;

    case UnaryFunctionType::FLOOR:
      func = llvm::Intrinsic::floor;
      break;

      // case UnaryFunctionType::IMAG:

    case UnaryFunctionType::INT:
      func = llvm::Intrinsic::round;
      break;

    case UnaryFunctionType::LOG:
      func = llvm::Intrinsic::log;
      break;

    case UnaryFunctionType::LOG10:
      func = llvm::Intrinsic::log10;
      break;

    case UnaryFunctionType::LOG2:
      func = llvm::Intrinsic::log2;
      break;

      // case UnaryFunctionType::REAL:

    case UnaryFunctionType::SEC:
      // return jit_insn_div(
      //     func,
      //     jit_value_create_float64_constant(func, jit_type_float64, (jit_float64)1.0),
      //     jit_insn_cos(func, A));

    case UnaryFunctionType::SIN:
      func = llvm::Intrinsic::sin;
      break;

      // case UnaryFunctionType::SINH:

    case UnaryFunctionType::SQRT:
      func = llvm::Intrinsic::sqrt;
      break;

      // case UnaryFunctionType::T:
      // case UnaryFunctionType::TAN:
      // case UnaryFunctionType::TANH:

    case UnaryFunctionType::TRUNC:
      func = llvm::Intrinsic::trunc;
      break;

    default:
      fatalError("Function not implemented");
  }

  n->_args[0].apply(*this);
  _value = _state->builder.CreateCall(
      Intrinsic::getDeclaration(_state->M, func, {_state->builder.getDoubleTy()}), {_value});
}

template <typename T>
void
CompiledLLVM<T>::operator()(BinaryFunctionData<T> * n)
{
  n->_args[0].apply(*this);
  const auto A = _value;
  n->_args[1].apply(*this);
  const auto B = _value;

  llvm::Intrinsic::ID func;

  switch (n->_type)
  {
      // case BinaryFunctionType::ATAN2:
      // case BinaryFunctionType::HYPOT:
      // case BinaryFunctionType::PLOG:

    case BinaryFunctionType::MIN:
      func = llvm::Intrinsic::minimum;
      return;

    case BinaryFunctionType::MAX:
      func = llvm::Intrinsic::maximum;
      return;

    case BinaryFunctionType::POW:
      func = llvm::Intrinsic::pow;
      break;

      // case BinaryFunctionType::POLAR:
    default:
      fatalError("Function not implemented");
  }

  _value = _state->builder.CreateCall(
      Intrinsic::getDeclaration(_state->M, func, {_state->builder.getDoubleTy()}), {A, B});
}

template <>
void
CompiledLLVM<Real>::operator()(RealNumberData<Real> * n)
{
  _value = ConstantFP::get(_state->builder.getDoubleTy(), n->_value);
}

template <>
void
CompiledLLVM<Real>::operator()(RealReferenceData<Real> * n)
{
  auto adr = ConstantInt::get(_state->builder.getInt64Ty(), (int64_t)&n->_ref);
  auto ptr = llvm::ConstantExpr::getIntToPtr(
      adr, llvm::PointerType::getUnqual(_state->builder.getDoubleTy()));
  _value = _state->builder.CreateLoad(ptr);
}

template <typename T>
void
CompiledLLVM<T>::operator()(RealArrayReferenceData<T> * n)
{
  fatalError("Not implemented yet");
}

template <typename T>
void
CompiledLLVM<T>::operator()(LocalVariableData<T> * n)
{
  fatalError("Not implemented");
}

template <typename T>
void
CompiledLLVM<T>::operator()(ConditionalData<T> * n)
{
  fatalError("Conditional not implemented");
}

template <>
void
CompiledLLVM<Real>::operator()(IntegerPowerData<Real> * n)
{
  _value = ConstantFP::get(_state->builder.getDoubleTy(), 1.0);

  n->_arg.apply(*this);
  auto A = _value;

  int e = n->_exponent > 0 ? n->_exponent : -n->_exponent;
  while (e)
  {
    // if bit 0 is set multiply the current power of two factor of the exponent
    if (e & 1)
      _value = _state->builder.CreateFMul(_value, A);

    // x is incrementally set to consecutive powers of powers of two
    A = _state->builder.CreateFMul(A, A);

    // bit shift the exponent down
    e >>= 1;
  }

  if (n->_exponent < 0)
    _value =
        _state->builder.CreateFDiv(ConstantFP::get(_state->builder.getDoubleTy(), 1.0), _value);
}

template <typename T>
CompiledLLVM<T>::Helper::Helper()
{
  LLJITBuilder Builder;
  // TT = Builder.JTMB->getTargetTriple();
  LLJIT = std::move(Builder.create().get());

  if (auto R = createHostProcessResolver(LLJIT->getDataLayout()))
    LLJIT->getMainJITDylib().setGenerator(std::move(R));
}

template <typename T>
JITDylib::GeneratorFunction
CompiledLLVM<T>::Helper::createHostProcessResolver(DataLayout DL)
{
  char Prefix = DL.getGlobalPrefix();
  Expected<JITDylib::GeneratorFunction> R =
      DynamicLibrarySearchGenerator::GetForCurrentProcess(Prefix);

  if (!R)
  {
    LLJIT->getExecutionSession().reportError(R.takeError());
    return nullptr;
  }

  if (!*R)
  {
    LLJIT->getExecutionSession().reportError(createStringError(
        inconvertibleErrorCode(), "Generator function for host process symbols must not be null"));
    return nullptr;
  }

  return *R;
}

template <typename T>
Error
CompiledLLVM<T>::Helper::submitModule(std::unique_ptr<Module> M, std::unique_ptr<LLVMContext> C)
{
  return LLJIT->addIRModule(ThreadSafeModule(std::move(M), std::move(C)));
}

template <typename T>
Expected<JITTargetAddress>
CompiledLLVM<T>::Helper::getFunctionAddr(StringRef Name)
{
  Expected<JITEvaluatedSymbol> S = LLJIT->lookup(Name);
  if (!S)
    return S.takeError();

  JITTargetAddress A = S->getAddress();
  if (!A)
    return createStringError(inconvertibleErrorCode(), "'%s' evaluated to nullptr", Name.data());

  return A;
}

template class CompiledLLVM<Real>;

} // namespace SymbolicMath
