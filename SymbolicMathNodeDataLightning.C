#ifdef SYMBOLICMATH_USE_SLJIT

#include "SymbolicMathNodeData.h"
#include "SymbolicMathUtils.h"

#include <cmath>
#include <iostream> // debug

namespace SymbolicMath
{

#define _jit state.C

void
stack_push(JITStateValue & state)
{
  if (sp >= 0)
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
  jit_ldi_d(JIT_F0, &_ref);
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

    case BinaryOperatorType::POWER:
      sljit_emit_op1(state.C, SLJIT_MOV, SLJIT_R0, 0, SLJIT_IMM, (sljit_sw)(state.stack - 1));
      sljit_emit_op1(state.C, SLJIT_MOV, SLJIT_R1, 0, SLJIT_IMM, (sljit_sw)state.stack);
      sljit_emit_ijump(state.C, SLJIT_CALL2, SLJIT_IMM, SLJIT_FUNC_OFFSET(sljit_wrap_pow));
      return;

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
      jit_ger_d(JIT_R0, JIT_F1, JIT_F0);
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
      jit_finishi(std::abs<double>);
      jit_retval_d(JIT_F0);
      return;

    case UnaryFunctionType::ACOS:
      jit_finishi(std::acos<double>);
      jit_retval_d(JIT_F0);
      return;

    case UnaryFunctionType::ACOSH:
      fatalError("Function not implemented");

    case UnaryFunctionType::ARG:
      fatalError("Function not implemented");

    case UnaryFunctionType::ASIN:
      jit_finishi(std::asin<double>);
      jit_retval_d(JIT_F0);
      return;

    case UnaryFunctionType::ASINH:
      fatalError("Function not implemented");

    case UnaryFunctionType::ATAN:
      jit_finishi(std::atan<double>);
      jit_retval_d(JIT_F0);
      return;

    case UnaryFunctionType::ATANH:
      fatalError("Function not implemented");

    case UnaryFunctionType::CBRT:
      jit_finishi(std::cbrt<double>);
      jit_retval_d(JIT_F0);
      return;

    case UnaryFunctionType::CEIL:
      jit_finishi(std::ceil<double>);
      jit_retval_d(JIT_F0);
      return;

    case UnaryFunctionType::CONJ:
      fatalError("Function not implemented");

    case UnaryFunctionType::COS:
      jit_finishi(std::cos<double>);
      jit_retval_d(JIT_F0);
      return;

    case UnaryFunctionType::COSH:
      jit_finishi(std::cosh<double>);
      jit_retval_d(JIT_F0);
      return;

    case UnaryFunctionType::COT:
      jit_finishi(std::tan<double>);
      jit_retval_d(JIT_F0);
      jit_movi_d(JIT_F1, 1.0);
      jit_divr_d(JIT_F0, JIT_F1, JIT_F0);
      return;

    case UnaryFunctionType::CSC:
      jit_finishi(std::sin<double>);
      jit_retval_d(JIT_F0);
      jit_movi_d(JIT_F1, 1.0);
      jit_divr_d(JIT_F0, JIT_F1, JIT_F0);
      return;

    case UnaryFunctionType::ERF:
      jit_finishi(std::erf<double>);
      jit_retval_d(JIT_F0);
      return;

    case UnaryFunctionType::EXP:
      jit_finishi(std::exp<double>);
      jit_retval_d(JIT_F0);
      return;

    case UnaryFunctionType::EXP2:
      fatalError("Function not implemented");

    case UnaryFunctionType::FLOOR:
      jit_finishi(std::floor<double>);
      jit_retval_d(JIT_F0);
      return;

    case UnaryFunctionType::IMAG:
      fatalError("Function not implemented");

    case UnaryFunctionType::INT:
      fatalError("Function not implemented");

    case UnaryFunctionType::LOG:
      jit_finishi(std::log<double>);
      jit_retval_d(JIT_F0);
      return;

    case UnaryFunctionType::LOG10:
      jit_finishi(std::log10<double>);
      jit_retval_d(JIT_F0);
      return;

    case UnaryFunctionType::LOG2:
      fatalError("Function not implemented");

    case UnaryFunctionType::REAL:
      fatalError("Function not implemented");

    case UnaryFunctionType::SEC:
      jit_finishi(std::cos<double>);
      jit_movi_d(JIT_F1, 1.0);
      jit_divr_d(JIT_F0, JIT_F1, JIT_F0);
      return;

    case UnaryFunctionType::SIN:
      jit_finishi(std::sin<double>);
      jit_retval_d(JIT_F0);
      return;

    case UnaryFunctionType::SINH:
      jit_finishi(std::sinh<double>);
      jit_retval_d(JIT_F0);
      return;

    case UnaryFunctionType::SQRT:
      jit_finishi(std::sqrt<double>);
      jit_retval_d(JIT_F0);
      return;

    case UnaryFunctionType::T:
      fatalError("Function not implemented");

    case UnaryFunctionType::TAN:
      jit_finishi(std::tan<double>);
      jit_retval_d(JIT_F0);
      return;

    case UnaryFunctionType::TANH:
      jit_finishi(std::tanh<double>);
      jit_retval_d(JIT_F0);
      return;

    case UnaryFunctionType::TRUNC:
      fatalError("Function not implemented");

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
      jit_finishi(std::atan2<double>);
      jit_retval_d(JIT_F0);
      return;

    case BinaryFunctionType::HYPOT:
      fatalError("Function not implemented");

    case BinaryFunctionType::MIN:
      jit_finishi(std::min<double>);
      jit_retval_d(JIT_F0);
      return;

    case BinaryFunctionType::MAX:
      jit_finishi(std::max<double>);
      jit_retval_d(JIT_F0);
      return;

    case BinaryFunctionType::PLOG:
      fatalError("Function not implemented");
    // return A < B
    //            ? std::log(B) + (A - B) / B - (A - B) * (A - B) / (2.0 * B * B) +
    //                  (A - B) * (A - B) * (A - B) / (3.0 * B * B * B)
    //            : std::log(A);

    case BinaryFunctionType::POW:
      jit_finishi(std::pow<double>);
      jit_retval_d(JIT_F0);
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
  // if (_type != ConditionalType::IF)
    fatalError("Conditional not implemented");

    // struct sljit_jump * false_case;
    // struct sljit_jump * end_if;
    //
    // sljit_emit_op1(state.C, SLJIT_MOV, SLJIT_FR0, 0, SLJIT_MEM, (sljit_sw)state.stack);
    // state.stack--; //?
    // false_case = sljit_emit_cmp(state.C, SLJIT_EQUAL, SLJIT_R0, 0, SLJIT_IMM, 0);
    //
    // // true case`
    // _args[0].jit(state);
    // end_if = sljit_emit_jump(state.C, SLJIT_JUMP);
    //
    // // false case
    // sljit_set_label(false_case, sljit_emit_label(state.C));
    // _args[1].jit(state);
    //
    // // end if
    // sljit_set_label(end_if, sljit_emit_label(state.C));
}

// end namespace SymbolicMath
}

#endif // SYMBOLICMATH_USE_SLJIT
