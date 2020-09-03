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

#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
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
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

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

// C Helper functions that are called from the JIT code

// clang-format off
extern "C" double sm_llvm_acos(double a) { return std::acos(a); }
extern "C" double sm_llvm_acosh(double a) { return std::acosh(a); }
extern "C" double sm_llvm_asin(double a) { return std::asin(a); }
extern "C" double sm_llvm_asinh(double a) { return std::asinh(a); }
extern "C" double sm_llvm_atan(double a) { return std::atan(a); }
extern "C" double sm_llvm_atanh(double a) { return std::atanh(a); }
extern "C" double sm_llvm_cbrt(double a) { return std::cbrt(a); }
extern "C" double sm_llvm_cosh(double a) { return std::cosh(a); }
extern "C" double sm_llvm_cot(double a) { return 1.0 / std::tan(a); }
extern "C" double sm_llvm_csc(double a) { return 1.0 / std::sin(a); }
extern "C" double sm_llvm_erf(double a) { return std::erf(a); }
extern "C" double sm_llvm_erfc(double a) { return std::erfc(a); }
extern "C" double sm_llvm_sec(double a) { return 1.0 / std::cos(a); }
extern "C" double sm_llvm_sinh(double a) { return std::sinh(a); }
extern "C" double sm_llvm_tan(double a) { return std::tan(a); }
extern "C" double sm_llvm_tanh(double a) { return std::tanh(a); }

