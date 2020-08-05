///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#ifdef SYMBOLICMATH_USE_LLVMIR

#include "SymbolicMathNodeData.h"
#include "SymbolicMathUtils.h"

#include "llvm/IR/Intrinsics.h"

#include <cmath>
#include <iostream> // debug

using namespace llvm;

namespace SymbolicMath
{

/********************************************************
 * Real Number immediate
 ********************************************************/

JITReturnValue
RealNumberData::jit(JITStateValue & state)
{
  return ConstantFP::get(state.builder.getDoubleTy(), _value);
}

/********************************************************
 * Real Number reference value provider
 ********************************************************/

JITReturnValue
RealReferenceData::jit(JITStateValue & state)
{
  auto adr = ConstantInt::get(state.builder.getInt64Ty(), (int64_t)&_ref);
  auto ptr = llvm::ConstantExpr::getIntToPtr(
      adr, llvm::PointerType::getUnqual(state.builder.getDoubleTy()));
  return state.builder.CreateLoad(ptr);
}

/********************************************************
 * Real Number Array reference value provider
 ********************************************************/

JITReturnValue
RealArrayReferenceData::jit(JITStateValue & state)
{
  fatalError("Not implemented yet");
}

/********************************************************
 * Unary Operator
 ********************************************************/

JITReturnValue
UnaryOperatorData::jit(JITStateValue & state)
{
  switch (_type)
  {
    case UnaryOperatorType::PLUS:
      return _args[0].jit(state);

    case UnaryOperatorType::MINUS:
      return state.builder.CreateFNeg(_args[0].jit(state));

    default:
      fatalError("Unknown operator");
  }
}

/********************************************************
 * Binary Operator Node
 ********************************************************/

JITReturnValue
BinaryOperatorData::jit(JITStateValue & state)
{
  auto A = _args[0].jit(state);
  auto B = _args[1].jit(state);

  switch (_type)
  {
    case BinaryOperatorType::SUBTRACTION:
      return state.builder.CreateFSub(A, B);

    case BinaryOperatorType::DIVISION:
      return state.builder.CreateFDiv(A, B);

    case BinaryOperatorType::MODULO:
      fatalError("Operator not implemented yet");

    case BinaryOperatorType::POWER:
      fatalError("Operator not implemented yet");

    case BinaryOperatorType::LOGICAL_OR:
      return state.builder.CreateOr(A, B); //?

    case BinaryOperatorType::LOGICAL_AND:
      return state.builder.CreateAnd(A, B); //?

    case BinaryOperatorType::LESS_THAN:
      return state.builder.CreateFCmpOLT(A, B);

    case BinaryOperatorType::GREATER_THAN:
      return state.builder.CreateFCmpOGT(A, B);

    case BinaryOperatorType::LESS_EQUAL:
      return state.builder.CreateFCmpOLE(A, B);

    case BinaryOperatorType::GREATER_EQUAL:
      return state.builder.CreateFCmpOGE(A, B);

    case BinaryOperatorType::EQUAL:
      return state.builder.CreateFCmpOEQ(A, B);

    case BinaryOperatorType::NOT_EQUAL:
      return state.builder.CreateFCmpONE(A, B);

    default:
      fatalError("Unknown operator");
  }
}

/********************************************************
 * Multinary Operator Node
 ********************************************************/

JITReturnValue
MultinaryOperatorData::jit(JITStateValue & state)
{
  if (_args.size() == 0)
    fatalError("No child nodes in multinary operator");
  else if (_args.size() == 1)
    return _args[0].jit(state);
  else
  {
    JITReturnValue temp = _args[0].jit(state);
    for (std::size_t i = 1; i < _args.size(); ++i)
      switch (_type)
      {
        case MultinaryOperatorType::ADDITION:
          temp = state.builder.CreateFAdd(temp, _args[i].jit(state));
          break;

        case MultinaryOperatorType::MULTIPLICATION:
          temp = state.builder.CreateFMul(temp, _args[i].jit(state));
          break;

        default:
          fatalError("Unknown operator");
      }
    return temp;
  }
}

/********************************************************
 * Unary Function Node
 ********************************************************/

JITReturnValue
UnaryFunctionData::jit(JITStateValue & state)
{
  std::vector<Value *> args = {_args[0].jit(state)};
  llvm::Intrinsic::ID func;

  switch (_type)
  {
    case UnaryFunctionType::ABS:
      func = llvm::Intrinsic::fabs;
      break;

    case UnaryFunctionType::ACOS:

    case UnaryFunctionType::ACOSH:
      fatalError("Function not implemented");

    case UnaryFunctionType::ARG:
      fatalError("Function not implemented");

    case UnaryFunctionType::ASIN:

    case UnaryFunctionType::ASINH:

    case UnaryFunctionType::ATAN:

    case UnaryFunctionType::ATANH:

    case UnaryFunctionType::CBRT:

    case UnaryFunctionType::CEIL:
      func = llvm::Intrinsic::ceil;
      break;

    case UnaryFunctionType::CONJ:
      fatalError("Function not implemented");

    case UnaryFunctionType::COS:
      func = llvm::Intrinsic::cos;
      break;

    case UnaryFunctionType::COSH:

    case UnaryFunctionType::COT:
      // return jit_insn_div(
      //     func,
      //     jit_value_create_float64_constant(func, jit_type_float64, (jit_float64)1.0),
      //     jit_insn_tan(func, A));

    case UnaryFunctionType::CSC:
      // return jit_insn_div(
      //     func,
      //     jit_value_create_float64_constant(func, jit_type_float64, (jit_float64)1.0),
      //     jit_insn_sin(func, A));

    case UnaryFunctionType::ERF:

    case UnaryFunctionType::EXP:
      func = llvm::Intrinsic::exp;
      break;

    case UnaryFunctionType::EXP2:
      func = llvm::Intrinsic::exp2;
      break;

    case UnaryFunctionType::FLOOR:
      func = llvm::Intrinsic::floor;
      break;

    case UnaryFunctionType::IMAG:
      fatalError("Function not implemented");

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

    case UnaryFunctionType::REAL:
      fatalError("Function not implemented");

    case UnaryFunctionType::SEC:
      // return jit_insn_div(
      //     func,
      //     jit_value_create_float64_constant(func, jit_type_float64, (jit_float64)1.0),
      //     jit_insn_cos(func, A));

    case UnaryFunctionType::SIN:
      func = llvm::Intrinsic::sin;
      break;

    case UnaryFunctionType::SINH:

    case UnaryFunctionType::SQRT:
      func = llvm::Intrinsic::sqrt;
      break;

    case UnaryFunctionType::T:

    case UnaryFunctionType::TAN:

    case UnaryFunctionType::TANH:

    case UnaryFunctionType::TRUNC:
      func = llvm::Intrinsic::trunc;
      break;

    default:
      fatalError("Function not implemented");
  }

  return state.builder.CreateCall(
      Intrinsic::getDeclaration(state.M, func, {state.builder.getDoubleTy()}), args);
}

/********************************************************
 * Binary Function Node
 ********************************************************/

JITReturnValue
BinaryFunctionData::jit(JITStateValue & state)
{
  std::vector<Value *> args = {_args[0].jit(state), _args[1].jit(state)};

  switch (_type)
  {
    case BinaryFunctionType::ATAN2:

    case BinaryFunctionType::HYPOT:
      // return jit_insn_sqrt(func,
      //                      jit_insn_add(func, jit_insn_mul(func, A, A), jit_insn_mul(func, B,
      //                      B)));

    case BinaryFunctionType::MIN:

    case BinaryFunctionType::MAX:

    case BinaryFunctionType::PLOG:
    {
      fatalError("Function not implemented");
      // return A < B
      //            ? std::log(B) + (A - B) / B - (A - B) * (A - B) / (2.0 * B * B) +
      //                  (A - B) * (A - B) * (A - B) / (3.0 * B * B * B)
      //            : std::log(A);
    }

    case BinaryFunctionType::POW:
      // return jit_insn_pow(func, A, B);

    case BinaryFunctionType::POLAR:
    default:
      fatalError("Function not implemented");
  }
}

/********************************************************
 * Conditional Node
 ********************************************************/

JITReturnValue
ConditionalData::jit(JITStateValue & state)
{
  // if (_type != ConditionalType::IF)
  fatalError("Conditional not implemented");

  // jit_label_t label1 = jit_label_undefined;
  // jit_label_t label2 = jit_label_undefined;
  // JITReturnValue result = jit_value_create(func, jit_type_float64);
  //
  // jit_insn_branch_if_not(func, _args[0].jit(state), &label1);
  // // true branch
  // jit_insn_store(func, result, _args[1].jit(state));
  // jit_insn_branch(func, &label2);
  // jit_insn_label(func, &label1);
  // // false branch
  // jit_insn_store(func, result, _args[2].jit(state));
  // jit_insn_label(func, &label2);
  // return jit_insn_load(func, result);
}

/********************************************************
 * Integer power Node
 ********************************************************/

JITReturnValue
IntegerPowerData::jit(JITStateValue & state)
{
  Value * result = ConstantFP::get(state.builder.getDoubleTy(), 1.0);

  Value * A = _arg.jit(state);
  int e = _exponent > 0 ? _exponent : -_exponent;
  while (e)
  {
    // if bit 0 is set multiply the current power of two factor of the exponent
    if (e & 1)
      result = state.builder.CreateFMul(result, A);

    // x is incrementally set to consecutive powers of powers of two
    A = state.builder.CreateFMul(A, A);

    // bit shift the exponent down
    e >>= 1;
  }

  if (_exponent >= 0)
    return result;
  else
    return state.builder.CreateFDiv(ConstantFP::get(state.builder.getDoubleTy(), 1.0), result);
}

} // namespace SymbolicMath

#endif // SYMBOLICMATH_USE_LLVMIR
