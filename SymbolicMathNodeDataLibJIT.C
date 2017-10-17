#ifdef SYMBOLICMATH_USE_LIBJIT

#include "SymbolicMathNodeData.h"
#include "SymbolicMathUtils.h"

#include <cmath>
#include <iostream> // debug

namespace SymbolicMath
{

#define GLUE_HELPER(x, y) x##y
#define GLUE(x, y) GLUE_HELPER(x, y)

#define LIBJIT_MATH_WRAPPER1(FUNC)                                                                 \
  {                                                                                                \
    double (*GLUE(FUNC, _ptr))(double) = std::FUNC;                                                \
    jit_value_t GLUE(FUNC, _args)[] = {A};                                                         \
    return jit_insn_call_native(func,                                                              \
                                "std::" #FUNC,                                                     \
                                reinterpret_cast<void *>(GLUE(FUNC, _ptr)),                        \
                                signature,                                                         \
                                GLUE(FUNC, _args),                                                 \
                                1,                                                                 \
                                JIT_CALL_NOTHROW);                                                 \
  }

/********************************************************
 * Real Number immediate
 ********************************************************/

JITReturnValue
RealNumberData::jit(JITStateValue & func)
{
  return jit_value_create_float64_constant(func, jit_type_float64, (jit_float64)_value);
}

/********************************************************
 * Real Number reference value provider
 ********************************************************/

JITReturnValue
RealReferenceData::jit(JITStateValue & func)
{
  return jit_insn_load_relative(
      func,
      jit_value_create_nint_constant(func, jit_type_void_ptr, reinterpret_cast<jit_nint>(&_ref)),
      0,
      jit_type_float64);
}

/********************************************************
 * Real Number Array reference value provider
 ********************************************************/

JITReturnValue
RealArrayReferenceData::jit(JITStateValue & func)
{
  auto index = jit_insn_load_relative(
      func,
      jit_value_create_nint_constant(func, jit_type_void_ptr, reinterpret_cast<jit_nint>(&_index)),
      0,
      jit_type_int);

  return jit_insn_load_elem_address(
      func,
      jit_value_create_nint_constant(func, jit_type_void_ptr, reinterpret_cast<jit_nint>(&_ref)),
      index,
      jit_type_float64);
}

/********************************************************
 * Unary Operator
 ********************************************************/

JITReturnValue
UnaryOperatorData::jit(JITStateValue & func)
{
  switch (_type)
  {
    case UnaryOperatorType::PLUS:
      return _args[0].jit(func);

    case UnaryOperatorType::MINUS:
      return jit_insn_neg(func, _args[0].jit(func));

    default:
      fatalError("Unknown operator");
  }
}

/********************************************************
 * Binary Operator Node
 ********************************************************/

JITReturnValue
BinaryOperatorData::jit(JITStateValue & func)
{
  auto A = _args[0].jit(func);
  auto B = _args[1].jit(func);

  switch (_type)
  {
    case BinaryOperatorType::SUBTRACTION:
      return jit_insn_sub(func, A, B);

    case BinaryOperatorType::DIVISION:
      return jit_insn_div(func, A, B);

    case BinaryOperatorType::POWER:
      return jit_insn_pow(func, A, B);

    case BinaryOperatorType::LOGICAL_OR:
      // using bitwise or
      return jit_insn_or(func, A, B);

    case BinaryOperatorType::LOGICAL_AND:
      // using bitwise and
      return jit_insn_and(func, A, B);

    case BinaryOperatorType::LESS_THAN:
      return jit_insn_lt(func, A, B);

    case BinaryOperatorType::GREATER_THAN:
      return jit_insn_gt(func, A, B);

    case BinaryOperatorType::LESS_EQUAL:
      return jit_insn_le(func, A, B);

    case BinaryOperatorType::GREATER_EQUAL:
      return jit_insn_ge(func, A, B);

    case BinaryOperatorType::EQUAL:
      return jit_insn_eq(func, A, B);

    case BinaryOperatorType::NOT_EQUAL:
      return jit_insn_ne(func, A, B);

    default:
      fatalError("Unknown operator");
  }
}

/********************************************************
 * Multinary Operator Node
 ********************************************************/

JITReturnValue
MultinaryOperatorData::jit(JITStateValue & func)
{
  if (_args.size() == 0)
    fatalError("No child nodes in multinary operator");
  else if (_args.size() == 1)
    return _args[0].jit(func);
  else
  {
    JITReturnValue temp = _args[0].jit(func);
    for (std::size_t i = 1; i < _args.size(); ++i)
      switch (_type)
      {
        case MultinaryOperatorType::ADDITION:
          temp = jit_insn_add(func, temp, _args[i].jit(func));
          break;

        case MultinaryOperatorType::MULTIPLICATION:
          temp = jit_insn_mul(func, temp, _args[i].jit(func));
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
UnaryFunctionData::jit(JITStateValue & func)
{
  const auto A = _args[0].jit(func);

  // Prepare calling native_mult: create its signature
  jit_type_t params[] = {jit_type_float64};
  jit_type_t signature = jit_type_create_signature(jit_abi_cdecl, jit_type_float64, params, 1, 1);

  switch (_type)
  {
    case UnaryFunctionType::ABS:
      return jit_insn_abs(func, A);

    case UnaryFunctionType::ACOS:
      return jit_insn_acos(func, A);

    case UnaryFunctionType::ACOSH:
      fatalError("Function not implemented");

    case UnaryFunctionType::ARG:
      fatalError("Function not implemented");

    case UnaryFunctionType::ASIN:
      return jit_insn_asin(func, A);

    case UnaryFunctionType::ASINH:
      LIBJIT_MATH_WRAPPER1(asinh)

    case UnaryFunctionType::ATAN:
      return jit_insn_atan(func, A);

    case UnaryFunctionType::ATANH:
      LIBJIT_MATH_WRAPPER1(atanh)

    case UnaryFunctionType::CBRT:
      LIBJIT_MATH_WRAPPER1(cbrt)

    case UnaryFunctionType::CEIL:
      return jit_insn_ceil(func, A);

    case UnaryFunctionType::CONJ:
      fatalError("Function not implemented");

    case UnaryFunctionType::COS:
      return jit_insn_cos(func, A);

    case UnaryFunctionType::COSH:
      return jit_insn_cosh(func, A);

    case UnaryFunctionType::COT:
      return jit_insn_div(
          func,
          jit_value_create_float64_constant(func, jit_type_float64, (jit_float64)1.0),
          jit_insn_tan(func, A));

    case UnaryFunctionType::CSC:
      return jit_insn_div(
          func,
          jit_value_create_float64_constant(func, jit_type_float64, (jit_float64)1.0),
          jit_insn_sin(func, A));

    case UnaryFunctionType::ERF:
      LIBJIT_MATH_WRAPPER1(erf)

    case UnaryFunctionType::EXP:
      return jit_insn_exp(func, A);

    case UnaryFunctionType::EXP2:
      LIBJIT_MATH_WRAPPER1(exp2)

    case UnaryFunctionType::FLOOR:
      return jit_insn_floor(func, A);

    case UnaryFunctionType::IMAG:
      fatalError("Function not implemented");

    case UnaryFunctionType::INT:
      return jit_insn_round(func, A);

    case UnaryFunctionType::LOG:
      return jit_insn_log(func, A);

    case UnaryFunctionType::LOG10:
      return jit_insn_log10(func, A);

    case UnaryFunctionType::LOG2:
      LIBJIT_MATH_WRAPPER1(log2)

    case UnaryFunctionType::REAL:
      fatalError("Function not implemented");

    case UnaryFunctionType::SEC:
      return jit_insn_div(
          func,
          jit_value_create_float64_constant(func, jit_type_float64, (jit_float64)1.0),
          jit_insn_cos(func, A));

    case UnaryFunctionType::SIN:
      return jit_insn_sin(func, A);

    case UnaryFunctionType::SINH:
      return jit_insn_sinh(func, A);

    case UnaryFunctionType::SQRT:
      return jit_insn_sqrt(func, A);

    case UnaryFunctionType::T:
      fatalError("Function not implemented");

    case UnaryFunctionType::TAN:
      return jit_insn_tan(func, A);

    case UnaryFunctionType::TANH:
      return jit_insn_tan(func, A);

    case UnaryFunctionType::TRUNC:
      return jit_insn_rint(func, A);

    default:
      fatalError("Function not implemented");
  }
}

/********************************************************
 * Binary Function Node
 ********************************************************/

JITReturnValue
BinaryFunctionData::jit(JITStateValue & func)
{
  const auto A = _args[0].jit(func);
  const auto B = _args[1].jit(func);

  switch (_type)
  {
    case BinaryFunctionType::ATAN2:
      return jit_insn_atan2(func, A, B);

    case BinaryFunctionType::HYPOT:
      return jit_insn_sqrt(func,
                           jit_insn_add(func, jit_insn_mul(func, A, A), jit_insn_mul(func, B, B)));

    case BinaryFunctionType::MIN:
      return jit_insn_min(func, A, B);

    case BinaryFunctionType::MAX:
      return jit_insn_max(func, A, B);

    case BinaryFunctionType::PLOG:
    {
      fatalError("Function not implemented");
      // return A < B
      //            ? std::log(B) + (A - B) / B - (A - B) * (A - B) / (2.0 * B * B) +
      //                  (A - B) * (A - B) * (A - B) / (3.0 * B * B * B)
      //            : std::log(A);
    }

    case BinaryFunctionType::POW:
      return jit_insn_pow(func, A, B);

    case BinaryFunctionType::POLAR:
    default:
      fatalError("Function not implemented");
  }
}

/********************************************************
 * Conditional Node
 ********************************************************/

JITReturnValue
ConditionalData::jit(JITStateValue & func)
{
  if (_type != ConditionalType::IF)
    fatalError("Conditional not implemented");

  jit_label_t label1 = jit_label_undefined;
  jit_label_t label2 = jit_label_undefined;
  JITReturnValue result = jit_value_create(func, jit_type_float64);

  jit_insn_branch_if_not(func, _args[0].jit(func), &label1);
  // true branch
  jit_insn_store(func, result, _args[1].jit(func));
  jit_insn_branch(func, &label2);
  jit_insn_label(func, &label1);
  // false branch
  jit_insn_store(func, result, _args[2].jit(func));
  jit_insn_label(func, &label2);
  return jit_insn_load(func, result);
}

/********************************************************
 * Integer power Node
 ********************************************************/

JITReturnValue
IntegerPowerData::jit(JITStateValue & state)
{
  fatalError("Function not implemented");
}

// end namespace SymbolicMath
}

#endif // SYMBOLICMATH_USE_LIBJIT
