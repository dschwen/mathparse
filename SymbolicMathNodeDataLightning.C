///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#ifdef SYMBOLICMATH_USE_LIGHTNING

#include "SymbolicMathNodeData.h"
#include "SymbolicMathUtils.h"

#include <cmath>
#include <iostream> // debug

namespace SymbolicMath
{

#define _jit state.C

void
libjit_unary_function_call(JITStateValue & state, double (*func)(double))
{
  jit_finishi(reinterpret_cast<void *>(func));
  jit_retval_d(JIT_F0);
}

void
libjit_binary_function_call(JITStateValue & state, double (*func)(double, double))
{
  jit_finishi(reinterpret_cast<void *>(func));
  jit_retval_d(JIT_F0);
}

double
ligntning_wrap_min(double a, double b)
{
  return std::min(a, b);
}

double
ligntning_wrap_max(double a, double b)
{
  return std::max(a, b);
}

void
stack_push(JITStateValue & state)
{
  if (state.sp >= 0)
    jit_stxi_d(state.sp * sizeof(double) + state.stack_base, JIT_FP, JIT_F0);
  state.sp++;
}

void
stack_pop(JITStateValue & state, int reg)
{
  if (state.sp == 0)
    fatalError("Stack exhausted in stack_pop");
  state.sp--;
  jit_ldxi_d(reg, JIT_FP, state.sp * sizeof(double) + state.stack_base);
}

/********************************************************
 * Real Number immediate
 ********************************************************/

JITReturnValue
RealNumberData::jit(JITStateValue & state)
{
  stack_push(state);
  jit_movi_d(JIT_F0, _value);
}

/********************************************************
 * Real Number reference value provider
 ********************************************************/

JITReturnValue
RealReferenceData::jit(JITStateValue & state)
{
  stack_push(state);
  jit_ldi_d(JIT_F0, const_cast<void *>(reinterpret_cast<const void *>(&_ref)));
}

/********************************************************
 * Real Number Array reference value provider
 ********************************************************/

JITReturnValue
RealArrayReferenceData::jit(JITStateValue & state)
{
  // auto index = jit_insn_load_relative(
  //     state.C,
  //     jit_value_create_nint_constant(func, jit_type_void_ptr,
  //     reinterpret_cast<jit_nint>(&_index)),
  //     0,
  //     jit_type_int);
  //
  // return jit_insn_load_elem_address(
  //     func,
  //     jit_value_create_nint_constant(func, jit_type_void_ptr, reinterpret_cast<jit_nint>(&_ref)),
  //     index,
  //     jit_type_float64);
}

/********************************************************
 * Unary Operator
 ********************************************************/

JITReturnValue
UnaryOperatorData::jit(JITStateValue & state)
{
  _args[0].jit(state);

  switch (_type)
  {
    case UnaryOperatorType::PLUS:
      return;

    case UnaryOperatorType::MINUS:
      jit_negr_d(JIT_F0, JIT_F0);
      return;

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
  _args[0].jit(state);
  _args[1].jit(state);

  // Arguments A = JIT_F1, B = JIT_F0 !
  stack_pop(state, JIT_F1);

  switch (_type)
  {
    case BinaryOperatorType::SUBTRACTION:
      jit_subr_d(JIT_F0, JIT_F1, JIT_F0);
      return;

    case BinaryOperatorType::DIVISION:
      jit_divr_d(JIT_F0, JIT_F1, JIT_F0);
      return;

    case BinaryOperatorType::MODULO:
    {
      jit_prepare();
      jit_pushargr_d(JIT_F1);
      jit_pushargr_d(JIT_F0);
      libjit_binary_function_call(state, std::fmod);
      return;
    }

    case BinaryOperatorType::POWER:
    {
      jit_prepare();
      jit_pushargr_d(JIT_F1);
      jit_pushargr_d(JIT_F0);
      libjit_binary_function_call(state, std::pow);
      return;
    }

    case BinaryOperatorType::LOGICAL_OR:
      // logical or
      jit_nei_d(JIT_R0, JIT_F0, 0.0);
      jit_nei_d(JIT_R1, JIT_F1, 0.0);
      jit_orr(JIT_R0, JIT_R0, JIT_R1);
      jit_extr_d(JIT_F0, JIT_R0);
      return;

    case BinaryOperatorType::LOGICAL_AND:
      // logical and
      jit_nei_d(JIT_R0, JIT_F0, 0.0);
      jit_nei_d(JIT_R1, JIT_F1, 0.0);
      jit_andr(JIT_R0, JIT_R0, JIT_R1);
      jit_extr_d(JIT_F0, JIT_R0);
      return;

    case BinaryOperatorType::LESS_THAN:
      jit_ltr_d(JIT_R0, JIT_F1, JIT_F0);
      jit_extr_d(JIT_F0, JIT_R0);
      return;

    case BinaryOperatorType::GREATER_THAN:
      jit_gtr_d(JIT_R0, JIT_F1, JIT_F0);
      jit_extr_d(JIT_F0, JIT_R0);
      return;

    case BinaryOperatorType::LESS_EQUAL:
      jit_ler_d(JIT_R0, JIT_F1, JIT_F0);
      jit_extr_d(JIT_F0, JIT_R0);
      return;

    case BinaryOperatorType::GREATER_EQUAL:
      jit_ger_d(JIT_R0, JIT_F1, JIT_F0);
      jit_extr_d(JIT_F0, JIT_R0);
      return;

    case BinaryOperatorType::EQUAL:
      jit_eqr_d(JIT_R0, JIT_F1, JIT_F0);
      jit_extr_d(JIT_F0, JIT_R0);
      return;

    case BinaryOperatorType::NOT_EQUAL:
      jit_ner_d(JIT_R0, JIT_F1, JIT_F0);
      jit_extr_d(JIT_F0, JIT_R0);
      return;

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

  _args[0].jit(state);

  if (_args.size() > 1)
    for (std::size_t i = 1; i < _args.size(); ++i)
    {
      _args[i].jit(state);
      stack_pop(state, JIT_F1);
      switch (_type)
      {
        case MultinaryOperatorType::ADDITION:
          jit_addr_d(JIT_F0, JIT_F1, JIT_F0);
          break;

        case MultinaryOperatorType::MULTIPLICATION:
          jit_mulr_d(JIT_F0, JIT_F1, JIT_F0);
          break;

        default:
          fatalError("Unknown operator");
      }
    }
}

/********************************************************
 * Unary Function Node
 ********************************************************/

JITReturnValue
UnaryFunctionData::jit(JITStateValue & state)
{
  _args[0].jit(state);
  jit_prepare();
  jit_pushargr_d(JIT_F0);

  switch (_type)
  {
    case UnaryFunctionType::ABS:
      libjit_unary_function_call(state, std::abs);
      return;

    case UnaryFunctionType::ACOS:
      libjit_unary_function_call(state, std::acos);
      return;

    case UnaryFunctionType::ACOSH:
      libjit_unary_function_call(state, std::acosh);
      return;

    case UnaryFunctionType::ARG:
      fatalError("Function not implemented");

    case UnaryFunctionType::ASIN:
      libjit_unary_function_call(state, std::asin);
      return;

    case UnaryFunctionType::ASINH:
      libjit_unary_function_call(state, std::asinh);
      return;

    case UnaryFunctionType::ATAN:
      libjit_unary_function_call(state, std::atan);
      return;

    case UnaryFunctionType::ATANH:
      libjit_unary_function_call(state, std::atanh);
      return;

    case UnaryFunctionType::CBRT:
      libjit_unary_function_call(state, std::cbrt);
      return;

    case UnaryFunctionType::CEIL:
      libjit_unary_function_call(state, std::ceil);
      return;

    case UnaryFunctionType::CONJ:
      fatalError("Function not implemented");

    case UnaryFunctionType::COS:
      libjit_unary_function_call(state, std::cos);
      return;

    case UnaryFunctionType::COSH:
      libjit_unary_function_call(state, std::cosh);
      return;

    case UnaryFunctionType::COT:
      libjit_unary_function_call(state, std::tan);
      jit_movi_d(JIT_F1, 1.0);
      jit_divr_d(JIT_F0, JIT_F1, JIT_F0);
      return;

    case UnaryFunctionType::CSC:
      libjit_unary_function_call(state, std::sin);
      jit_movi_d(JIT_F1, 1.0);
      jit_divr_d(JIT_F0, JIT_F1, JIT_F0);
      return;

    case UnaryFunctionType::ERF:
      libjit_unary_function_call(state, std::erf);
      return;

    case UnaryFunctionType::EXP:
      libjit_unary_function_call(state, std::exp);
      return;

    case UnaryFunctionType::EXP2:
      libjit_unary_function_call(state, std::exp2);
      return;

    case UnaryFunctionType::FLOOR:
      libjit_unary_function_call(state, std::floor);
      return;

    case UnaryFunctionType::IMAG:
      fatalError("Function not implemented");

    case UnaryFunctionType::INT:
      libjit_unary_function_call(state, std::round);
      return;

    case UnaryFunctionType::LOG:
      libjit_unary_function_call(state, std::log);
      return;

    case UnaryFunctionType::LOG10:
      libjit_unary_function_call(state, std::log10);
      return;

    case UnaryFunctionType::LOG2:
      libjit_unary_function_call(state, std::log2);
      return;

    case UnaryFunctionType::REAL:
      fatalError("Function not implemented");

    case UnaryFunctionType::SEC:
      libjit_unary_function_call(state, std::cos);
      jit_movi_d(JIT_F1, 1.0);
      jit_divr_d(JIT_F0, JIT_F1, JIT_F0);
      return;

    case UnaryFunctionType::SIN:
      libjit_unary_function_call(state, std::sin);
      return;

    case UnaryFunctionType::SINH:
      libjit_unary_function_call(state, std::sinh);
      return;

    case UnaryFunctionType::SQRT:
      libjit_unary_function_call(state, std::sqrt);
      return;

    case UnaryFunctionType::T:
      fatalError("Function not implemented");

    case UnaryFunctionType::TAN:
      libjit_unary_function_call(state, std::tan);
      return;

    case UnaryFunctionType::TANH:
      libjit_unary_function_call(state, std::tanh);
      return;

    case UnaryFunctionType::TRUNC:
      jit_truncr_d_l(JIT_R0, JIT_F0);
      jit_extr_d(JIT_F0, JIT_R0);
      return;

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
  _args[0].jit(state);
  _args[1].jit(state);

  // Arguments A = JIT_F1, B = JIT_F0 !
  stack_pop(state, JIT_F1);

  jit_prepare();
  jit_pushargr_d(JIT_F1);
  jit_pushargr_d(JIT_F0);

  switch (_type)
  {
    case BinaryFunctionType::ATAN2:
      libjit_binary_function_call(state, std::atan2);
      return;

    case BinaryFunctionType::HYPOT:
      fatalError("Function not implemented");

    case BinaryFunctionType::MIN:
      libjit_binary_function_call(state, ligntning_wrap_min);
      return;

    case BinaryFunctionType::MAX:
      libjit_binary_function_call(state, ligntning_wrap_max);
      return;

    case BinaryFunctionType::PLOG:
      fatalError("Function not implemented");
      // return A < B
      //            ? std::log(B) + (A - B) / B - (A - B) * (A - B) / (2.0 * B * B) +
      //                  (A - B) * (A - B) * (A - B) / (3.0 * B * B * B)
      //            : std::log(A);

    case BinaryFunctionType::POW:
      libjit_binary_function_call(state, std::pow);
      return;

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

  // we could inspect the condition node here and do a jmp/cmp combo instruction
  _args[0].jit(state);
  jit_node_t * jump_false = jit_beqi_d(JIT_F0, 0.0);

  // true case
  auto stack_pos = state.sp;
  _args[1].jit(state);
  jit_node_t * jump_end = jit_jmpi();

  // false case
  state.sp = stack_pos;
  jit_patch(jump_false);
  _args[2].jit(state);

  // end if
  jit_patch(jump_end);
}

/********************************************************
 * Integer power Node
 ********************************************************/

JITReturnValue
IntegerPowerData::jit(JITStateValue & state)
{
  if (_exponent == 0)
    jit_movi_d(JIT_F0, 1.0);
  else
  {
    _arg.jit(state);
    int e = _exponent > 0 ? _exponent : -_exponent;
    jit_movi_d(JIT_F1, 1.0);
    while (e)
    {
      // if bit 0 is set multiply the current power of two factor of the exponent
      if (e & 1)
        jit_mulr_d(JIT_F1, JIT_F0, JIT_F1);

      // x is incrementally set to consecutive powers of powers of two
      jit_mulr_d(JIT_F0, JIT_F0, JIT_F0);

      // bit shift the exponent down
      e >>= 1;
    }
    if (_exponent > 0)
      jit_movr_d(JIT_F0, JIT_F1);
    else
    {
      jit_movi_d(JIT_F0, 1.0);
      jit_divr_d(JIT_F0, JIT_F0, JIT_F1);
    }
  }
}

} // namespace SymbolicMath

#endif // SYMBOLICMATH_USE_LIGHTNING
