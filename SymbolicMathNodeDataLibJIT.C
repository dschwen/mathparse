///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#ifdef SYMBOLICMATH_USE_LIBJIT

#include "SymbolicMathNodeData.h"
#include "SymbolicMathUtils.h"

#include <cmath>
#include <iostream> // debug

namespace SymbolicMath
{

jit_value_t
libjit_unary_function_call(JITStateValue & state, double (*func)(double), jit_value_t A)
{
  // Prepare calling native_mult: create its signature
  jit_type_t params[] = {jit_type_float64};
  jit_type_t signature = jit_type_create_signature(jit_abi_cdecl, jit_type_float64, params, 1, 1);

  // set arguments and call function
  jit_value_t args[] = {A};
  return jit_insn_call_native(
      state, "", reinterpret_cast<void *>(func), signature, args, 1, JIT_CALL_NOTHROW);
}

/********************************************************
 * Real Number immediate
 ********************************************************/

JITReturnValue
RealNumberData::jit(JITStateValue & state)
{
  return jit_value_create_float64_constant(state, jit_type_float64, (jit_float64)_value);
}

/********************************************************
 * Real Number reference value provider
 ********************************************************/

JITReturnValue
RealReferenceData::jit(JITStateValue & state)
{
  return jit_insn_load_relative(
      state,
      jit_value_create_nint_constant(state, jit_type_void_ptr, reinterpret_cast<jit_nint>(&_ref)),
      0,
      jit_type_float64);
}

/********************************************************
 * Real Number Array reference value provider
 ********************************************************/

JITReturnValue
RealArrayReferenceData::jit(JITStateValue & state)
{
  auto index = jit_insn_load_relative(
      state,
      jit_value_create_nint_constant(state, jit_type_void_ptr, reinterpret_cast<jit_nint>(&_index)),
      0,
      jit_type_int);

  return jit_insn_load_elem_address(
      state,
      jit_value_create_nint_constant(state, jit_type_void_ptr, reinterpret_cast<jit_nint>(&_ref)),
      index,
      jit_type_float64);
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
      return jit_insn_neg(state, _args[0].jit(state));

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
      return jit_insn_sub(state, A, B);

    case BinaryOperatorType::DIVISION:
      return jit_insn_div(state, A, B);

    case BinaryOperatorType::MODULO:
    {
      jit_type_t params[] = {jit_type_float64, jit_type_float64};
      jit_type_t signature =
          jit_type_create_signature(jit_abi_cdecl, jit_type_float64, params, 2, 1);
      jit_value_t args[] = {A, B};
      double (*func)(double, double) = std::fmod;
      return jit_insn_call_native(
          state, "", reinterpret_cast<void *>(func), signature, args, 2, JIT_CALL_NOTHROW);
    }

    case BinaryOperatorType::POWER:
      return jit_insn_pow(state, A, B);

    case BinaryOperatorType::LOGICAL_OR:
    {
      auto iA = jit_insn_to_bool(state, A);
      auto iB = jit_insn_to_bool(state, B);
      return jit_insn_or(state, iA, iB);
    }

    case BinaryOperatorType::LOGICAL_AND:
    {
      auto iA = jit_insn_to_bool(state, A);
      auto iB = jit_insn_to_bool(state, B);
      return jit_insn_and(state, iA, iB);
    }

    case BinaryOperatorType::LESS_THAN:
      return jit_insn_lt(state, A, B);

    case BinaryOperatorType::GREATER_THAN:
      return jit_insn_gt(state, A, B);

    case BinaryOperatorType::LESS_EQUAL:
      return jit_insn_le(state, A, B);

    case BinaryOperatorType::GREATER_EQUAL:
      return jit_insn_ge(state, A, B);

    case BinaryOperatorType::EQUAL:
      return jit_insn_eq(state, A, B);

    case BinaryOperatorType::NOT_EQUAL:
      return jit_insn_ne(state, A, B);

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
          temp = jit_insn_add(state, temp, _args[i].jit(state));
          break;

        case MultinaryOperatorType::MULTIPLICATION:
          temp = jit_insn_mul(state, temp, _args[i].jit(state));
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
  const auto A = _args[0].jit(state);

  switch (_type)
  {
    case UnaryFunctionType::ABS:
      return jit_insn_abs(state, A);

    case UnaryFunctionType::ACOS:
      return jit_insn_acos(state, A);

    case UnaryFunctionType::ACOSH:
      return libjit_unary_function_call(state, std::acosh, A);

    case UnaryFunctionType::ARG:
      fatalError("Function not implemented");

    case UnaryFunctionType::ASIN:
      return jit_insn_asin(state, A);

    case UnaryFunctionType::ASINH:
      return libjit_unary_function_call(state, std::asinh, A);

    case UnaryFunctionType::ATAN:
      return jit_insn_atan(state, A);

    case UnaryFunctionType::ATANH:
      return libjit_unary_function_call(state, std::atanh, A);

    case UnaryFunctionType::CBRT:
      return libjit_unary_function_call(state, std::cbrt, A);

    case UnaryFunctionType::CEIL:
      return jit_insn_ceil(state, A);

    case UnaryFunctionType::CONJ:
      fatalError("Function not implemented");

    case UnaryFunctionType::COS:
      return jit_insn_cos(state, A);

    case UnaryFunctionType::COSH:
      return jit_insn_cosh(state, A);

    case UnaryFunctionType::COT:
      return jit_insn_div(
          state,
          jit_value_create_float64_constant(state, jit_type_float64, (jit_float64)1.0),
          jit_insn_tan(state, A));

    case UnaryFunctionType::CSC:
      return jit_insn_div(
          state,
          jit_value_create_float64_constant(state, jit_type_float64, (jit_float64)1.0),
          jit_insn_sin(state, A));

    case UnaryFunctionType::ERF:
      return libjit_unary_function_call(state, std::erf, A);

    case UnaryFunctionType::EXP:
      return jit_insn_exp(state, A);

    case UnaryFunctionType::EXP2:
      return libjit_unary_function_call(state, std::exp2, A);

    case UnaryFunctionType::FLOOR:
      return jit_insn_floor(state, A);

    case UnaryFunctionType::IMAG:
      fatalError("Function not implemented");

    case UnaryFunctionType::INT:
      return jit_insn_round(state, A);

    case UnaryFunctionType::LOG:
      return jit_insn_log(state, A);

    case UnaryFunctionType::LOG10:
      return jit_insn_log10(state, A);

    case UnaryFunctionType::LOG2:
      return libjit_unary_function_call(state, std::log2, A);

    case UnaryFunctionType::REAL:
      fatalError("Function not implemented");

    case UnaryFunctionType::SEC:
      return jit_insn_div(
          state,
          jit_value_create_float64_constant(state, jit_type_float64, (jit_float64)1.0),
          jit_insn_cos(state, A));

    case UnaryFunctionType::SIN:
      return jit_insn_sin(state, A);

    case UnaryFunctionType::SINH:
      return jit_insn_sinh(state, A);

    case UnaryFunctionType::SQRT:
      return jit_insn_sqrt(state, A);

    case UnaryFunctionType::T:
      fatalError("Function not implemented");

    case UnaryFunctionType::TAN:
      return jit_insn_tan(state, A);

    case UnaryFunctionType::TANH:
      return jit_insn_tanh(state, A);

    case UnaryFunctionType::TRUNC:
      return jit_insn_convert(
          state, jit_insn_convert(state, A, jit_type_int, 0), jit_type_float64, 0);

    default:
      fatalError("Function not implemented");
  }
}

/********************************************************
 * Binary Function Node
 ********************************************************/

JITReturnValue
BinaryFunctionData::jit(JITStateValue & state)
{
  const auto A = _args[0].jit(state);
  const auto B = _args[1].jit(state);

  switch (_type)
  {
    case BinaryFunctionType::ATAN2:
      return jit_insn_atan2(state, A, B);

    case BinaryFunctionType::HYPOT:
      return jit_insn_sqrt(
          state, jit_insn_add(state, jit_insn_mul(state, A, A), jit_insn_mul(state, B, B)));

    case BinaryFunctionType::MIN:
      return jit_insn_min(state, A, B);

    case BinaryFunctionType::MAX:
      return jit_insn_max(state, A, B);

    case BinaryFunctionType::PLOG:
    {
      fatalError("Function not implemented");
      // return A < B
      //            ? std::log(B) + (A - B) / B - (A - B) * (A - B) / (2.0 * B * B) +
      //                  (A - B) * (A - B) * (A - B) / (3.0 * B * B * B)
      //            : std::log(A);
    }

    case BinaryFunctionType::POW:
      return jit_insn_pow(state, A, B);

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
  if (_type != ConditionalType::IF)
    fatalError("Conditional not implemented");

  jit_label_t label1 = jit_label_undefined;
  jit_label_t label2 = jit_label_undefined;
  JITReturnValue result = jit_value_create(state, jit_type_float64);

  jit_insn_branch_if_not(state, _args[0].jit(state), &label1);
  // true branch
  jit_insn_store(state, result, _args[1].jit(state));
  jit_insn_branch(state, &label2);
  jit_insn_label(state, &label1);
  // false branch
  jit_insn_store(state, result, _args[2].jit(state));
  jit_insn_label(state, &label2);
  return jit_insn_load(state, result);
}

/********************************************************
 * Integer power Node
 ********************************************************/

JITReturnValue
IntegerPowerData::jit(JITStateValue & state)
{
  auto result = jit_value_create_float64_constant(state, jit_type_float64, (jit_float64)1.0);

  auto A = _arg.jit(state);
  int e = _exponent > 0 ? _exponent : -_exponent;
  while (e)
  {
    // if bit 0 is set multiply the current power of two factor of the exponent
    if (e & 1)
      result = jit_insn_mul(state, result, A);

    // x is incrementally set to consecutive powers of powers of two
    A = jit_insn_mul(state, A, A);

    // bit shift the exponent down
    e >>= 1;
  }

  if (_exponent >= 0)
    return result;
  else
    return jit_insn_div(
        state,
        jit_value_create_float64_constant(state, jit_type_float64, (jit_float64)1.0),
        result);
}

} // namespace SymbolicMath

#endif // SYMBOLICMATH_USE_LIBJIT
