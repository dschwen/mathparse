#ifdef SYMBOLICMATH_USE_SLJIT

#include "SymbolicMathNodeData.h"
#include "SymbolicMathUtils.h"

#include <cmath>
#include <iostream> // debug

namespace SymbolicMath
{

#define GLUE_HELPER(x, y) x##y
#define GLUE(x, y) GLUE_HELPER(x, y)

#define SLJIT_MATH_WRAPPER1(FUNC)                                                                  \
  static long SLJIT_CALL GLUE(sljit_wrap_, FUNC)(long a)                                           \
  {                                                                                                \
    auto & A = *reinterpret_cast<double *>(a);                                                     \
    A = std::FUNC(A);                                                                              \
    return 0;                                                                                      \
  }

#define SLJIT_MATH_WRAPPER1X(FUNC, EXPR)                                                           \
  static long SLJIT_CALL GLUE(sljit_wrap_, FUNC)(long a)                                           \
  {                                                                                                \
    auto & A = *reinterpret_cast<double *>(a);                                                     \
    A = EXPR;                                                                                      \
    return 0;                                                                                      \
  }

#define SLJIT_MATH_WRAPPER2(FUNC)                                                                  \
  static long SLJIT_CALL GLUE(sljit_wrap_, FUNC)(long a, long b)                                   \
  {                                                                                                \
    auto & A = *reinterpret_cast<double *>(a);                                                     \
    auto & B = *reinterpret_cast<double *>(b);                                                     \
    A = std::FUNC(A, B);                                                                           \
    return 0;                                                                                      \
  }

SLJIT_MATH_WRAPPER1(abs)
SLJIT_MATH_WRAPPER1(acos)
SLJIT_MATH_WRAPPER1(acosh)
SLJIT_MATH_WRAPPER1(asin)
SLJIT_MATH_WRAPPER1(asinh)
SLJIT_MATH_WRAPPER1(atan)
SLJIT_MATH_WRAPPER1(atanh)
SLJIT_MATH_WRAPPER1(cbrt)
SLJIT_MATH_WRAPPER1(ceil)
SLJIT_MATH_WRAPPER1(cos)
SLJIT_MATH_WRAPPER1(cosh)
SLJIT_MATH_WRAPPER1(erf)
SLJIT_MATH_WRAPPER1(exp)
SLJIT_MATH_WRAPPER1(exp2)
SLJIT_MATH_WRAPPER1(floor)
SLJIT_MATH_WRAPPER1(log)
SLJIT_MATH_WRAPPER1(log10)
SLJIT_MATH_WRAPPER1(log2)
SLJIT_MATH_WRAPPER1(round)
SLJIT_MATH_WRAPPER1(sin)
SLJIT_MATH_WRAPPER1(sinh)
SLJIT_MATH_WRAPPER1(sqrt)
SLJIT_MATH_WRAPPER1(tan)
SLJIT_MATH_WRAPPER1(tanh)
SLJIT_MATH_WRAPPER1X(sec, 1.0 / std::cos(A))
SLJIT_MATH_WRAPPER1X(csc, 1.0 / std::sin(A))
SLJIT_MATH_WRAPPER1X(cot, 1.0 / std::tan(A))
SLJIT_MATH_WRAPPER1X(int, A < 0 ? std::ceil(A - 0.5) : std::floor(A + 0.5))
SLJIT_MATH_WRAPPER1X(trunc, static_cast<int>(A))

SLJIT_MATH_WRAPPER2(atan2)
SLJIT_MATH_WRAPPER2(max)
SLJIT_MATH_WRAPPER2(min)
SLJIT_MATH_WRAPPER2(pow)

void
emit_sljit_fop2(JITStateValue & state, sljit_s32 op)
{
  sljit_emit_fop2(state.C,
                  op,
                  SLJIT_MEM,
                  (sljit_sw)(&(state.stack) - 1),
                  SLJIT_MEM,
                  (sljit_sw)(&(state.stack) - 1),
                  SLJIT_MEM,
                  (sljit_sw) & (state.stack));
}

/********************************************************
 * Real Number immediate
 ********************************************************/

JITReturnValue
RealNumberData::jit(JITStateValue & state)
{
  // sljit does not have any 64bit floating point immediates, so we need to make a mem->mem transfer
  // this makes teh JIT code point to data in the expression tree! When the tree
  // gets simplified the node holding this data may be freed. We therefore need to
  // invalidate the JIT code upon simplification!
  sljit_emit_fop1(
      state.C, SLJIT_MOV_F64, SLJIT_MEM, (sljit_sw) & (state.stack), SLJIT_MEM, (sljit_sw)&_value);
  state.stack++;
}

/********************************************************
 * Real Number reference value provider
 ********************************************************/

JITReturnValue
RealReferenceData::jit(JITStateValue & state)
{
  sljit_emit_fop1(
      state.C, SLJIT_MOV_F64, SLJIT_MEM, (sljit_sw) & (state.stack), SLJIT_MEM, (sljit_sw)&_ref);
  state.stack++;
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
      sljit_emit_fop1(state.C,
                      SLJIT_NEG_F64,
                      SLJIT_MEM,
                      (sljit_sw)(&(state.stack) - 1),
                      SLJIT_MEM,
                      (sljit_sw)(&(state.stack) - 1));
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
  state.stack--;

  switch (_type)
  {
    case BinaryOperatorType::SUBTRACTION:
      emit_sljit_fop2(state, SLJIT_SUB_F64);
      return;

    case BinaryOperatorType::DIVISION:
      emit_sljit_fop2(state, SLJIT_DIV_F64);
      return;

    case BinaryOperatorType::POWER:
      sljit_emit_op1(state.C, SLJIT_MOV, SLJIT_R0, 0, SLJIT_IMM, (sljit_sw)(&(state.stack) - 1));
      sljit_emit_op1(state.C, SLJIT_MOV, SLJIT_R1, 0, SLJIT_IMM, (sljit_sw) & (state.stack));
      sljit_emit_ijump(state.C, SLJIT_CALL2, SLJIT_IMM, SLJIT_FUNC_OFFSET(sljit_wrap_pow));
      return;

    case BinaryOperatorType::LOGICAL_OR:
      // bitwise (integer) or
      sljit_emit_op2(state.C,
                     SLJIT_OR,
                     SLJIT_MEM,
                     (sljit_sw)(&(state.stack) - 1),
                     SLJIT_MEM,
                     (sljit_sw)(&(state.stack) - 1),
                     SLJIT_MEM,
                     (sljit_sw) & (state.stack));
      return;

    case BinaryOperatorType::LOGICAL_AND:
      // bitwise (integer) and
      sljit_emit_op2(state.C,
                     SLJIT_AND,
                     SLJIT_MEM,
                     (sljit_sw)(&(state.stack) - 1),
                     SLJIT_MEM,
                     (sljit_sw)(&(state.stack) - 1),
                     SLJIT_MEM,
                     (sljit_sw) & (state.stack));
      return;

    case BinaryOperatorType::LESS_THAN:
      emit_sljit_fop2(state, SLJIT_LESS_F64);
      return;

    case BinaryOperatorType::GREATER_THAN:
      emit_sljit_fop2(state, SLJIT_GREATER_F64);
      return;

    case BinaryOperatorType::LESS_EQUAL:
      emit_sljit_fop2(state, SLJIT_LESS_EQUAL_F64);
      return;

    case BinaryOperatorType::GREATER_EQUAL:
      emit_sljit_fop2(state, SLJIT_GREATER_EQUAL_F64);
      return;

    case BinaryOperatorType::EQUAL:
      emit_sljit_fop2(state, SLJIT_EQUAL_F64);
      return;

    case BinaryOperatorType::NOT_EQUAL:
      emit_sljit_fop2(state, SLJIT_NOT_EQUAL_F64);
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
      state.stack--;
      switch (_type)
      {
        case MultinaryOperatorType::ADDITION:
          emit_sljit_fop2(state, SLJIT_ADD_F64);
          break;

        case MultinaryOperatorType::MULTIPLICATION:
          emit_sljit_fop2(state, SLJIT_MUL_F64);
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
  sljit_emit_op1(state.C, SLJIT_MOV, SLJIT_R0, 0, SLJIT_IMM, (sljit_sw) & (state.stack));

  switch (_type)
  {
    case UnaryFunctionType::ABS:
      sljit_emit_ijump(state.C, SLJIT_CALL1, SLJIT_IMM, SLJIT_FUNC_OFFSET(sljit_wrap_abs));
      return;

    case UnaryFunctionType::ACOS:
      sljit_emit_ijump(state.C, SLJIT_CALL1, SLJIT_IMM, SLJIT_FUNC_OFFSET(sljit_wrap_acos));
      return;

    case UnaryFunctionType::ACOSH:
      fatalError("Function not implemented");

    case UnaryFunctionType::ARG:
      fatalError("Function not implemented");

    case UnaryFunctionType::ASIN:
      sljit_emit_ijump(state.C, SLJIT_CALL1, SLJIT_IMM, SLJIT_FUNC_OFFSET(sljit_wrap_asin));
      return;

    case UnaryFunctionType::ASINH:
      fatalError("Function not implemented");

    case UnaryFunctionType::ATAN:
      sljit_emit_ijump(state.C, SLJIT_CALL1, SLJIT_IMM, SLJIT_FUNC_OFFSET(sljit_wrap_atan));
      return;

    case UnaryFunctionType::ATANH:
      fatalError("Function not implemented");

    case UnaryFunctionType::CBRT:
      sljit_emit_ijump(state.C, SLJIT_CALL1, SLJIT_IMM, SLJIT_FUNC_OFFSET(sljit_wrap_cbrt));
      return;

    case UnaryFunctionType::CEIL:
      sljit_emit_ijump(state.C, SLJIT_CALL1, SLJIT_IMM, SLJIT_FUNC_OFFSET(sljit_wrap_ceil));
      return;

    case UnaryFunctionType::CONJ:
      fatalError("Function not implemented");

    case UnaryFunctionType::COS:
      sljit_emit_ijump(state.C, SLJIT_CALL1, SLJIT_IMM, SLJIT_FUNC_OFFSET(sljit_wrap_cos));
      return;

    case UnaryFunctionType::COSH:
      sljit_emit_ijump(state.C, SLJIT_CALL1, SLJIT_IMM, SLJIT_FUNC_OFFSET(sljit_wrap_cosh));
      return;

    case UnaryFunctionType::COT:
      sljit_emit_ijump(state.C, SLJIT_CALL1, SLJIT_IMM, SLJIT_FUNC_OFFSET(sljit_wrap_cosh));
      sljit_emit_fop2(state.C,
                      SLJIT_DIV_F64,
                      SLJIT_MEM,
                      (sljit_sw)(&(state.stack) - 1),
                      SLJIT_IMM,
                      (sljit_f64)1.0, // OOPS!
                      SLJIT_MEM,
                      (sljit_sw)(&(state.stack) - 1));
      return;

    case UnaryFunctionType::CSC:
      // return jit_insn_div(
      //     func,
      //     jit_value_create_float64_constant(func, jit_type_float64, (jit_float64)1.0),
      //     jit_insn_sin(func, A));
      fatalError("Function not implemented");

    case UnaryFunctionType::ERF:
      // use jit_insn_call_native here!
      fatalError("Function not implemented");

    case UnaryFunctionType::EXP:
      sljit_emit_ijump(state.C, SLJIT_CALL1, SLJIT_IMM, SLJIT_FUNC_OFFSET(sljit_wrap_exp));
      return;

    case UnaryFunctionType::EXP2:
      // return jit_insn_pow(
      //     func, jit_value_create_float64_constant(func, jit_type_float64, (jit_float64)2.0), A);
      fatalError("Function not implemented");

    case UnaryFunctionType::FLOOR:
      sljit_emit_ijump(state.C, SLJIT_CALL1, SLJIT_IMM, SLJIT_FUNC_OFFSET(sljit_wrap_floor));
      return;

    case UnaryFunctionType::IMAG:
      fatalError("Function not implemented");

    case UnaryFunctionType::INT:
      sljit_emit_ijump(state.C, SLJIT_CALL1, SLJIT_IMM, SLJIT_FUNC_OFFSET(sljit_wrap_round));
      return;

    case UnaryFunctionType::LOG:
      sljit_emit_ijump(state.C, SLJIT_CALL1, SLJIT_IMM, SLJIT_FUNC_OFFSET(sljit_wrap_log));
      return;

    case UnaryFunctionType::LOG10:
      sljit_emit_ijump(state.C, SLJIT_CALL1, SLJIT_IMM, SLJIT_FUNC_OFFSET(sljit_wrap_log10));
      return;

    case UnaryFunctionType::LOG2:
      fatalError("Function not implemented");

    case UnaryFunctionType::REAL:
      fatalError("Function not implemented");

    case UnaryFunctionType::SEC:
      // return jit_insn_div(
      //     func,
      //     jit_value_create_float64_constant(func, jit_type_float64, (jit_float64)1.0),
      //     jit_insn_cos(func, A));
      fatalError("Function not implemented");

    case UnaryFunctionType::SIN:
      sljit_emit_ijump(state.C, SLJIT_CALL1, SLJIT_IMM, SLJIT_FUNC_OFFSET(sljit_wrap_sin));
      return;

    case UnaryFunctionType::SINH:
      sljit_emit_ijump(state.C, SLJIT_CALL1, SLJIT_IMM, SLJIT_FUNC_OFFSET(sljit_wrap_sinh));
      return;

    case UnaryFunctionType::SQRT:
      sljit_emit_ijump(state.C, SLJIT_CALL1, SLJIT_IMM, SLJIT_FUNC_OFFSET(sljit_wrap_sqrt));
      return;

    case UnaryFunctionType::T:
      fatalError("Function not implemented");

    case UnaryFunctionType::TAN:
      sljit_emit_ijump(state.C, SLJIT_CALL1, SLJIT_IMM, SLJIT_FUNC_OFFSET(sljit_wrap_tan));
      return;

    case UnaryFunctionType::TANH:
      sljit_emit_ijump(state.C, SLJIT_CALL1, SLJIT_IMM, SLJIT_FUNC_OFFSET(sljit_wrap_tanh));
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
  state.stack--;

  sljit_emit_op1(state.C, SLJIT_MOV, SLJIT_R0, 0, SLJIT_IMM, (sljit_sw)(&(state.stack) - 1));
  sljit_emit_op1(state.C, SLJIT_MOV, SLJIT_R1, 0, SLJIT_IMM, (sljit_sw) & (state.stack));

  switch (_type)
  {
    case BinaryFunctionType::ATAN2:
      sljit_emit_ijump(state.C, SLJIT_CALL2, SLJIT_IMM, SLJIT_FUNC_OFFSET(sljit_wrap_atan2));
      return;

    case BinaryFunctionType::HYPOT:
      // return jit_insn_sqrt(func,
      //                      jit_insn_add(func, jit_insn_mul(func, A, A), jit_insn_mul(func, B,
      //                      B)));
      fatalError("Function not implemented");

    case BinaryFunctionType::MIN:
      sljit_emit_ijump(state.C, SLJIT_CALL2, SLJIT_IMM, SLJIT_FUNC_OFFSET(sljit_wrap_min));
      return;

    case BinaryFunctionType::MAX:
      sljit_emit_ijump(state.C, SLJIT_CALL2, SLJIT_IMM, SLJIT_FUNC_OFFSET(sljit_wrap_max));
      return;

    case BinaryFunctionType::PLOG:
      fatalError("Function not implemented");
    // return A < B
    //            ? std::log(B) + (A - B) / B - (A - B) * (A - B) / (2.0 * B * B) +
    //                  (A - B) * (A - B) * (A - B) / (3.0 * B * B * B)
    //            : std::log(A);

    case BinaryFunctionType::POW:
      sljit_emit_ijump(state.C, SLJIT_CALL2, SLJIT_IMM, SLJIT_FUNC_OFFSET(sljit_wrap_pow));
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

  sljit_emit_op1(state.C, SLJIT_MOV, SLJIT_FR0, 0, SLJIT_MEM, (sljit_sw) & (state.stack));
  state.stack--; //?
  false_case = sljit_emit_cmp(state.C, SLJIT_EQUAL, SLJIT_R0, 0, SLJIT_IMM, 0);

  // true case`
  _args[0].jit(state);
  end_if = sljit_emit_jump(state.C, SLJIT_JUMP);

  // false case
  sljit_set_label(false_case, sljit_emit_label(state.C));
  _args[1].jit(state);

  // end if
  sljit_set_label(end_if, sljit_emit_label(state.C));
}

// end namespace SymbolicMath
}

#endif // SYMBOLICMATH_USE_SLJIT
