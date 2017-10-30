#ifdef SYMBOLICMATH_USE_GCCJIT

#include "SymbolicMathNodeData.h"
#include "SymbolicMathUtils.h"

#include <cmath>
#include <iostream> // debug

namespace SymbolicMath
{

double
gccjit_wrap_min(double a, double b)
{
  return std::min(a, b);
}

double
gccjit_wrap_max(double a, double b)
{
  return std::min(a, b);
}

/********************************************************
 * Real Number immediate
 ********************************************************/

JITReturnValue
RealNumberData::jit(JITStateValue & ctxt)
{
  return gcc_jit_context_new_rvalue_from_double(
      ctxt, gcc_jit_context_get_type(ctxt, GCC_JIT_TYPE_DOUBLE), (double)_value);
}

/********************************************************
 * Real Number reference value provider
 ********************************************************/

JITReturnValue
RealReferenceData::jit(JITStateValue & ctxt)
{
  gcc_jit_rvalue * ptr = gcc_jit_context_new_rvalue_from_ptr(
      ctxt,
      gcc_jit_type_get_pointer(gcc_jit_context_get_type(ctxt, GCC_JIT_TYPE_DOUBLE)),
      reinterpret_cast<void *>(const_cast<double *>(&_ref)));
  return gcc_jit_lvalue_as_rvalue(gcc_jit_rvalue_dereference(ptr, nullptr));
}

/********************************************************
 * Real Number Array reference value provider
 ********************************************************/

JITReturnValue
RealArrayReferenceData::jit(JITStateValue & ctxt)
{
  fatalError("Not implemented yet");
}

/********************************************************
 * Unary Operator
 ********************************************************/

JITReturnValue
UnaryOperatorData::jit(JITStateValue & ctxt)
{
  switch (_type)
  {
    case UnaryOperatorType::PLUS:
      return _args[0].jit(ctxt);

    case UnaryOperatorType::MINUS:
      return gcc_jit_context_new_unary_op(ctxt,
                                          nullptr,
                                          GCC_JIT_UNARY_OP_MINUS,
                                          gcc_jit_context_get_type(ctxt, GCC_JIT_TYPE_DOUBLE),
                                          _args[0].jit(ctxt));

    default:
      fatalError("Unknown operator");
  }
}

/********************************************************
 * Binary Operator Node
 ********************************************************/

JITReturnValue
BinaryOperatorData::jit(JITStateValue & ctxt)
{
  auto A = _args[0].jit(ctxt);
  auto B = _args[1].jit(ctxt);

  enum gcc_jit_binary_op op;

  switch (_type)
  {
    case BinaryOperatorType::SUBTRACTION:
      op = GCC_JIT_BINARY_OP_MINUS;
      break;

    case BinaryOperatorType::DIVISION:
      op = GCC_JIT_BINARY_OP_DIVIDE;
      break;

    case BinaryOperatorType::MODULO:
      op = GCC_JIT_BINARY_OP_MODULO;
      break;

    case BinaryOperatorType::POWER:
      fatalError("not implemented yet");
      // return jit_insn_pow(func, A, B);

    case BinaryOperatorType::LOGICAL_OR:
      op = GCC_JIT_BINARY_OP_LOGICAL_OR;
      break;

    case BinaryOperatorType::LOGICAL_AND:
      op = GCC_JIT_BINARY_OP_LOGICAL_AND;
      break;

      // case BinaryOperatorType::LESS_THAN:
      //   return jit_insn_lt(func, A, B);
      //
      // case BinaryOperatorType::GREATER_THAN:
      //   return jit_insn_gt(func, A, B);
      //
      // case BinaryOperatorType::LESS_EQUAL:
      //   return jit_insn_le(func, A, B);
      //
      // case BinaryOperatorType::GREATER_EQUAL:
      //   return jit_insn_ge(func, A, B);
      //
      // case BinaryOperatorType::EQUAL:
      //   return jit_insn_eq(func, A, B);
      //
      // case BinaryOperatorType::NOT_EQUAL:
      //   return jit_insn_ne(func, A, B);

    default:
      fatalError("Unknown operator");
  }

  return gcc_jit_context_new_binary_op(
      ctxt, nullptr, op, gcc_jit_context_get_type(ctxt, GCC_JIT_TYPE_DOUBLE), A, B);
}

/********************************************************
 * Multinary Operator Node
 ********************************************************/

JITReturnValue
MultinaryOperatorData::jit(JITStateValue & ctxt)
{
  auto double_type = gcc_jit_context_get_type(ctxt, GCC_JIT_TYPE_DOUBLE);
  enum gcc_jit_binary_op op;

  switch (_type)
  {
    case MultinaryOperatorType::ADDITION:
      op = GCC_JIT_BINARY_OP_PLUS;
      break;

    case MultinaryOperatorType::MULTIPLICATION:
      op = GCC_JIT_BINARY_OP_MULT;
      break;

    default:
      fatalError("Unknown operator");
  }

  if (_args.size() == 0)
    fatalError("No child nodes in multinary operator");
  else if (_args.size() == 1)
    return _args[0].jit(ctxt);
  else
  {
    JITReturnValue temp = _args[0].jit(ctxt);
    for (std::size_t i = 1; i < _args.size(); ++i)
      temp =
          gcc_jit_context_new_binary_op(ctxt, nullptr, op, double_type, temp, _args[i].jit(ctxt));

    return temp;
  }
}

/********************************************************
 * Unary Function Node
 ********************************************************/

JITReturnValue
UnaryFunctionData::jit(JITStateValue & ctxt)
{
  auto double_type = gcc_jit_context_get_type(ctxt, GCC_JIT_TYPE_DOUBLE);

  gcc_jit_type * param_types[] = {double_type};
  gcc_jit_rvalue * args[] = {_args[0].jit(ctxt)};

  gcc_jit_type * fn_ptr_type =
      gcc_jit_context_new_function_ptr_type(ctxt, nullptr, double_type, 1, param_types, 0);

  gcc_jit_rvalue * ptr;

  switch (_type)
  {
    case UnaryFunctionType::ABS:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::abs)));
      break;

    case UnaryFunctionType::ACOS:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::acos)));
      break;

    case UnaryFunctionType::ACOSH:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::acosh)));
      break;

    case UnaryFunctionType::ARG:
      fatalError("Function not implemented");

    case UnaryFunctionType::ASIN:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::asin)));
      break;

    case UnaryFunctionType::ASINH:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::asinh)));
      break;

    case UnaryFunctionType::ATAN:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::atan)));
      break;

    case UnaryFunctionType::ATANH:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::atanh)));
      break;

    case UnaryFunctionType::CBRT:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::cbrt)));
      break;

    case UnaryFunctionType::CEIL:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::ceil)));
      break;

    case UnaryFunctionType::CONJ:
      fatalError("Function not implemented");

    case UnaryFunctionType::COS:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::cos)));
      break;

    case UnaryFunctionType::COSH:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::cosh)));
      break;

    case UnaryFunctionType::COT:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::tan)));
      return gcc_jit_context_new_binary_op(
          ctxt,
          nullptr,
          GCC_JIT_BINARY_OP_DIVIDE,
          double_type,
          gcc_jit_context_one(ctxt, double_type),
          gcc_jit_context_new_call_through_ptr(ctxt, nullptr, ptr, 1, args));

    case UnaryFunctionType::CSC:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::sin)));
      return gcc_jit_context_new_binary_op(
          ctxt,
          nullptr,
          GCC_JIT_BINARY_OP_DIVIDE,
          double_type,
          gcc_jit_context_one(ctxt, double_type),
          gcc_jit_context_new_call_through_ptr(ctxt, nullptr, ptr, 1, args));

    case UnaryFunctionType::ERF:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::erf)));
      break;

    case UnaryFunctionType::EXP:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::exp)));
      break;

    case UnaryFunctionType::EXP2:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::exp2)));
      break;

    case UnaryFunctionType::FLOOR:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::floor)));
      break;

    case UnaryFunctionType::IMAG:
      fatalError("Function not implemented");

    case UnaryFunctionType::INT:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::round)));
      break;

    case UnaryFunctionType::LOG:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::log)));
      break;

    case UnaryFunctionType::LOG10:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::log10)));
      break;

    case UnaryFunctionType::LOG2:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::log2)));
      break;

    case UnaryFunctionType::REAL:
      fatalError("Function not implemented");

    case UnaryFunctionType::SEC:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::cos)));
      return gcc_jit_context_new_binary_op(
          ctxt,
          nullptr,
          GCC_JIT_BINARY_OP_DIVIDE,
          double_type,
          gcc_jit_context_one(ctxt, double_type),
          gcc_jit_context_new_call_through_ptr(ctxt, nullptr, ptr, 1, args));

    case UnaryFunctionType::SIN:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::sin)));
      break;

    case UnaryFunctionType::SINH:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::sinh)));
      break;

    case UnaryFunctionType::SQRT:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::sqrt)));
      break;

    case UnaryFunctionType::T:
      fatalError("Function not implemented");

    case UnaryFunctionType::TAN:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::tan)));
      break;

    case UnaryFunctionType::TANH:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double))(std::tanh)));
      break;

    case UnaryFunctionType::TRUNC:
      return gcc_jit_context_new_cast(
          ctxt,
          nullptr,
          gcc_jit_context_new_cast(
              ctxt, nullptr, args[0], gcc_jit_context_get_type(ctxt, GCC_JIT_TYPE_INT)),
          double_type);

    default:
      fatalError("Function not implemented");
  }

  return gcc_jit_context_new_call_through_ptr(ctxt, nullptr, ptr, 1, args);
}

