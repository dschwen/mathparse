///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include "SMFunction.h"
#include "SMCompiledLightning.h"
#include "SMCompilerFactory.h"

extern "C"
{
#include <lightning.h>
}

namespace SymbolicMath
{

registerCompiler(CompiledLightning, "CompiledLightning", Real);

template <typename T>
CompiledLightning<T>::CompiledLightning(Function<T> & fb)
  : Transform<T>(fb), _jit(nullptr), _jit_function(nullptr)
{
  // global one time initialization
  static struct InitializationSingleton
  {
    InitializationSingleton() { init_jit(nullptr); }
  } initialize;

  // build and lock context
  _jit = jit_new_state();

  // build function
  jit_prolog();

  // determine required stack size
  auto current_max = std::make_pair(0, 0);
  fb.root().stackDepth(current_max);
  if (current_max.first <= 0)
    fatalError("Stack depleted at function end");
  _stack_base = jit_allocai(current_max.second * sizeof(T));

  // start of stack in the empty. Actual top of stack is register F0 and at program start
  // that register does not contain anything to be pushed into the stack.
  _sp = -1;

  // build function from expression tree
  apply();

  // return top of stack (register F0)
  jit_retr_d(JIT_F0);
  jit_epilog();

  // generate machine code
  _jit_function = reinterpret_cast<JITFunctionPtr>(jit_emit());

  // free the compiler data
  jit_clear_state();
}

template <typename T>
CompiledLightning<T>::~CompiledLightning()
{
  if (_jit_function)
    jit_destroy_state();
}

// Helper methods

template <typename T>
void CompiledLightning<T>::unaryFunctionCall(T (*func)(T))
{
  jit_finishi(reinterpret_cast<void *>(func));
  jit_retval_d(JIT_F0);
}
template <typename T>
void CompiledLightning<T>::binaryFunctionCall(T (*func)(T, T))
{
  jit_prepare();
  jit_pushargr_d(JIT_F1);
  jit_pushargr_d(JIT_F0);
  jit_finishi(reinterpret_cast<void *>(func));
  jit_retval_d(JIT_F0);
}

template <>
void
CompiledLightning<double>::stackPush()
{
  if (_sp >= 0)
    jit_stxi_d(_sp * sizeof(double) + _stack_base, JIT_FP, JIT_F0);
  _sp++;
}

template <>
void
CompiledLightning<float>::stackPush()
{
  if (_sp >= 0)
    jit_stxi_f(_sp * sizeof(float) + _stack_base, JIT_FP, JIT_F0);
  _sp++;
}

template <>
void
CompiledLightning<double>::stackPop(int reg)
{
  if (_sp == 0)
    fatalError("Stack exhausted in stackPop");
  _sp--;
  jit_ldxi_d(reg, JIT_FP, _sp * sizeof(double) + _stack_base);
}

template <>
void
CompiledLightning<float>::stackPop(int reg)
{
  if (_sp == 0)
    fatalError("Stack exhausted in stackPop");
  _sp--;
  jit_ldxi_f(reg, JIT_FP, _sp * sizeof(float) + _stack_base);
}

template <typename T>
T
CompiledLightning<T>::wrapMin(T a, T b)
{
  return std::min(a, b);
}

template <typename T>
T
CompiledLightning<T>::wrapMax(T a, T b)
{
  return std::max(a, b);
}

template <typename T>
T
CompiledLightning<T>::plog(T a, T b)
{
  return a < b ? std::log(b) + (a - b) / b - (a - b) * (a - b) / (2.0 * b * b) +
                     (a - b) * (a - b) * (a - b) / (3.0 * b * b * b)
               : std::log(a);
}

// Visitor operators

template <typename T>
void
CompiledLightning<T>::operator()(Node<T> & node, SymbolData<T> & data)
{
  fatalError("Symbol in compiled function");
}

template <typename T>
void
CompiledLightning<T>::operator()(Node<T> & node, UnaryOperatorData<T> & data)
{
  data._args[0].apply(*this);

  switch (data._type)
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

template <typename T>
void
CompiledLightning<T>::operator()(Node<T> & node, BinaryOperatorData<T> & data)
{
  data._args[0].apply(*this);
  data._args[1].apply(*this);

  // Arguments A = JIT_F1, B = JIT_F0 !
  stackPop(JIT_F1);

  switch (data._type)
  {
    case BinaryOperatorType::SUBTRACTION:
      jit_subr_d(JIT_F0, JIT_F1, JIT_F0);
      return;

    case BinaryOperatorType::DIVISION:
      jit_divr_d(JIT_F0, JIT_F1, JIT_F0);
      return;

    case BinaryOperatorType::MODULO:
      binaryFunctionCall(std::fmod);
      return;

    case BinaryOperatorType::POWER:
      binaryFunctionCall(std::pow);
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

template <typename T>
void
CompiledLightning<T>::operator()(Node<T> & node, MultinaryOperatorData<T> & data)
{
  if (data._args.size() == 0)
    fatalError("No child nodes in multinary operator");

  data._args[0].apply(*this);
  if (data._args.size() == 1)
    return;

  for (std::size_t i = 1; i < data._args.size(); ++i)
  {
    data._args[i].apply(*this);
    stackPop(JIT_F1);
    switch (data._type)
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

template <typename T>
void
CompiledLightning<T>::operator()(Node<T> & node, UnaryFunctionData<T> & data)
{
  data._args[0].apply(*this);
  jit_prepare();
  jit_pushargr_d(JIT_F0);

  switch (data._type)
  {
    case UnaryFunctionType::ABS:
      unaryFunctionCall(std::abs);
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
      jit_movi_d(JIT_F1, 1.0);
      jit_divr_d(JIT_F0, JIT_F1, JIT_F0);
      return;

    case UnaryFunctionType::CSC:
      unaryFunctionCall(std::sin);
      jit_movi_d(JIT_F1, 1.0);
      jit_divr_d(JIT_F0, JIT_F1, JIT_F0);
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
      jit_movi_d(JIT_F1, 1.0);
      jit_divr_d(JIT_F0, JIT_F1, JIT_F0);
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
      jit_truncr_d_l(JIT_R0, JIT_F0);
      jit_extr_d(JIT_F0, JIT_R0);
      return;

    default:
      fatalError("Function not implemented");
  }
}

template <typename T>
void
CompiledLightning<T>::operator()(Node<T> & node, BinaryFunctionData<T> & data)
{
  data._args[0].apply(*this);
  data._args[1].apply(*this);

  // Arguments A = JIT_F1, B = JIT_F0 !
  stackPop(JIT_F1);

  switch (data._type)
  {
    case BinaryFunctionType::ATAN2:
      binaryFunctionCall(std::atan2);
      return;

    case BinaryFunctionType::HYPOT:
      fatalError("Function not implemented");

    case BinaryFunctionType::MIN:
      binaryFunctionCall(wrapMin);
      return;

    case BinaryFunctionType::MAX:
      binaryFunctionCall(wrapMax);
      return;

    case BinaryFunctionType::PLOG:
      binaryFunctionCall(plog);

    case BinaryFunctionType::POW:
      binaryFunctionCall(std::pow);
      return;

    case BinaryFunctionType::POLAR:
    default:
      fatalError("Function not implemented");
  }
}

template <>
void
CompiledLightning<Real>::operator()(Node<Real> & node, RealNumberData<Real> & data)
{
  stackPush();
  jit_movi_d(JIT_F0, data._value);
}

template <>
void
CompiledLightning<Real>::operator()(Node<Real> & node, RealReferenceData<Real> & data)
{
  stackPush();
  jit_ldi_d(JIT_F0, const_cast<void *>(reinterpret_cast<const void *>(&data._ref)));
}

template <typename T>
void
CompiledLightning<T>::operator()(Node<T> & node, RealArrayReferenceData<T> & data)
{
  fatalError("Not implemented");
}

template <typename T>
void
CompiledLightning<T>::operator()(Node<T> & node, LocalVariableData<T> & data)
{
  fatalError("Not implemented");
}

template <typename T>
void
CompiledLightning<T>::operator()(Node<T> & node, ConditionalData<T> & data)
{
  if (data._type != ConditionalType::IF)
    fatalError("Conditional not implemented");

  // we could inspect the condition node here and do a jmp/cmp combo instruction
  data._args[0].apply(*this);
  jit_node_t * jump_false = jit_beqi_d(JIT_F0, 0.0);

  // true case
  auto stack_pos = _sp;
  data._args[1].apply(*this);
  jit_node_t * jump_end = jit_jmpi();

  // false case
  _sp = stack_pos;
  jit_patch(jump_false);
  data._args[2].apply(*this);

  // end if
  jit_patch(jump_end);
}

template <typename T>
void
CompiledLightning<T>::operator()(Node<T> & node, IntegerPowerData<T> & data)
{
  if (data._exponent == 0)
    jit_movi_d(JIT_F0, 1.0);
  else
  {
    data._arg.apply(*this);
    int e = data._exponent > 0 ? data._exponent : -data._exponent;
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
    if (data._exponent > 0)
      jit_movr_d(JIT_F0, JIT_F1);
    else
    {
      jit_movi_d(JIT_F0, 1.0);
      jit_divr_d(JIT_F0, JIT_F0, JIT_F1);
    }
  }
}

template class CompiledLightning<Real>;

} // namespace SymbolicMath