extern "C" double sm_llvm_atan2(double a, double b) { return std::atan2(a, b); }
extern "C" double sm_llvm_plog(double a, double b) { return a < b ? std::log(b) + (a-b)/b - (a-b)*(a-b)/(2.0*b*b) + (a-b)*(a-b)*(a-b)/(3.0*b*b*b) : std::log(a); }
// clang-format on

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

  // setup bindings to native functions
  const std::vector<std::pair<Native, std::string>> unary_functions = {{Native::acos, "acos"},
                                                                       {Native::acosh, "acosh"},
                                                                       {Native::asin, "asin"},
                                                                       {Native::asinh, "asinh"},
                                                                       {Native::atan, "atan"},
                                                                       {Native::atanh, "atanh"},
                                                                       {Native::cbrt, "cbrt"},
                                                                       {Native::cosh, "cosh"},
                                                                       {Native::cot, "cot"},
                                                                       {Native::csc, "csc"},
                                                                       {Native::erf, "erf"},
                                                                       {Native::erfc, "erfc"},
                                                                       {Native::sec, "sec"},
                                                                       {Native::sinh, "sinh"},
                                                                       {Native::tan, "tan"},
                                                                       {Native::tanh, "tanh"}};

  for (auto & unary : unary_functions)
    _native[unary.first] = llvm::Function::Create(
        llvm::FunctionType::get(
            llvm::Type::getDoubleTy(ctx), {llvm::Type::getDoubleTy(ctx)}, false),
        llvm::GlobalValue::ExternalLinkage,
        "sm_llvm_" + unary.second,
        *M);

  const std::vector<std::pair<Native, std::string>> binary_functions = {{Native::atan2, "atan2"},
                                                                        {Native::plog, "plog"}};

  for (auto & binary : binary_functions)
    _native[binary.first] = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getDoubleTy(ctx),
                                {llvm::Type::getDoubleTy(ctx), llvm::Type::getDoubleTy(ctx)},
                                false),
        llvm::GlobalValue::ExternalLinkage,
        "sm_llvm_" + binary.second,
        *M);

  auto * FT = llvm::FunctionType::get(llvm::Type::getDoubleTy(ctx), false);
  auto * F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "F", M.get());

  auto * BB = llvm::BasicBlock::Create(ctx, "EntryBlock", F);
  _state = std::unique_ptr<JITStateValue>(new JITStateValue(BB, M.get()));

  // Build IR form tree recursively
  apply();

  // Return result
  _state->builder.CreateRet(_value);

  // Verification

  std::string buffer;
  llvm::raw_string_ostream es(buffer);

  if (verifyFunction(*F, &es))
    throw std::runtime_error("Function verification failed: " + es.str());

  if (verifyModule(*M, &es))
    throw std::runtime_error("Module verification failed: " + es.str());

  // Optimization

  auto machine = llvm::EngineBuilder().selectTarget();

  llvm::legacy::PassManager passes;
  passes.add(new llvm::TargetLibraryInfoWrapperPass(machine->getTargetTriple()));
  passes.add(llvm::createTargetTransformInfoWrapperPass(machine->getTargetIRAnalysis()));

  llvm::legacy::FunctionPassManager fnPasses(M.get());
  fnPasses.add(llvm::createTargetTransformInfoWrapperPass(machine->getTargetIRAnalysis()));

  auto FPM = std::make_unique<llvm::legacy::FunctionPassManager>(M.get());

  llvm::PassManagerBuilder pmb;
  pmb.OptLevel = 3;
  pmb.SizeLevel = 0;
  pmb.Inliner = llvm::createFunctionInliningPass(3, 0, false);
  pmb.LoopVectorize = true;
  pmb.SLPVectorize = true;
  machine->adjustPassManager(pmb);

  pmb.populateFunctionPassManager(fnPasses);
  pmb.populateModulePassManager(passes);

  fnPasses.doInitialization();
  // for (auto & func : *M)
  // fnPasses.run(func);
  fnPasses.run(*F);
  fnPasses.doFinalization();

  passes.add(llvm::createVerifierPass());
  passes.run(*M);

  // Compilation

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
      _value = _state->builder.CreateFRem(A, B);
      return;

    case BinaryOperatorType::POWER:
      _value = _state->builder.CreateCall(
          Intrinsic::getDeclaration(
              _state->M, llvm::Intrinsic::pow, {_state->builder.getDoubleTy()}),
          {A, B});
      return;

    case BinaryOperatorType::LOGICAL_OR:
      _value = _state->builder.CreateSelect(
          _state->builder.CreateOr(
              _state->builder.CreateFCmpONE(A, ConstantFP::get(_state->builder.getDoubleTy(), 0.0)),
              _state->builder.CreateFCmpONE(B,
                                            ConstantFP::get(_state->builder.getDoubleTy(), 0.0))),
          ConstantFP::get(_state->builder.getDoubleTy(), 0.0),
          ConstantFP::get(_state->builder.getDoubleTy(), 1.0));
      return;

    case BinaryOperatorType::LOGICAL_AND:
      _value = _state->builder.CreateSelect(
          _state->builder.CreateAnd(
              _state->builder.CreateFCmpONE(A, ConstantFP::get(_state->builder.getDoubleTy(), 0.0)),
              _state->builder.CreateFCmpONE(B,
                                            ConstantFP::get(_state->builder.getDoubleTy(), 0.0))),
          ConstantFP::get(_state->builder.getDoubleTy(), 0.0),
          ConstantFP::get(_state->builder.getDoubleTy(), 1.0));
      return;

    case BinaryOperatorType::LESS_THAN:
      _value = _state->builder.CreateSelect(_state->builder.CreateFCmpOLT(A, B),
                                            ConstantFP::get(_state->builder.getDoubleTy(), 0.0),
                                            ConstantFP::get(_state->builder.getDoubleTy(), 1.0));
      return;

    case BinaryOperatorType::GREATER_THAN:
      _value = _state->builder.CreateSelect(_state->builder.CreateFCmpOGT(A, B),
                                            ConstantFP::get(_state->builder.getDoubleTy(), 0.0),
                                            ConstantFP::get(_state->builder.getDoubleTy(), 1.0));
      return;

    case BinaryOperatorType::LESS_EQUAL:
      _value = _state->builder.CreateSelect(_state->builder.CreateFCmpOLE(A, B),
                                            ConstantFP::get(_state->builder.getDoubleTy(), 0.0),
                                            ConstantFP::get(_state->builder.getDoubleTy(), 1.0));
      return;

    case BinaryOperatorType::GREATER_EQUAL:
      _value = _state->builder.CreateSelect(_state->builder.CreateFCmpOGE(A, B),
                                            ConstantFP::get(_state->builder.getDoubleTy(), 0.0),
                                            ConstantFP::get(_state->builder.getDoubleTy(), 1.0));
      return;

    case BinaryOperatorType::EQUAL:
      _value = _state->builder.CreateSelect(_state->builder.CreateFCmpOEQ(A, B),
                                            ConstantFP::get(_state->builder.getDoubleTy(), 0.0),
                                            ConstantFP::get(_state->builder.getDoubleTy(), 1.0));
      return;

    case BinaryOperatorType::NOT_EQUAL:
      _value = _state->builder.CreateSelect(_state->builder.CreateFCmpONE(A, B),
                                            ConstantFP::get(_state->builder.getDoubleTy(), 0.0),
                                            ConstantFP::get(_state->builder.getDoubleTy(), 1.0));
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
  n->_args[0].apply(*this);

  switch (n->_type)
  {
    case UnaryFunctionType::ABS:
      func = llvm::Intrinsic::fabs;
      break;

    case UnaryFunctionType::ACOS:
      _value = _state->builder.CreateCall(_native[Native::acos], {_value});
      return;

    case UnaryFunctionType::ACOSH:
      _value = _state->builder.CreateCall(_native[Native::acosh], {_value});
      return;

      // case UnaryFunctionType::ARG:

    case UnaryFunctionType::ASIN:
      _value = _state->builder.CreateCall(_native[Native::asin], {_value});
      return;

    case UnaryFunctionType::ASINH:
      _value = _state->builder.CreateCall(_native[Native::asinh], {_value});
      return;

    case UnaryFunctionType::ATAN:
      _value = _state->builder.CreateCall(_native[Native::atan], {_value});
      return;

    case UnaryFunctionType::ATANH:
      _value = _state->builder.CreateCall(_native[Native::atanh], {_value});
      return;

    case UnaryFunctionType::CBRT:
      _value = _state->builder.CreateCall(_native[Native::cbrt], {_value});
      return;

    case UnaryFunctionType::CEIL:
      func = llvm::Intrinsic::ceil;
      break;

      // case UnaryFunctionType::CONJ:

    case UnaryFunctionType::COS:
      func = llvm::Intrinsic::cos;
      break;

    case UnaryFunctionType::COSH:
      _value = _state->builder.CreateCall(_native[Native::cosh], {_value});
      return;

    case UnaryFunctionType::COT:
      _value = _state->builder.CreateCall(_native[Native::cot], {_value});
      return;

    case UnaryFunctionType::CSC:
      _value = _state->builder.CreateCall(_native[Native::csc], {_value});
      return;

    case UnaryFunctionType::ERF:
      _value = _state->builder.CreateCall(_native[Native::erf], {_value});
      return;

    case UnaryFunctionType::ERFC:
      _value = _state->builder.CreateCall(_native[Native::erfc], {_value});
      return;

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
      _value = _state->builder.CreateCall(_native[Native::sec], {_value});
      return;

    case UnaryFunctionType::SIN:
      func = llvm::Intrinsic::sin;
      break;

    case UnaryFunctionType::SINH:
      _value = _state->builder.CreateCall(_native[Native::sinh], {_value});
      return;

    case UnaryFunctionType::SQRT:
      func = llvm::Intrinsic::sqrt;
      break;

      // case UnaryFunctionType::T:

    case UnaryFunctionType::TAN:
      _value = _state->builder.CreateCall(_native[Native::tan], {_value});
      return;

    case UnaryFunctionType::TANH:
      _value = _state->builder.CreateCall(_native[Native::tanh], {_value});
      return;

    case UnaryFunctionType::TRUNC:
      func = llvm::Intrinsic::trunc;
      break;

    default:
      fatalError("Function not implemented");
  }

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
    case BinaryFunctionType::ATAN2:
      _value = _state->builder.CreateCall(_native[Native::atan2], {A, B});
      return;

      // case BinaryFunctionType::HYPOT:

    case BinaryFunctionType::PLOG:
      _value = _state->builder.CreateCall(_native[Native::plog], {A, B});
      return;

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
  n->_args[0].apply(*this);
  const auto A = _value;
  n->_args[1].apply(*this);
  const auto B = _value;
  n->_args[2].apply(*this);
  const auto C = _value;

  _value = _state->builder.CreateSelect(
      _state->builder.CreateFCmpONE(A, ConstantFP::get(_state->builder.getDoubleTy(), 0.0)), B, C);
}