/********************************************************
 * Binary Function Node
 ********************************************************/

JITReturnValue
BinaryFunctionData::jit(JITStateValue & ctxt)
{
  auto double_type = gcc_jit_context_get_type(ctxt, GCC_JIT_TYPE_DOUBLE);

  gcc_jit_type * param_types[] = {double_type, double_type};
  gcc_jit_rvalue * args[] = {_args[0].jit(ctxt), _args[1].jit(ctxt)};

  gcc_jit_type * fn_ptr_type =
      gcc_jit_context_new_function_ptr_type(ctxt, nullptr, double_type, 2, param_types, 0);

  gcc_jit_rvalue * ptr;

  const auto A = _args[0].jit(ctxt);
  const auto B = _args[1].jit(ctxt);

  switch (_type)
  {
    case BinaryFunctionType::ATAN2:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double, double))(std::atan2)));
      break;

    case BinaryFunctionType::HYPOT:
      // return jit_insn_sqrt(func,
      //                      jit_insn_add(func, jit_insn_mul(func, A, A), jit_insn_mul(func, B,
      //                      B)));
      fatalError("Function not implemented");

    case BinaryFunctionType::MIN:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt,
          fn_ptr_type,
          reinterpret_cast<void *>((double (*)(double, double))(gccjit_wrap_min)));
      break;

    case BinaryFunctionType::MAX:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt,
          fn_ptr_type,
          reinterpret_cast<void *>((double (*)(double, double))(gccjit_wrap_max)));
      break;

    case BinaryFunctionType::PLOG:
    {
      fatalError("Function not implemented");
      // return A < B
      //            ? std::log(B) + (A - B) / B - (A - B) * (A - B) / (2.0 * B * B) +
      //                  (A - B) * (A - B) * (A - B) / (3.0 * B * B * B)
      //            : std::log(A);
    }

    case BinaryFunctionType::POW:
      ptr = gcc_jit_context_new_rvalue_from_ptr(
          ctxt, fn_ptr_type, reinterpret_cast<void *>((double (*)(double, double))(std::pow)));
      break;

    case BinaryFunctionType::POLAR:
    default:
      fatalError("Function not implemented");
  }

  return gcc_jit_context_new_call_through_ptr(ctxt, nullptr, ptr, 2, args);
}

