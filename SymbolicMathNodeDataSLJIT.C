#ifdef SYMBOLICMATH_USE_SLJIT

#include "SymbolicMathNodeData.h"
#include "SymbolicMathUtils.h"

#include <cmath>
#include <iostream> // debug

namespace SymbolicMath
{

const double sljit_one = 1.0;
const double sljit_zero = 0.0;

void
sljit_unary_function_call(struct sljit_compiler * C, double (*func)(double))
{
  sljit_emit_icall(C,
                   SLJIT_CALL_CDECL,
                   SLJIT_RET(F64) | SLJIT_ARG1(F64),
                   SLJIT_IMM,
                   reinterpret_cast<sljit_sw>(func));
}

void
sljit_binary_function_call(struct sljit_compiler * C, double (*func)(double, double))
{
  sljit_emit_icall(C,
                   SLJIT_CALL_CDECL,
                   SLJIT_RET(F64) | SLJIT_ARG1(F64) | SLJIT_ARG2(F64),
                   SLJIT_IMM,
                   reinterpret_cast<sljit_sw>(func));
}

void
sljit_fcmp_wrapper(JITStateValue & state, sljit_s32 op)
{
  struct sljit_jump * true_lbl = sljit_emit_fcmp(state.C, op, SLJIT_FR0, 0, SLJIT_FR1, 0);

  // false case
  // put 0.0 on stack
  sljit_emit_fop1(state.C, SLJIT_MOV_F64, SLJIT_FR0, 0, SLJIT_MEM, (sljit_sw)&sljit_zero);
  struct sljit_jump * out_lbl = sljit_emit_jump(state.C, SLJIT_JUMP);

  // true case
  sljit_set_label(true_lbl, sljit_emit_label(state.C));

  // put 1.0 on stack
  sljit_emit_fop1(state.C, SLJIT_MOV_F64, SLJIT_FR0, 0, SLJIT_MEM, (sljit_sw)&sljit_one);

  // end if
  sljit_set_label(out_lbl, sljit_emit_label(state.C));
}

double
sljit_trunc_wrapper(double A)
{
  return static_cast<int>(A);
}

void
stack_push(JITStateValue & state)
{
  if (state.sp >= 0)
    sljit_emit_fop1(
        state.C, SLJIT_MOV_F64, SLJIT_MEM1(SLJIT_SP), state.sp * sizeof(double), SLJIT_FR0, 0);
  state.sp++;
}

void
stack_pop(JITStateValue & state, sljit_s32 op)
{
  if (state.sp == 0)
    fatalError("Stack exhausted in stack_pop");
  state.sp--;
  sljit_emit_fop1(state.C, SLJIT_MOV_F64, op, 0, SLJIT_MEM1(SLJIT_SP), state.sp * sizeof(double));
}

/********************************************************
 * Real Number immediate
 ********************************************************/

JITReturnValue
RealNumberData::jit(JITStateValue & state)
{
  // sljit does not have any 64bit floating point immediates, so we need to make a mem->register
  // transfer this makes the JIT code point to data in the expression tree! When the tree gets
  // simplified the node holding this data may be freed. We therefore need to invalidate the JIT
  // code upon simplification!
  stack_push(state);
  sljit_emit_fop1(state.C, SLJIT_MOV_F64, SLJIT_FR0, 0, SLJIT_MEM, (sljit_sw)&_value);
}

/********************************************************
 * Real Number reference value provider
 ********************************************************/

JITReturnValue
RealReferenceData::jit(JITStateValue & state)
{
  stack_push(state);
  sljit_emit_fop1(state.C, SLJIT_MOV_F64, SLJIT_FR0, 0, SLJIT_MEM, (sljit_sw)&_ref);
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
      sljit_emit_fop1(state.C, SLJIT_NEG_F64, SLJIT_FR0, 0, SLJIT_FR0, 0);
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

  // Arguments A = SLJIT_FR0, B = SLJIT_FR1
  sljit_emit_fop1(state.C, SLJIT_MOV_F64, SLJIT_FR1, 0, SLJIT_FR0, 0);
  stack_pop(state, SLJIT_FR0);

  switch (_type)
  {
    case BinaryOperatorType::SUBTRACTION:
      sljit_emit_fop2(state.C, SLJIT_SUB_F64, SLJIT_FR0, 0, SLJIT_FR0, 0, SLJIT_FR1, 0);
      return;

    case BinaryOperatorType::DIVISION:
      sljit_emit_fop2(state.C, SLJIT_DIV_F64, SLJIT_FR0, 0, SLJIT_FR0, 0, SLJIT_FR1, 0);
      return;

    case BinaryOperatorType::MODULO:
      sljit_binary_function_call(state.C, std::fmod);
      return;

    case BinaryOperatorType::POWER:
      sljit_binary_function_call(state.C, std::pow);
      return;

    case BinaryOperatorType::LOGICAL_OR:
    {
      sljit_emit_fop1(state.C, SLJIT_MOV_F64, SLJIT_FR2, 0, SLJIT_MEM, (sljit_sw)&sljit_zero);
      // either argument is true -> true
      struct sljit_jump * true_lbl1 =
          sljit_emit_fcmp(state.C, SLJIT_NOT_EQUAL_F64, SLJIT_FR0, 0, SLJIT_FR2, 0);
      struct sljit_jump * true_lbl2 =
          sljit_emit_fcmp(state.C, SLJIT_NOT_EQUAL_F64, SLJIT_FR1, 0, SLJIT_FR2, 0);

      // false case
      // put 0.0 on stack
      sljit_emit_fop1(state.C, SLJIT_MOV_F64, SLJIT_FR0, 0, SLJIT_FR2, 0);
      struct sljit_jump * out_lbl = sljit_emit_jump(state.C, SLJIT_JUMP);

      // true case
      sljit_set_label(true_lbl1, sljit_emit_label(state.C));
      sljit_set_label(true_lbl2, sljit_emit_label(state.C));

      // put 1.0 on stack
      sljit_emit_fop1(state.C, SLJIT_MOV_F64, SLJIT_FR0, 0, SLJIT_MEM, (sljit_sw)&sljit_one);

      // end if
      sljit_set_label(out_lbl, sljit_emit_label(state.C));

      return;
    }

    case BinaryOperatorType::LOGICAL_AND:
    {
      sljit_emit_fop1(state.C, SLJIT_MOV_F64, SLJIT_FR2, 0, SLJIT_MEM, (sljit_sw)&sljit_zero);
      // either argument is false -> false
      struct sljit_jump * false_lbl1 =
          sljit_emit_fcmp(state.C, SLJIT_EQUAL_F64, SLJIT_FR0, 0, SLJIT_FR2, 0);
      struct sljit_jump * false_lbl2 =
          sljit_emit_fcmp(state.C, SLJIT_EQUAL_F64, SLJIT_FR1, 0, SLJIT_FR2, 0);

      // true case
      // put 1.0 on stack
      sljit_emit_fop1(state.C, SLJIT_MOV_F64, SLJIT_FR0, 0, SLJIT_MEM, (sljit_sw)&sljit_one);
      struct sljit_jump * out_lbl = sljit_emit_jump(state.C, SLJIT_JUMP);

      // false case
      sljit_set_label(false_lbl1, sljit_emit_label(state.C));
      sljit_set_label(false_lbl2, sljit_emit_label(state.C));

      // put 0.0 on stack
      sljit_emit_fop1(state.C, SLJIT_MOV_F64, SLJIT_FR0, 0, SLJIT_FR2, 0);

      // end if
      sljit_set_label(out_lbl, sljit_emit_label(state.C));

      return;
    }

    case BinaryOperatorType::LESS_THAN:
      sljit_fcmp_wrapper(state, SLJIT_LESS_F64);
      return;

    case BinaryOperatorType::GREATER_THAN:
      sljit_fcmp_wrapper(state, SLJIT_GREATER_F64);
      return;

    case BinaryOperatorType::LESS_EQUAL:
      sljit_fcmp_wrapper(state, SLJIT_LESS_EQUAL_F64);
      return;

    case BinaryOperatorType::GREATER_EQUAL:
      sljit_fcmp_wrapper(state, SLJIT_GREATER_EQUAL_F64);
      return;

    case BinaryOperatorType::EQUAL:
      sljit_fcmp_wrapper(state, SLJIT_EQUAL_F64);
      return;

    case BinaryOperatorType::NOT_EQUAL:
      sljit_fcmp_wrapper(state, SLJIT_NOT_EQUAL_F64);
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
      stack_pop(state, SLJIT_FR1);
      switch (_type)
      {
        case MultinaryOperatorType::ADDITION:
          sljit_emit_fop2(state.C, SLJIT_ADD_F64, SLJIT_FR0, 0, SLJIT_FR0, 0, SLJIT_FR1, 0);
          break;

        case MultinaryOperatorType::MULTIPLICATION:
          sljit_emit_fop2(state.C, SLJIT_MUL_F64, SLJIT_FR0, 0, SLJIT_FR0, 0, SLJIT_FR1, 0);
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

  switch (_type)
  {
    case UnaryFunctionType::ABS:
      sljit_emit_fop1(state.C, SLJIT_ABS_F64, SLJIT_FR0, 0, SLJIT_FR0, 0);
      return;

    case UnaryFunctionType::ACOS:
      sljit_unary_function_call(state.C, std::acos);
      return;

    case UnaryFunctionType::ACOSH:
      sljit_unary_function_call(state.C, std::acosh);
      return;

    case UnaryFunctionType::ARG:
      fatalError("Function not implemented");

    case UnaryFunctionType::ASIN:
      sljit_unary_function_call(state.C, std::asin);
      return;

    case UnaryFunctionType::ASINH:
      sljit_unary_function_call(state.C, std::asinh);
      return;

    case UnaryFunctionType::ATAN:
      sljit_unary_function_call(state.C, std::atan);
      return;

    case UnaryFunctionType::ATANH:
      sljit_unary_function_call(state.C, std::atanh);
      return;

    case UnaryFunctionType::CBRT:
      sljit_unary_function_call(state.C, std::cbrt);
      return;

    case UnaryFunctionType::CEIL:
      sljit_unary_function_call(state.C, std::ceil);
      return;

    case UnaryFunctionType::CONJ:
      fatalError("Function not implemented");

    case UnaryFunctionType::COS:
      sljit_unary_function_call(state.C, std::cos);
      return;

    case UnaryFunctionType::COSH:
      sljit_unary_function_call(state.C, std::cosh);
      return;

    case UnaryFunctionType::COT:
      sljit_unary_function_call(state.C, std::tan);
      sljit_emit_fop2(
          state.C, SLJIT_DIV_F64, SLJIT_FR0, 0, SLJIT_MEM, (sljit_sw)&sljit_one, SLJIT_FR0, 0);
      return;

    case UnaryFunctionType::CSC:
      sljit_unary_function_call(state.C, std::sin);
      sljit_emit_fop2(
          state.C, SLJIT_DIV_F64, SLJIT_FR0, 0, SLJIT_MEM, (sljit_sw)&sljit_one, SLJIT_FR0, 0);
      return;

    case UnaryFunctionType::ERF:
      sljit_unary_function_call(state.C, std::erf);
      return;

    case UnaryFunctionType::EXP:
      sljit_unary_function_call(state.C, std::exp);
      return;

    case UnaryFunctionType::EXP2:
      sljit_unary_function_call(state.C, std::exp2);
      return;

    case UnaryFunctionType::FLOOR:
      sljit_unary_function_call(state.C, std::floor);
      return;

    case UnaryFunctionType::IMAG:
      fatalError("Function not implemented");

    case UnaryFunctionType::INT:
      sljit_unary_function_call(state.C, std::round);
      return;

    case UnaryFunctionType::LOG:
      sljit_unary_function_call(state.C, std::log);
      return;

    case UnaryFunctionType::LOG10:
      sljit_unary_function_call(state.C, std::log10);
      return;

    case UnaryFunctionType::LOG2:
      sljit_unary_function_call(state.C, std::log2);
      return;

    case UnaryFunctionType::REAL:
      fatalError("Function not implemented");

    case UnaryFunctionType::SEC:
      sljit_unary_function_call(state.C, std::cos);
      sljit_emit_fop2(
          state.C, SLJIT_DIV_F64, SLJIT_FR0, 0, SLJIT_MEM, (sljit_sw)&sljit_one, SLJIT_FR0, 0);
      return;

    case UnaryFunctionType::SIN:
      sljit_unary_function_call(state.C, std::sin);
      return;

    case UnaryFunctionType::SINH:
      sljit_unary_function_call(state.C, std::sinh);
      return;

    case UnaryFunctionType::SQRT:
      sljit_unary_function_call(state.C, std::sqrt);
      return;

    case UnaryFunctionType::T:
      fatalError("Function not implemented");

    case UnaryFunctionType::TAN:
      sljit_unary_function_call(state.C, std::tan);
      return;

    case UnaryFunctionType::TANH:
      sljit_unary_function_call(state.C, std::tanh);
      return;

    case UnaryFunctionType::TRUNC:
      sljit_unary_function_call(state.C, sljit_trunc_wrapper);
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

  // Arguments A = SLJIT_FR0, B = SLJIT_FR1
  sljit_emit_fop1(state.C, SLJIT_MOV_F64, SLJIT_FR1, 0, SLJIT_FR0, 0);
  stack_pop(state, SLJIT_FR0);

  switch (_type)
  {
    case BinaryFunctionType::ATAN2:
      sljit_binary_function_call(state.C, std::atan2);
      return;

    case BinaryFunctionType::HYPOT:
      // return jit_insn_sqrt(func,
      //                      jit_insn_add(func, jit_insn_mul(func, A, A), jit_insn_mul(func, B,
      //                      B)));
      fatalError("Function not implemented");

    case BinaryFunctionType::MIN:
    {
      struct sljit_jump * out_lbl =
          sljit_emit_fcmp(state.C, SLJIT_LESS_F64, SLJIT_FR0, 0, SLJIT_FR1, 0);

      // FR0 >= FR1 case
      // put FR1 on stack
      sljit_emit_fop1(state.C, SLJIT_MOV_F64, SLJIT_FR0, 0, SLJIT_FR1, 0);

      // else jump here and leave FR0
      sljit_set_label(out_lbl, sljit_emit_label(state.C));

      return;
    }

    case BinaryFunctionType::MAX:
    {
      struct sljit_jump * out_lbl =
          sljit_emit_fcmp(state.C, SLJIT_GREATER_F64, SLJIT_FR0, 0, SLJIT_FR1, 0);

      // FR0 >= FR1 case
      // put FR1 on stack
      sljit_emit_fop1(state.C, SLJIT_MOV_F64, SLJIT_FR0, 0, SLJIT_FR1, 0);

      // else jump here and leave FR0
      sljit_set_label(out_lbl, sljit_emit_label(state.C));

      return;
    }

    case BinaryFunctionType::PLOG:
      fatalError("Function not implemented");
      // return A < B
      //            ? std::log(B) + (A - B) / B - (A - B) * (A - B) / (2.0 * B * B) +
      //                  (A - B) * (A - B) * (A - B) / (3.0 * B * B * B)
      //            : std::log(A);

    case BinaryFunctionType::POW:
      sljit_binary_function_call(state.C, std::pow);
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

  struct sljit_jump * false_case;
  struct sljit_jump * end_if;

  _args[0].jit(state);

  // sljit_emit_op1(state.C, SLJIT_MOV, SLJIT_R0, 0, SLJIT_MEM, (sljit_sw)state.stack);
  false_case =
      sljit_emit_fcmp(state.C, SLJIT_EQUAL_F64, SLJIT_FR0, 0, SLJIT_MEM, (sljit_sw)&sljit_zero);

  // true case
  auto stack_pos = state.sp;
  _args[1].jit(state);
  end_if = sljit_emit_jump(state.C, SLJIT_JUMP);

  // false case
  state.sp = stack_pos;
  sljit_set_label(false_case, sljit_emit_label(state.C));
  _args[2].jit(state);

  // end if
  sljit_set_label(end_if, sljit_emit_label(state.C));
}

/********************************************************
 * Integer power Node
 ********************************************************/

JITReturnValue
IntegerPowerData::jit(JITStateValue & state)
{
  if (_exponent == 0)
  {
    // this case should be simplified away and never reached
    sljit_emit_fop1(state.C, SLJIT_MOV_F64, SLJIT_FR0, 0, SLJIT_MEM, (sljit_sw)&sljit_one);
    return;
  }

  // FR0 = A
  _arg.jit(state);

  // FR1 = FR2 = 1.0
  sljit_emit_fop1(state.C, SLJIT_MOV_F64, SLJIT_FR2, 0, SLJIT_MEM, (sljit_sw)&sljit_one);
  sljit_emit_fop1(state.C, SLJIT_MOV_F64, SLJIT_FR1, 0, SLJIT_FR2, 0);

  int e = _exponent > 0 ? _exponent : -_exponent;
  while (e)
  {
    // if bit 0 is set multiply the current power of two factor of the exponent
    if (e & 1)
      sljit_emit_fop2(state.C, SLJIT_MUL_F64, SLJIT_FR1, 0, SLJIT_FR0, 0, SLJIT_FR1, 0);

    // x is incrementally set to consecutive powers of powers of two
    sljit_emit_fop2(state.C, SLJIT_MUL_F64, SLJIT_FR0, 0, SLJIT_FR0, 0, SLJIT_FR0, 0);

    // bit shift the exponent down
    e >>= 1;
  }

  if (_exponent >= 0)
    sljit_emit_fop1(state.C, SLJIT_MOV_F64, SLJIT_FR0, 0, SLJIT_FR1, 0);
  else
  {
    sljit_emit_fop1(state.C, SLJIT_MOV_F64, SLJIT_FR0, 0, SLJIT_FR2, 0); // FR0 = 1.0
    sljit_emit_fop2(state.C, SLJIT_DIV_F64, SLJIT_FR0, 0, SLJIT_FR0, 0, SLJIT_FR1, 0);
  }
}

} // namespace SymbolicMath

#endif // SYMBOLICMATH_USE_SLJIT