template <>
void
CompiledLLVM<Real>::operator()(IntegerPowerData<Real> * n)
{
  n->_arg.apply(*this);
  auto A = _value;

  _value = ConstantFP::get(_state->builder.getDoubleTy(), 1.0);

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
  _lljit = std::move(Builder.create().get());

  if (auto R = createHostProcessResolver(_lljit->getDataLayout()))
    _lljit->getMainJITDylib().setGenerator(std::move(R));
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
    _lljit->getExecutionSession().reportError(R.takeError());
    return nullptr;
  }

  if (!*R)
  {
    _lljit->getExecutionSession().reportError(createStringError(
        inconvertibleErrorCode(), "Generator function for host process symbols must not be null"));
    return nullptr;
  }

  return *R;
}

template <typename T>
Error
CompiledLLVM<T>::Helper::submitModule(std::unique_ptr<Module> M, std::unique_ptr<LLVMContext> C)
{
  return _lljit->addIRModule(ThreadSafeModule(std::move(M), std::move(C)));
}

template <typename T>
Expected<JITTargetAddress>
CompiledLLVM<T>::Helper::getFunctionAddr(StringRef Name)
{
  Expected<JITEvaluatedSymbol> S = _lljit->lookup(Name);
  if (!S)
    return S.takeError();

  JITTargetAddress A = S->getAddress();
  if (!A)
    return createStringError(inconvertibleErrorCode(), "'%s' evaluated to nullptr", Name.data());

  return A;
}

template class CompiledLLVM<Real>;

} // namespace SymbolicMath