/********************************************************
 * Conditional Node
 ********************************************************/

JITReturnValue
ConditionalData::jit(JITStateValue & ctxt)
{
  // if (_type != ConditionalType::IF)
  fatalError("Conditional not implemented");

  // jit_label_t label1 = jit_label_undefined;
  // jit_label_t label2 = jit_label_undefined;
  // JITReturnValue result = jit_value_create(func, jit_type_float64);
  //
  // jit_insn_branch_if_not(func, _args[0].jit(ctxt), &label1);
  // // true branch
  // jit_insn_store(func, result, _args[1].jit(ctxt));
  // jit_insn_branch(func, &label2);
  // jit_insn_label(func, &label1);
  // // false branch
  // jit_insn_store(func, result, _args[2].jit(ctxt));
  // jit_insn_label(func, &label2);
  // return jit_insn_load(func, result);
}

/********************************************************
 * Integer power Node
 ********************************************************/

JITReturnValue
IntegerPowerData::jit(JITStateValue & ctxt)
{
  auto double_type = gcc_jit_context_get_type(ctxt, GCC_JIT_TYPE_DOUBLE);
  gcc_jit_rvalue * result = gcc_jit_context_one(ctxt, double_type);
  gcc_jit_rvalue * A = _arg.jit(ctxt);

  int e = _exponent > 0 ? _exponent : -_exponent;
  while (e)
  {
    // if bit 0 is set multiply the current power of two factor of the exponent
    if (e & 1)
      result = gcc_jit_context_new_binary_op(
          ctxt, nullptr, GCC_JIT_BINARY_OP_MULT, double_type, result, A);

    // x is incrementally set to consecutive powers of powers of two
    A = gcc_jit_context_new_binary_op(ctxt, nullptr, GCC_JIT_BINARY_OP_MULT, double_type, A, A);

    // bit shift the exponent down
    e >>= 1;
  }

  if (_exponent >= 0)
    return result;
  else
    return gcc_jit_context_new_binary_op(ctxt,
                                         nullptr,
                                         GCC_JIT_BINARY_OP_DIVIDE,
                                         double_type,
                                         gcc_jit_context_one(ctxt, double_type),
                                         result);
}

// end namespace SymbolicMath
}

#endif // SYMBOLICMATH_USE_GCCJIT
