///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include "SMFunction.h"
#include "SMCompiledSLJIT.h"
#include "SMCompilerFactory.h"

namespace SymbolicMath
{

registerCompiler(CompiledSLJIT, "CompiledSLJIT", Real, 20);

const double sljit_one = 1.0;
const double sljit_zero = 0.0;

template <typename T>
CompiledSLJIT<T>::CompiledSLJIT(Function<T> & fb) : Transform<T>(fb), _jit_function(nullptr)
{
  // determine required stack size
  auto current_max = std::make_pair(0, 0);
  fb.root().stackDepth(current_max);
  if (current_max.first <= 0)
    fatalError("Stack depleted at function end");

  // build function
  _ctx = sljit_create_compiler(NULL, NULL);
  sljit_emit_enter(_ctx, 0, SLJIT_ARGS0(F64), 4, 0, 4, 0, current_max.second * sizeof(T));

  // initialize stack pointer
  _sp = -1;

  // build function from expression tree
  apply();

  // return stack top (FR0)
  sljit_emit_return(_ctx, SLJIT_MOV_F64, SLJIT_FR0, 0);

  // generate machine code
  _jit_function = reinterpret_cast<JITFunctionPtr>(sljit_generate_code(_ctx));

  // free the compiler data
  sljit_free_compiler(_ctx);
}

template <typename T>
CompiledSLJIT<T>::~CompiledSLJIT()
{
  if (_jit_function)
    sljit_free_code(reinterpret_cast<void *>(_jit_function), nullptr);
}

// Helper methods

template <>
void
CompiledSLJIT<Real>::stackPush()
{
  if (_sp >= 0)
    sljit_emit_fop1(_ctx, SLJIT_MOV_F64, SLJIT_MEM1(SLJIT_SP), _sp * sizeof(double), SLJIT_FR0, 0);
  _sp++;
}

template <>
void
CompiledSLJIT<Real>::stackPop(sljit_s32 op)
{
  if (_sp == 0)
    fatalError("Stack exhausted in stack_pop");
  _sp--;
  sljit_emit_fop1(_ctx, SLJIT_MOV_F64, op, 0, SLJIT_MEM1(SLJIT_SP), _sp * sizeof(double));
}

template <typename T>
void
CompiledSLJIT<T>::unaryFunctionCall(T (*func)(T))
{
  sljit_emit_icall(
      _ctx, SLJIT_CALL, SLJIT_ARGS1(F64, F64), SLJIT_IMM, reinterpret_cast<sljit_sw>(func));
}

template <typename T>
void
CompiledSLJIT<T>::binaryFunctionCall(T (*func)(T, T))
{
  sljit_emit_icall(
      _ctx, SLJIT_CALL, SLJIT_ARGS2(F64, F64, F64), SLJIT_IMM, reinterpret_cast<sljit_sw>(func));
}

template <typename T>
void
CompiledSLJIT<T>::emitFcmp(sljit_s32 op)
{
  struct sljit_jump * true_lbl = sljit_emit_fcmp(_ctx, op, SLJIT_FR0, 0, SLJIT_FR1, 0);

  // false case
  // put 0.0 on stack
  sljit_emit_fop1(_ctx, SLJIT_MOV_F64, SLJIT_FR0, 0, SLJIT_MEM, (sljit_sw)&sljit_zero);
  struct sljit_jump * out_lbl = sljit_emit_jump(_ctx, SLJIT_JUMP);

  // true case
  sljit_set_label(true_lbl, sljit_emit_label(_ctx));

  // put 1.0 on stack
  sljit_emit_fop1(_ctx, SLJIT_MOV_F64, SLJIT_FR0, 0, SLJIT_MEM, (sljit_sw)&sljit_one);

  // end if
  sljit_set_label(out_lbl, sljit_emit_label(_ctx));
}

template <typename T>
T
CompiledSLJIT<T>::truncWrapper(T a)
{
  return static_cast<int>(a);
}

template <typename T>
T
CompiledSLJIT<T>::plog(T a, T b)
{
  return a < b ? std::log(b) + (a - b) / b - (a - b) * (a - b) / (2.0 * b * b) +
                     (a - b) * (a - b) * (a - b) / (3.0 * b * b * b)
               : std::log(a);
}

// Visitor operators

template <typename T>
void
CompiledSLJIT<T>::operator()(Node<T> & node, SymbolData<T> & data)
{
  fatalError("Symbol in compiled function");
}

template <typename T>
void
CompiledSLJIT<T>::operator()(Node<T> & node, UnaryOperatorData<T> & data)
{
  data._args[0].apply(*this);

  switch (data._type)
  {
    case UnaryOperatorType::PLUS:
      return;

    case UnaryOperatorType::MINUS:
      sljit_emit_fop1(_ctx, SLJIT_NEG_F64, SLJIT_FR0, 0, SLJIT_FR0, 0);
      return;

    default:
      fatalError("Unknown operator");
  }
}

template <typename T>
void
CompiledSLJIT<T>::operator()(Node<T> & node, BinaryOperatorData<T> & data)
{
  data._args[0].apply(*this);
  data._args[1].apply(*this);

  // Arguments A = SLJIT_FR0, B = SLJIT_FR1
  sljit_emit_fop1(_ctx, SLJIT_MOV_F64, SLJIT_FR1, 0, SLJIT_FR0, 0);
  stackPop(SLJIT_FR0);

  switch (data._type)
  {
    case BinaryOperatorType::SUBTRACTION:
      sljit_emit_fop2(_ctx, SLJIT_SUB_F64, SLJIT_FR0, 0, SLJIT_FR0, 0, SLJIT_FR1, 0);
      return;

    case BinaryOperatorType::DIVISION:
      sljit_emit_fop2(_ctx, SLJIT_DIV_F64, SLJIT_FR0, 0, SLJIT_FR0, 0, SLJIT_FR1, 0);
      return;

    case BinaryOperatorType::MODULO:
      binaryFunctionCall(std::fmod);
      return;

    case BinaryOperatorType::POWER:
      binaryFunctionCall(std::pow);
      return;

    case BinaryOperatorType::LOGICAL_OR:
    {
      sljit_emit_fop1(_ctx, SLJIT_MOV_F64, SLJIT_FR2, 0, SLJIT_MEM, (sljit_sw)&sljit_zero);
      // either argument is true -> true
      struct sljit_jump * true_lbl1 =
          sljit_emit_fcmp(_ctx, SLJIT_UNORDERED_OR_NOT_EQUAL, SLJIT_FR0, 0, SLJIT_FR2, 0);
      struct sljit_jump * true_lbl2 =
          sljit_emit_fcmp(_ctx, SLJIT_UNORDERED_OR_NOT_EQUAL, SLJIT_FR1, 0, SLJIT_FR2, 0);

      // false case
      // put 0.0 on stack
      sljit_emit_fop1(_ctx, SLJIT_MOV_F64, SLJIT_FR0, 0, SLJIT_FR2, 0);
      struct sljit_jump * out_lbl = sljit_emit_jump(_ctx, SLJIT_JUMP);

      // true case
      sljit_set_label(true_lbl1, sljit_emit_label(_ctx));
      sljit_set_label(true_lbl2, sljit_emit_label(_ctx));

      // put 1.0 on stack
      sljit_emit_fop1(_ctx, SLJIT_MOV_F64, SLJIT_FR0, 0, SLJIT_MEM, (sljit_sw)&sljit_one);

      // end if
      sljit_set_label(out_lbl, sljit_emit_label(_ctx));

      return;
    }

    case BinaryOperatorType::LOGICAL_AND:
    {
      sljit_emit_fop1(_ctx, SLJIT_MOV_F64, SLJIT_FR2, 0, SLJIT_MEM, (sljit_sw)&sljit_zero);
      // either argument is false -> false
      struct sljit_jump * false_lbl1 =
          sljit_emit_fcmp(_ctx, SLJIT_ORDERED_EQUAL, SLJIT_FR0, 0, SLJIT_FR2, 0);
      struct sljit_jump * false_lbl2 =
          sljit_emit_fcmp(_ctx, SLJIT_ORDERED_EQUAL, SLJIT_FR1, 0, SLJIT_FR2, 0);

      // true case
      // put 1.0 on stack
      sljit_emit_fop1(_ctx, SLJIT_MOV_F64, SLJIT_FR0, 0, SLJIT_MEM, (sljit_sw)&sljit_one);
      struct sljit_jump * out_lbl = sljit_emit_jump(_ctx, SLJIT_JUMP);

      // false case
      sljit_set_label(false_lbl1, sljit_emit_label(_ctx));
      sljit_set_label(false_lbl2, sljit_emit_label(_ctx));

      // put 0.0 on stack
      sljit_emit_fop1(_ctx, SLJIT_MOV_F64, SLJIT_FR0, 0, SLJIT_FR2, 0);

      // end if
      sljit_set_label(out_lbl, sljit_emit_label(_ctx));

      return;
    }

    case BinaryOperatorType::LESS_THAN:
      emitFcmp(SLJIT_F_LESS);
      return;

    case BinaryOperatorType::GREATER_THAN:
      emitFcmp(SLJIT_F_GREATER);
      return;

    case BinaryOperatorType::LESS_EQUAL:
      emitFcmp(SLJIT_F_LESS_EQUAL);
      return;

    case BinaryOperatorType::GREATER_EQUAL:
      emitFcmp(SLJIT_F_GREATER_EQUAL);
      return;

    case BinaryOperatorType::EQUAL:
      emitFcmp(SLJIT_F_EQUAL);
      return;

    case BinaryOperatorType::NOT_EQUAL:
      emitFcmp(SLJIT_F_NOT_EQUAL);
      return;

    default:
      fatalError("Unknown operator");
  }
}

template <typename T>
void
CompiledSLJIT<T>::operator()(Node<T> & node, MultinaryOperatorData<T> & data)
{
  if (data._args.size() == 0)
    fatalError("No child nodes in multinary operator");

  data._args[0].apply(*this);

  for (std::size_t i = 1; i < data._args.size(); ++i)
  {
    data._args[i].apply(*this);
    stackPop(SLJIT_FR1);
    switch (data._type)
    {
      case MultinaryOperatorType::ADDITION:
        sljit_emit_fop2(_ctx, SLJIT_ADD_F64, SLJIT_FR0, 0, SLJIT_FR0, 0, SLJIT_FR1, 0);
        break;

      case MultinaryOperatorType::MULTIPLICATION:
        sljit_emit_fop2(_ctx, SLJIT_MUL_F64, SLJIT_FR0, 0, SLJIT_FR0, 0, SLJIT_FR1, 0);
        break;

      default:
        fatalError("Unknown operator");
    }
  }
}

template <typename T>
void
CompiledSLJIT<T>::operator()(Node<T> & node, UnaryFunctionData<T> & data)
{
  data._args[0].apply(*this);

  switch (data._type)
  {
    case UnaryFunctionType::ABS:
      sljit_emit_fop1(_ctx, SLJIT_ABS_F64, SLJIT_FR0, 0, SLJIT_FR0, 0);
      return;

    case UnaryFunctionType::ACOS:
      unaryFunctionCall(std::acos);
      return;

    case UnaryFunctionType::ACOSH:
      unaryFunctionCall(std::acosh);
      return;

    case UnaryFunctionType::ARG:
      fatalError("Function not implemented");

    case UnaryFunctionType::ASIN:
      unaryFunctionCall(std::asin);
      return;

    case UnaryFunctionType::ASINH:
      unaryFunctionCall(std::asinh);
      return;

    case UnaryFunctionType::ATAN:
      unaryFunctionCall(std::atan);
      return;

    case UnaryFunctionType::ATANH:
      unaryFunctionCall(std::atanh);
      return;

    case UnaryFunctionType::CBRT:
      unaryFunctionCall(std::cbrt);
      return;

    case UnaryFunctionType::CEIL:
      unaryFunctionCall(std::ceil);
      return;

    case UnaryFunctionType::CONJ:
      fatalError("Function not implemented");

    case UnaryFunctionType::COS:
      unaryFunctionCall(std::cos);
      return;

    case UnaryFunctionType::COSH:
      unaryFunctionCall(std::cosh);
      return;

    case UnaryFunctionType::COT:
      unaryFunctionCall(std::tan);
      sljit_emit_fop2(
          _ctx, SLJIT_DIV_F64, SLJIT_FR0, 0, SLJIT_MEM, (sljit_sw)&sljit_one, SLJIT_FR0, 0);
      return;

    case UnaryFunctionType::CSC:
      unaryFunctionCall(std::sin);
      sljit_emit_fop2(
          _ctx, SLJIT_DIV_F64, SLJIT_FR0, 0, SLJIT_MEM, (sljit_sw)&sljit_one, SLJIT_FR0, 0);
      return;

    case UnaryFunctionType::ERF:
      unaryFunctionCall(std::erf);
      return;

    case UnaryFunctionType::ERFC:
      unaryFunctionCall(std::erfc);
      return;

    case UnaryFunctionType::EXP:
      unaryFunctionCall(std::exp);
      return;

    case UnaryFunctionType::EXP2:
      unaryFunctionCall(std::exp2);
      return;

    case UnaryFunctionType::FLOOR:
      unaryFunctionCall(std::floor);
      return;

    case UnaryFunctionType::IMAG:
      fatalError("Function not implemented");

    case UnaryFunctionType::INT:
      unaryFunctionCall(std::round);
      return;

    case UnaryFunctionType::LOG:
      unaryFunctionCall(std::log);
      return;

    case UnaryFunctionType::LOG10:
      unaryFunctionCall(std::log10);
      return;

    case UnaryFunctionType::LOG2:
      unaryFunctionCall(std::log2);
      return;

    case UnaryFunctionType::REAL:
      fatalError("Function not implemented");

    case UnaryFunctionType::SEC:
      unaryFunctionCall(std::cos);
      sljit_emit_fop2(
          _ctx, SLJIT_DIV_F64, SLJIT_FR0, 0, SLJIT_MEM, (sljit_sw)&sljit_one, SLJIT_FR0, 0);
      return;

    case UnaryFunctionType::SIN:
      unaryFunctionCall(std::sin);
      return;

    case UnaryFunctionType::SINH:
      unaryFunctionCall(std::sinh);
      return;

    case UnaryFunctionType::SQRT:
      unaryFunctionCall(std::sqrt);
      return;

    case UnaryFunctionType::T:
      fatalError("Function not implemented");

    case UnaryFunctionType::TAN:
      unaryFunctionCall(std::tan);
      return;

    case UnaryFunctionType::TANH:
      unaryFunctionCall(std::tanh);
      return;

    case UnaryFunctionType::TRUNC:
      unaryFunctionCall(truncWrapper);
      return;

    default:
      fatalError("Function not implemented");
  }
}

template <typename T>
void
CompiledSLJIT<T>::operator()(Node<T> & node, BinaryFunctionData<T> & data)
{
  data._args[0].apply(*this);
  data._args[1].apply(*this);

  // Arguments A = SLJIT_FR0, B = SLJIT_FR1
  sljit_emit_fop1(_ctx, SLJIT_MOV_F64, SLJIT_FR1, 0, SLJIT_FR0, 0);
  stackPop(SLJIT_FR0);

  switch (data._type)
  {
    case BinaryFunctionType::ATAN2:
      binaryFunctionCall(std::atan2);
      return;

    case BinaryFunctionType::HYPOT:
      // return jit_insn_sqrt(func,
      //                      jit_insn_add(func, jit_insn_mul(func, A, A), jit_insn_mul(func, B,
      //                      B)));
      fatalError("Function not implemented");

    case BinaryFunctionType::MIN:
    {
      struct sljit_jump * out_lbl =
          sljit_emit_fcmp(_ctx, SLJIT_ORDERED_LESS, SLJIT_FR0, 0, SLJIT_FR1, 0);

      // FR0 >= FR1 case
      // put FR1 on stack
      sljit_emit_fop1(_ctx, SLJIT_MOV_F64, SLJIT_FR0, 0, SLJIT_FR1, 0);

      // else jump here and leave FR0
      sljit_set_label(out_lbl, sljit_emit_label(_ctx));

      return;
    }

    case BinaryFunctionType::MAX:
    {
      struct sljit_jump * out_lbl =
          sljit_emit_fcmp(_ctx, SLJIT_UNORDERED_OR_GREATER, SLJIT_FR0, 0, SLJIT_FR1, 0);

      // FR0 >= FR1 case
      // put FR1 on stack
      sljit_emit_fop1(_ctx, SLJIT_MOV_F64, SLJIT_FR0, 0, SLJIT_FR1, 0);

      // else jump here and leave FR0
      sljit_set_label(out_lbl, sljit_emit_label(_ctx));

      return;
    }

    case BinaryFunctionType::PLOG:
      binaryFunctionCall(plog);
      return;

    case BinaryFunctionType::POW:
      binaryFunctionCall(std::pow);
      return;

    case BinaryFunctionType::POLAR:
    default:
      fatalError("Function not implemented");
  }
}

template <typename T>
void
CompiledSLJIT<T>::operator()(Node<T> & node, RealNumberData<T> & data)
{
  // sljit does not have any 64bit floating point immediates, so we need to make a mem->register
  // transfer this makes the JIT code point to data in the expression tree! We store a copy of the
  // immediate in this object to avoid depending on data in the original expression tree, which
  // might get shuffled around or freed.
  stackPush();
  _immediate.push_back(data._value);
  sljit_emit_fop1(_ctx, SLJIT_MOV_F64, SLJIT_FR0, 0, SLJIT_MEM, (sljit_sw)&_immediate.back());
}

template <typename T>
void
CompiledSLJIT<T>::operator()(Node<T> & node, RealReferenceData<T> & data)
{
  stackPush();
  sljit_emit_fop1(_ctx, SLJIT_MOV_F64, SLJIT_FR0, 0, SLJIT_MEM, (sljit_sw)&data._ref);
}

template <typename T>
void
CompiledSLJIT<T>::operator()(Node<T> & node, RealArrayReferenceData<T> & data)
{
  fatalError("Not implemented");
}

template <typename T>
void
CompiledSLJIT<T>::operator()(Node<T> & node, LocalVariableData<T> & data)
{
  fatalError("Not implemented");
}

template <typename T>
void
CompiledSLJIT<T>::operator()(Node<T> & node, ConditionalData<T> & data)
{
  if (data._type != ConditionalType::IF)
    fatalError("Conditional not implemented");

  struct sljit_jump * false_case;
  struct sljit_jump * end_if;

  data._args[0].apply(*this);

  // sljit_emit_op1(_ctx, SLJIT_MOV, SLJIT_R0, 0, SLJIT_MEM, (sljit_sw)state.stack);
  false_case =
      sljit_emit_fcmp(_ctx, SLJIT_ORDERED_EQUAL, SLJIT_FR0, 0, SLJIT_MEM, (sljit_sw)&sljit_zero);

  // true case
  auto stack_pos = _sp;
  data._args[1].apply(*this);
  end_if = sljit_emit_jump(_ctx, SLJIT_JUMP);

  // false case
  _sp = stack_pos;
  sljit_set_label(false_case, sljit_emit_label(_ctx));
  data._args[2].apply(*this);

  // end if
  sljit_set_label(end_if, sljit_emit_label(_ctx));
}

template <>
void
CompiledSLJIT<Real>::operator()(Node<Real> & node, IntegerPowerData<Real> & data)
{
  if (data._exponent == 0)
  {
    // this case should be simplified away and never reached
    sljit_emit_fop1(_ctx, SLJIT_MOV_F64, SLJIT_FR0, 0, SLJIT_MEM, (sljit_sw)&sljit_one);
    return;
  }

  // FR0 = A
  data._arg.apply(*this);

  // FR1 = FR2 = 1.0
  sljit_emit_fop1(_ctx, SLJIT_MOV_F64, SLJIT_FR2, 0, SLJIT_MEM, (sljit_sw)&sljit_one);
  sljit_emit_fop1(_ctx, SLJIT_MOV_F64, SLJIT_FR1, 0, SLJIT_FR2, 0);

  int e = data._exponent > 0 ? data._exponent : -data._exponent;
  while (e)
  {
    // if bit 0 is set multiply the current power of two factor of the exponent
    if (e & 1)
      sljit_emit_fop2(_ctx, SLJIT_MUL_F64, SLJIT_FR1, 0, SLJIT_FR0, 0, SLJIT_FR1, 0);

    // x is incrementally set to consecutive powers of powers of two
    sljit_emit_fop2(_ctx, SLJIT_MUL_F64, SLJIT_FR0, 0, SLJIT_FR0, 0, SLJIT_FR0, 0);

    // bit shift the exponent down
    e >>= 1;
  }

  if (data._exponent >= 0)
    sljit_emit_fop1(_ctx, SLJIT_MOV_F64, SLJIT_FR0, 0, SLJIT_FR1, 0);
  else
  {
    sljit_emit_fop1(_ctx, SLJIT_MOV_F64, SLJIT_FR0, 0, SLJIT_FR2, 0); // FR0 = 1.0
    sljit_emit_fop2(_ctx, SLJIT_DIV_F64, SLJIT_FR0, 0, SLJIT_FR0, 0, SLJIT_FR1, 0);
  }
}

template class CompiledSLJIT<Real>;

} // namespace SymbolicMath
