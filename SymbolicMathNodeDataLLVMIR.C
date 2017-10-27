#ifdef SYMBOLICMATH_USE_LLVMIR

#include "SymbolicMathNodeData.h"
#include "SymbolicMathUtils.h"

#include <cmath>
#include <iostream> // debug

using namespace llvm;

namespace SymbolicMath
{

/********************************************************
 * Real Number immediate
 ********************************************************/

JITReturnValue
RealNumberData::jit(JITStateValue & builder)
{
  return ConstantFP::get(builder.getDoubleTy(), _value);
}

/********************************************************
 * Real Number reference value provider
 ********************************************************/

JITReturnValue
RealReferenceData::jit(JITStateValue & builder)
{
  auto adr = ConstantInt::get(builder.getInt64Ty(), (int64_t)&_ref);
  return ConstantExpr::getIntToPtr(adr, PointerType::getUnqual(builder.getInt64Ty()));
}

/********************************************************
 * Real Number Array reference value provider
 ********************************************************/

JITReturnValue
RealArrayReferenceData::jit(JITStateValue & builder)
{
  return nullptr;
}

/********************************************************
 * Unary Operator
 ********************************************************/

JITReturnValue
UnaryOperatorData::jit(JITStateValue & builder)
{
  switch (_type)
  {
    case UnaryOperatorType::PLUS:
      return _args[0].jit(builder);

    case UnaryOperatorType::MINUS:
      return builder.CreateFNeg(_args[0].jit(builder));

    default:
      fatalError("Unknown operator");
  }
}

/********************************************************
 * Binary Operator Node
 ********************************************************/

JITReturnValue
BinaryOperatorData::jit(JITStateValue & builder)
{
  auto A = _args[0].jit(builder);
  auto B = _args[1].jit(builder);

  switch (_type)
  {
    case BinaryOperatorType::SUBTRACTION:
      return builder.CreateFSub(A, B);

    case BinaryOperatorType::DIVISION:
      return builder.CreateFDiv(A, B);

    case BinaryOperatorType::MODULO:
      fatalError("Operator not implemented yet");

    case BinaryOperatorType::POWER:
      fatalError("Operator not implemented yet");

    case BinaryOperatorType::LOGICAL_OR:
      return builder.CreateOr(A, B); //?

    case BinaryOperatorType::LOGICAL_AND:
      return builder.CreateAnd(A, B); //?

    case BinaryOperatorType::LESS_THAN:
      return builder.CreateFCmpOLT(A, B);

    case BinaryOperatorType::GREATER_THAN:
      return builder.CreateFCmpOGT(A, B);

    case BinaryOperatorType::LESS_EQUAL:
      return builder.CreateFCmpOLE(A, B);

    case BinaryOperatorType::GREATER_EQUAL:
      return builder.CreateFCmpOGE(A, B);

    case BinaryOperatorType::EQUAL:
      return builder.CreateFCmpOEQ(A, B);

    case BinaryOperatorType::NOT_EQUAL:
      return builder.CreateFCmpONE(A, B);

    default:
      fatalError("Unknown operator");
  }
}

/********************************************************
 * Multinary Operator Node
 ********************************************************/

JITReturnValue
MultinaryOperatorData::jit(JITStateValue & builder)
{
  if (_args.size() == 0)
    fatalError("No child nodes in multinary operator");
  else if (_args.size() == 1)
    return _args[0].jit(builder);
  else
  {
    JITReturnValue temp = _args[0].jit(builder);
    for (std::size_t i = 1; i < _args.size(); ++i)
      switch (_type)
      {
        case MultinaryOperatorType::ADDITION:
          temp = builder.CreateFAdd(temp, _args[i].jit(builder));
          break;

        case MultinaryOperatorType::MULTIPLICATION:
          temp = builder.CreateFMul(temp, _args[i].jit(builder));
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
UnaryFunctionData::jit(JITStateValue & builder)
{
  std::vector<Value *> args = {_args[0].jit(builder)};

  switch (_type)
  {
    case UnaryFunctionType::ABS:
    // return jit_insn_abs(func, A);

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

    case UnaryFunctionType::CONJ:
      fatalError("Function not implemented");

    case UnaryFunctionType::COS:

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

    case UnaryFunctionType::EXP2:

    case UnaryFunctionType::FLOOR:

    case UnaryFunctionType::IMAG:
      fatalError("Function not implemented");

    case UnaryFunctionType::INT:

    case UnaryFunctionType::LOG:

    case UnaryFunctionType::LOG10:

    case UnaryFunctionType::LOG2:

    case UnaryFunctionType::REAL:
      fatalError("Function not implemented");

    case UnaryFunctionType::SEC:
    // return jit_insn_div(
    //     func,
    //     jit_value_create_float64_constant(func, jit_type_float64, (jit_float64)1.0),
    //     jit_insn_cos(func, A));

    case UnaryFunctionType::SIN:

    case UnaryFunctionType::SINH:

    case UnaryFunctionType::SQRT:

    case UnaryFunctionType::T:

    case UnaryFunctionType::TAN:

    case UnaryFunctionType::TANH:

    case UnaryFunctionType::TRUNC:
    // return jit_insn_convert(
    //     func, jit_insn_convert(func, A, jit_type_int, 0), jit_type_float64, 0);

    default:
      fatalError("Function not implemented");
  }
}

/********************************************************
 * Binary Function Node
 ********************************************************/

JITReturnValue
BinaryFunctionData::jit(JITStateValue & builder)
{
  std::vector<Value *> args = {_args[0].jit(builder), _args[1].jit(builder)};

  switch (_type)
  {
    case BinaryFunctionType::ATAN2:

    case BinaryFunctionType::HYPOT:
    // return jit_insn_sqrt(func,
    //                      jit_insn_add(func, jit_insn_mul(func, A, A), jit_insn_mul(func, B, B)));

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
ConditionalData::jit(JITStateValue & builder)
{
  // if (_type != ConditionalType::IF)
  fatalError("Conditional not implemented");

  // jit_label_t label1 = jit_label_undefined;
  // jit_label_t label2 = jit_label_undefined;
  // JITReturnValue result = jit_value_create(func, jit_type_float64);
  //
  // jit_insn_branch_if_not(func, _args[0].jit(builder), &label1);
  // // true branch
  // jit_insn_store(func, result, _args[1].jit(builder));
  // jit_insn_branch(func, &label2);
  // jit_insn_label(func, &label1);
  // // false branch
  // jit_insn_store(func, result, _args[2].jit(builder));
  // jit_insn_label(func, &label2);
  // return jit_insn_load(func, result);
}

/********************************************************
 * Integer power Node
 ********************************************************/

JITReturnValue
IntegerPowerData::jit(JITStateValue & builder)
{
  fatalError("Not implemented yet");
  // auto result = jit_value_create_float64_constant(func, jit_type_float64, (jit_float64)1.0);
  //
  // auto A = _arg.jit(builder);
  // int e = _exponent > 0 ? _exponent : -_exponent;
  // while (e)
  // {
  //   // if bit 0 is set multiply the current power of two factor of the exponent
  //   if (e & 1)
  //     result = jit_insn_mul(func, result, A);
  //
  //   // x is incrementally set to consecutive powers of powers of two
  //   A = jit_insn_mul(func, A, A);
  //
  //   // bit shift the exponent down
  //   e >>= 1;
  // }
  //
  // if (_exponent >= 0)
  //   return result;
  // else
  //   return jit_insn_div(
  //       func, jit_value_create_float64_constant(func, jit_type_float64, (jit_float64)1.0),
  //       result);
}

// end namespace SymbolicMath
}

#endif // SYMBOLICMATH_USE_LLVMIR
