///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include "SMFunction.h"
#include "SMCompiledLibJIT.h"
#include "SMCompilerFactory.h"

namespace SymbolicMath
{

registerCompiler(CompiledLibJIT, "CompiledLibJIT", Real, 100);

template <typename T>
CompiledLibJIT<T>::CompiledLibJIT(Function<T> & fb) : Transform<T>(fb), _jit_function(nullptr)
{
  // build and lock context
  _jit_context = jit_context_create();
  jit_context_build_start(_jit_context);

  // build function objext (takes no parameters, returns a double)
  jit_type_t signature = jit_type_create_signature(jit_abi_cdecl, jit_type_float64, nullptr, 0, 1);
  _state = jit_function_create(_jit_context, signature);
  jit_type_free(signature);

  // return the value generated by the expression tree
  apply();
  jit_insn_return(_state, _value);

  // compile, unlock context, transform into closure (jit_function_apply does NOT work!)
  jit_function_compile(_state);
  jit_context_build_end(_jit_context);
  _jit_function = reinterpret_cast<JITFunctionPtr>(jit_function_to_closure(_state));
}

template <typename T>
CompiledLibJIT<T>::~CompiledLibJIT()
{
  if (_jit_context)
    jit_context_destroy(_jit_context);
}

// Helper methods

template <typename T>
jit_value_t
CompiledLibJIT<T>::unaryFunctionCall(T (*func)(T), jit_value_t A)
{
  // create signature
  jit_type_t params[] = {jit_type_float64};
  jit_type_t signature = jit_type_create_signature(jit_abi_cdecl, jit_type_float64, params, 1, 1);

  // set arguments and call function
  jit_value_t args[] = {A};
  return jit_insn_call_native(
      _state, "", reinterpret_cast<void *>(func), signature, args, 1, JIT_CALL_NOTHROW);
}

template <typename T>
jit_value_t
CompiledLibJIT<T>::binaryFunctionCall(T (*func)(T, T), jit_value_t A, jit_value_t B)
{
  // create signature
  jit_type_t params[] = {jit_type_float64, jit_type_float64};
  jit_type_t signature = jit_type_create_signature(jit_abi_cdecl, jit_type_float64, params, 2, 1);

  // set arguments and call function
  jit_value_t args[] = {A, B};
  return jit_insn_call_native(
      _state, "", reinterpret_cast<void *>(func), signature, args, 2, JIT_CALL_NOTHROW);
}

template <typename T>
T
CompiledLibJIT<T>::plog(T a, T b)
{
  return a < b ? std::log(b) + (a - b) / b - (a - b) * (a - b) / (2.0 * b * b) +
                     (a - b) * (a - b) * (a - b) / (3.0 * b * b * b)
               : std::log(a);
}

// Visitor operators

template <typename T>
void
CompiledLibJIT<T>::operator()(Node<T> & node, SymbolData<T> & data)
{
  fatalError("Symbol in compiled function");
}

template <typename T>
void
CompiledLibJIT<T>::operator()(Node<T> & node, UnaryOperatorData<T> & data)
{
  data._args[0].apply(*this);

  switch (data._type)
  {
    case UnaryOperatorType::PLUS:
      return;

    case UnaryOperatorType::MINUS:
      _value = jit_insn_neg(_state, _value);
      return;

    default:
      fatalError("Unknown operator");
  }
}

template <typename T>
void
CompiledLibJIT<T>::operator()(Node<T> & node, BinaryOperatorData<T> & data)
{
  data._args[0].apply(*this);
  const auto A = _value;
  data._args[1].apply(*this);
  const auto B = _value;

  switch (data._type)
  {
    case BinaryOperatorType::SUBTRACTION:
      _value = jit_insn_sub(_state, A, B);
      return;

    case BinaryOperatorType::DIVISION:
      _value = jit_insn_div(_state, A, B);
      return;

    case BinaryOperatorType::MODULO:
    {
      jit_type_t params[] = {jit_type_float64, jit_type_float64};
      jit_type_t signature =
          jit_type_create_signature(jit_abi_cdecl, jit_type_float64, params, 2, 1);
      jit_value_t args[] = {A, B};
      double (*func)(double, double) = std::fmod;
      _value = jit_insn_call_native(
          _state, "", reinterpret_cast<void *>(func), signature, args, 2, JIT_CALL_NOTHROW);
      return;
    }

    case BinaryOperatorType::POWER:
      _value = jit_insn_pow(_state, A, B);
      return;

    case BinaryOperatorType::LOGICAL_OR:
    {
      auto iA = jit_insn_to_bool(_state, A);
      auto iB = jit_insn_to_bool(_state, B);
      _value = jit_insn_or(_state, iA, iB);
      return;
    }

    case BinaryOperatorType::LOGICAL_AND:
    {
      auto iA = jit_insn_to_bool(_state, A);
      auto iB = jit_insn_to_bool(_state, B);
      _value = jit_insn_and(_state, iA, iB);
      return;
    }

    case BinaryOperatorType::LESS_THAN:
      _value = jit_insn_lt(_state, A, B);
      return;

    case BinaryOperatorType::GREATER_THAN:
      _value = jit_insn_gt(_state, A, B);
      return;

    case BinaryOperatorType::LESS_EQUAL:
      _value = jit_insn_le(_state, A, B);
      return;

    case BinaryOperatorType::GREATER_EQUAL:
      _value = jit_insn_ge(_state, A, B);
      return;

    case BinaryOperatorType::EQUAL:
      _value = jit_insn_eq(_state, A, B);
      return;

    case BinaryOperatorType::NOT_EQUAL:
      _value = jit_insn_ne(_state, A, B);
      return;

    default:
      fatalError("Unknown operator");
  }
}

template <typename T>
void
CompiledLibJIT<T>::operator()(Node<T> & node, MultinaryOperatorData<T> & data)
{
  if (data._args.size() == 0)
    fatalError("No child nodes in multinary operator");

  data._args[0].apply(*this);
  if (data._args.size() == 1)
    return;

  auto tmp = _value;
  for (std::size_t i = 1; i < data._args.size(); ++i)
  {
    data._args[i].apply(*this);
    switch (data._type)
    {
      case MultinaryOperatorType::ADDITION:
        tmp = jit_insn_add(_state, tmp, _value);
        break;

      case MultinaryOperatorType::MULTIPLICATION:
        tmp = jit_insn_mul(_state, tmp, _value);
        break;

      default:
        fatalError("Unknown operator");
    }
  }
  _value = tmp;
}

template <>
void
CompiledLibJIT<Real>::operator()(Node<Real> & node, UnaryFunctionData<Real> & data)
{
  data._args[0].apply(*this);
  const auto A = _value;

  switch (data._type)
  {
    case UnaryFunctionType::ABS:
      _value = jit_insn_abs(_state, A);
      return;

    case UnaryFunctionType::ACOS:
      _value = jit_insn_acos(_state, A);
      return;

    case UnaryFunctionType::ACOSH:
      _value = unaryFunctionCall(std::acosh, A);
      return;

    case UnaryFunctionType::ARG:
      fatalError("Function not implemented");

    case UnaryFunctionType::ASIN:
      _value = jit_insn_asin(_state, A);
      return;

    case UnaryFunctionType::ASINH:
      _value = unaryFunctionCall(std::asinh, A);
      return;

    case UnaryFunctionType::ATAN:
      _value = jit_insn_atan(_state, A);
      return;

    case UnaryFunctionType::ATANH:
      _value = unaryFunctionCall(std::atanh, A);
      return;

    case UnaryFunctionType::CBRT:
      _value = unaryFunctionCall(std::cbrt, A);
      return;

    case UnaryFunctionType::CEIL:
      _value = jit_insn_ceil(_state, A);
      return;

    case UnaryFunctionType::CONJ:
      fatalError("Function not implemented");

    case UnaryFunctionType::COS:
      _value = jit_insn_cos(_state, A);
      return;

    case UnaryFunctionType::COSH:
      _value = jit_insn_cosh(_state, A);
      return;

    case UnaryFunctionType::COT:
      _value = jit_insn_div(
          _state,
          jit_value_create_float64_constant(_state, jit_type_float64, (jit_float64)1.0),
          jit_insn_tan(_state, A));
      return;

    case UnaryFunctionType::CSC:
      _value = jit_insn_div(
          _state,
          jit_value_create_float64_constant(_state, jit_type_float64, (jit_float64)1.0),
          jit_insn_sin(_state, A));
      return;

    case UnaryFunctionType::ERF:
      _value = unaryFunctionCall(std::erf, A);
      return;

    case UnaryFunctionType::ERFC:
      _value = unaryFunctionCall(std::erfc, A);
      return;

    case UnaryFunctionType::EXP:
      _value = jit_insn_exp(_state, A);
      return;

    case UnaryFunctionType::EXP2:
      _value = unaryFunctionCall(std::exp2, A);
      return;

    case UnaryFunctionType::FLOOR:
      _value = jit_insn_floor(_state, A);
      return;

    case UnaryFunctionType::IMAG:
      fatalError("Function not implemented");

    case UnaryFunctionType::INT:
      _value = jit_insn_round(_state, A);
      return;

    case UnaryFunctionType::LOG:
      _value = jit_insn_log(_state, A);
      return;

    case UnaryFunctionType::LOG10:
      _value = jit_insn_log10(_state, A);
      return;

    case UnaryFunctionType::LOG2:
      _value = unaryFunctionCall(std::log2, A);
      return;

    case UnaryFunctionType::REAL:
      fatalError("Function not implemented");

    case UnaryFunctionType::SEC:
      _value = jit_insn_div(
          _state,
          jit_value_create_float64_constant(_state, jit_type_float64, (jit_float64)1.0),
          jit_insn_cos(_state, A));
      return;

    case UnaryFunctionType::SIN:
      _value = jit_insn_sin(_state, A);
      return;

    case UnaryFunctionType::SINH:
      _value = jit_insn_sinh(_state, A);
      return;

    case UnaryFunctionType::SQRT:
      _value = jit_insn_sqrt(_state, A);
      return;

    case UnaryFunctionType::T:
      fatalError("Function not implemented");

    case UnaryFunctionType::TAN:
      _value = jit_insn_tan(_state, A);
      return;

    case UnaryFunctionType::TANH:
      _value = jit_insn_tanh(_state, A);
      return;

    case UnaryFunctionType::TRUNC:
      _value = jit_insn_convert(
          _state, jit_insn_convert(_state, A, jit_type_int, 0), jit_type_float64, 0);
      return;

    default:
      fatalError("Function not implemented");
  }
}

template <typename T>
void
CompiledLibJIT<T>::operator()(Node<T> & node, BinaryFunctionData<T> & data)
{
  data._args[0].apply(*this);
  const auto A = _value;
  data._args[1].apply(*this);
  const auto B = _value;

  switch (data._type)
  {
    case BinaryFunctionType::ATAN2:
      _value = jit_insn_atan2(_state, A, B);
      return;

    case BinaryFunctionType::HYPOT:
      _value = jit_insn_sqrt(
          _state, jit_insn_add(_state, jit_insn_mul(_state, A, A), jit_insn_mul(_state, B, B)));
      return;

    case BinaryFunctionType::MIN:
      _value = jit_insn_min(_state, A, B);
      return;

    case BinaryFunctionType::MAX:
      _value = jit_insn_max(_state, A, B);
      return;

    case BinaryFunctionType::PLOG:
      _value = binaryFunctionCall(plog, A, B);
      return;

    case BinaryFunctionType::POW:
      _value = jit_insn_pow(_state, A, B);
      return;

    case BinaryFunctionType::POLAR:
    default:
      fatalError("Function not implemented");
  }
}

template <>
void
CompiledLibJIT<Real>::operator()(Node<Real> & node, RealNumberData<Real> & data)
{
  _value = jit_value_create_float64_constant(_state, jit_type_float64, (jit_float64)data._value);
}

template <>
void
CompiledLibJIT<Real>::operator()(Node<Real> & node, RealReferenceData<Real> & data)
{
  _value =
      jit_insn_load_relative(_state,
                             jit_value_create_nint_constant(
                                 _state, jit_type_void_ptr, reinterpret_cast<jit_nint>(&data._ref)),
                             0,
                             jit_type_float64);
}

template <>
void
CompiledLibJIT<Real>::operator()(Node<Real> & node, RealArrayReferenceData<Real> & data)
{
  auto index = jit_insn_load_relative(
      _state,
      jit_value_create_nint_constant(
          _state, jit_type_void_ptr, reinterpret_cast<jit_nint>(&data._index)),
      0,
      jit_type_int);

  _value = jit_insn_load_elem_address(
      _state,
      jit_value_create_nint_constant(
          _state, jit_type_void_ptr, reinterpret_cast<jit_nint>(&data._ref)),
      index,
      jit_type_float64);
}

template <typename T>
void
CompiledLibJIT<T>::operator()(Node<T> & node, LocalVariableData<T> & data)
{
  fatalError("Not implemented");
}

template <>
void
CompiledLibJIT<Real>::operator()(Node<Real> & node, ConditionalData<Real> & data)
{
  if (data._type != ConditionalType::IF)
    fatalError("Conditional not implemented");

  auto label1 = jit_label_undefined;
  auto label2 = jit_label_undefined;
  auto result = jit_value_create(_state, jit_type_float64);

  data._args[0].apply(*this);
  jit_insn_branch_if_not(_state, _value, &label1);
  // true branch
  data._args[1].apply(*this);
  jit_insn_store(_state, result, _value);
  jit_insn_branch(_state, &label2);
  jit_insn_label(_state, &label1);
  // false branch
  data._args[2].apply(*this);
  jit_insn_store(_state, result, _value);
  jit_insn_label(_state, &label2);
  _value = jit_insn_load(_state, result);
}

template <>
void
CompiledLibJIT<Real>::operator()(Node<Real> & node, IntegerPowerData<Real> & data)
{
  auto result = jit_value_create_float64_constant(_state, jit_type_float64, (jit_float64)1.0);

  data._arg.apply(*this);
  auto A = _value;
  int e = std::abs(data._exponent);
  while (e)
  {
    // if bit 0 is set multiply the current power of two factor of the exponent
    if (e & 1)
      result = jit_insn_mul(_state, result, A);

    // x is incrementally set to consecutive powers of powers of two
    A = jit_insn_mul(_state, A, A);

    // bit shift the exponent down
    e >>= 1;
  }

  if (data._exponent >= 0)
    _value = result;
  else
    _value =
        jit_insn_div(_state,
                     jit_value_create_float64_constant(_state, jit_type_float64, (jit_float64)1.0),
                     result);
}

template class CompiledLibJIT<Real>;

} // namespace SymbolicMath
