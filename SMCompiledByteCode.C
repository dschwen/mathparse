///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include "SMFunction.h"
#include "SMCompiledByteCode.h"

namespace SymbolicMath
{

template <typename T>
CompiledByteCodeTempl<T>::CompiledByteCodeTempl(Function & fb) : Transform(fb)
{
  // determine required stack size
  auto current_max = std::make_pair(0, 0);
  fb.root().stackDepth(current_max);
  _stack.resize(current_max.second);

  apply();
}

template <typename T>
void
CompiledByteCodeTempl<T>::operator()(SymbolData * n)
{
  fatalError("Symbol in compiled function");
}

template <typename T>
void
CompiledByteCodeTempl<T>::operator()(UnaryOperatorData * n)
{
  n->_args[0].apply(*this);
  _byte_code.emplace_back(VMInstruction::UNARY_OPERATOR, n->_type);
}

template <typename T>
void
CompiledByteCodeTempl<T>::operator()(BinaryOperatorData * n)
{
  n->_args[0].apply(*this);
  n->_args[1].apply(*this);
  _byte_code.emplace_back(VMInstruction::BINARY_OPERATOR, n->_type);
}

template <typename T>
void
CompiledByteCodeTempl<T>::operator()(MultinaryOperatorData * n)
{
  const int nargs = static_cast<int>(n->_args.size());
  for (auto arg : n->_args)
    arg.apply(*this);

  if (nargs < 2)
    return;

  switch (n->_type)
  {
    case MultinaryOperatorType::ADDITION:
      _byte_code.emplace_back(VMInstruction::MULTINARY_PLUS, nargs);
      break;

    case MultinaryOperatorType::MULTIPLICATION:
      _byte_code.emplace_back(VMInstruction::MULTINARY_MULTIPLY, nargs);
      break;

    default:
      fatalError("Unknown operator");
  }
}

template <typename T>
void
CompiledByteCodeTempl<T>::operator()(UnaryFunctionData * n)
{
  n->_args[0].apply(*this);
  _byte_code.emplace_back(VMInstruction::UNARY_FUNCTION, n->_type);
}

template <typename T>
void
CompiledByteCodeTempl<T>::operator()(BinaryFunctionData * n)
{
  n->_args[0].apply(*this);
  n->_args[1].apply(*this);
  _byte_code.emplace_back(VMInstruction::BINARY_FUNCTION, n->_type);
}

template <typename T>
void
CompiledByteCodeTempl<T>::operator()(RealNumberData * n)
{
  _byte_code.emplace_back(VMInstruction::LOAD_IMMEDIATE_REAL, n->_value);
}

template <typename T>
void
CompiledByteCodeTempl<T>::operator()(RealReferenceData * n)
{
  _byte_code.emplace_back(VMInstruction::LOAD_VARIABLE_REAL, &n->_ref);
}

template <typename T>
void
CompiledByteCodeTempl<T>::operator()(RealArrayReferenceData * n)
{
  fatalError("Not implemented");
}

template <typename T>
void
CompiledByteCodeTempl<T>::operator()(LocalVariableData * n)
{
  fatalError("Not implemented");
}

template <typename T>
void
CompiledByteCodeTempl<T>::operator()(ConditionalData * n)
{
  n->_args[0].apply(*this);
  const auto conditional_ip = _byte_code.size();
  _byte_code.emplace_back(VMInstruction::CONDITIONAL, 0);
  // true branch
  n->_args[1].apply(*this);
  // jump past false at the end of the true branch
  const auto jump_past_false_ip = _byte_code.size();
  _byte_code.emplace_back(VMInstruction::JUMP, 0);
  // set jump to false ip on conditional instruction
  _byte_code[conditional_ip].second._int_value = _byte_code.size();
  // false branch
  n->_args[2].apply(*this);
  // set jump past false target
  _byte_code[jump_past_false_ip].second._int_value = _byte_code.size();
}

template <typename T>
void
CompiledByteCodeTempl<T>::operator()(IntegerPowerData * n)
{
  n->_arg.apply(*this);
  _byte_code.emplace_back(VMInstruction::INTEGER_POWER, n->_exponent);
}

template <typename T>
T
CompiledByteCodeTempl<T>::operator()()
{
  // initialize instruction and stack pointer and loop over byte code
  std::size_t ip = 0, sp = 0;
  do
  {
    const auto & cur = _byte_code[ip];
    switch (cur.first)
    {
      case VMInstruction::JUMP:
        ip = cur.second._int_value - 1;
        continue;

      case VMInstruction::CONDITIONAL:
        if (_stack[--sp] == 0)
          ip = cur.second._int_value - 1;
        continue;

      case VMInstruction::LOAD_IMMEDIATE_INTEGER:
        _stack[sp++] = cur.second._int_value;
        continue;

      case VMInstruction::LOAD_IMMEDIATE_REAL:
        _stack[sp++] = cur.second._value;
        continue;

      case VMInstruction::LOAD_VARIABLE_REAL:
        _stack[sp++] = *(cur.second._variable);
        continue;

      case VMInstruction::MULTINARY_PLUS:
      {
        // take one summand off the stack and loop over remaining summands
        const auto & num = cur.second._int_value - 1;
        auto sum = _stack[--sp];
        for (std::size_t i = sp - num; i < sp; ++i)
          sum += _stack[i];
        sp -= num;

        // put sum on stack
        _stack[sp++] = sum;
        continue;
      }

      case VMInstruction::MULTINARY_MULTIPLY:
      {
        // take one factor off the stack and loop over remaining factors
        const auto & num = cur.second._int_value - 1;
        auto prod = _stack[--sp];
        for (std::size_t i = sp - num; i < sp; ++i)
          prod *= _stack[i];
        sp -= num;

        // put product on stack
        _stack[sp++] = prod;
        continue;
      }

      case VMInstruction::UNARY_OPERATOR:
      {
        auto & a = _stack[sp - 1];
        switch (cur.second._unary_operator)
        {
          case UnaryOperatorType::PLUS:
            continue;

          case UnaryOperatorType::MINUS:
            a = -a;
            continue;

          default:
            fatalError("Unknown operator");
        }
      }

      case VMInstruction::BINARY_OPERATOR:
      {
        const auto & b = _stack[--sp];
        auto & a = _stack[sp - 1];
        switch (cur.second._binary_operator)
        {
          case BinaryOperatorType::SUBTRACTION:
            a = a - b;
            continue;

          case BinaryOperatorType::DIVISION:
            a = a / b;
            continue;

          case BinaryOperatorType::MODULO:
            a = std::fmod(a, b);
            continue;

          case BinaryOperatorType::POWER:
            a = std::pow(a, b);
            continue;

          case BinaryOperatorType::LOGICAL_OR:
            a = a || b ? 1.0 : 0.0;
            continue;

          case BinaryOperatorType::LOGICAL_AND:
            a = a && b ? 1.0 : 0.0;
            continue;

          case BinaryOperatorType::LESS_THAN:
            a = a < b ? 1.0 : 0.0;
            continue;

          case BinaryOperatorType::GREATER_THAN:
            a = a > b ? 1.0 : 0.0;
            continue;

          case BinaryOperatorType::LESS_EQUAL:
            a = a <= b ? 1.0 : 0.0;
            continue;

          case BinaryOperatorType::GREATER_EQUAL:
            a = a >= b ? 1.0 : 0.0;
            continue;

          case BinaryOperatorType::EQUAL:
            a = a == b ? 1.0 : 0.0;
            continue;

          case BinaryOperatorType::NOT_EQUAL:
            a = a != b ? 1.0 : 0.0;
            continue;

          default:
            fatalError("Unknown operator");
        }
      }

      case VMInstruction::UNARY_FUNCTION:
      {
        auto & a = _stack[sp - 1];
        switch (cur.second._unary_function)
        {
          case UnaryFunctionType::ABS:
            a = std::abs(a);
            continue;

          case UnaryFunctionType::ACOS:
            a = std::acos(a);
            continue;

          case UnaryFunctionType::ACOSH:
            a = std::acosh(a);
            continue;

          case UnaryFunctionType::ASIN:
            a = std::asin(a);
            continue;

          case UnaryFunctionType::ASINH:
            a = std::asinh(a);
            continue;

          case UnaryFunctionType::ATAN:
            a = std::atan(a);
            continue;

          case UnaryFunctionType::ATANH:
            a = std::atanh(a);
            continue;

          case UnaryFunctionType::CBRT:
            a = std::cbrt(a);
            continue;

          case UnaryFunctionType::CEIL:
            a = std::ceil(a);
            continue;

          case UnaryFunctionType::COS:
            a = std::cos(a);
            continue;

          case UnaryFunctionType::COSH:
            a = std::cosh(a);
            continue;

          case UnaryFunctionType::COT:
            a = 1.0 / std::tan(a);
            continue;

          case UnaryFunctionType::CSC:
            a = 1.0 / std::sin(a);
            continue;

          case UnaryFunctionType::ERF:
            a = std::erf(a);
            continue;

          case UnaryFunctionType::EXP:
            a = std::exp(a);
            continue;

          case UnaryFunctionType::EXP2:
            a = std::exp2(a);
            continue;

          case UnaryFunctionType::FLOOR:
            a = std::floor(a);
            continue;

          case UnaryFunctionType::INT:
            a = std::round(a);
            continue;

          case UnaryFunctionType::LOG:
            a = std::log(a);
            continue;

          case UnaryFunctionType::LOG10:
            a = std::log10(a);
            continue;

          case UnaryFunctionType::LOG2:
            a = std::log2(a);
            continue;

          case UnaryFunctionType::SEC:
            a = 1.0 / std::cos(a);
            continue;

          case UnaryFunctionType::SIN:
            a = std::sin(a);
            continue;

          case UnaryFunctionType::SINH:
            a = std::sinh(a);
            continue;

          case UnaryFunctionType::SQRT:
            a = std::sqrt(a);
            continue;

          case UnaryFunctionType::TAN:
            a = std::tan(a);
            continue;

          case UnaryFunctionType::TANH:
            a = std::tanh(a);
            continue;

          case UnaryFunctionType::TRUNC:
            a = static_cast<int>(a);
            continue;

          default:
            fatalError("Function not implemented");
        }
      }

      case VMInstruction::BINARY_FUNCTION:
      {
        const auto & b = _stack[--sp];
        auto & a = _stack[sp - 1];
        switch (cur.second._binary_function)
        {
          case BinaryFunctionType::ATAN2:
            a = std::atan2(a, b);
            continue;

          case BinaryFunctionType::HYPOT:
            a = std::sqrt(a * a + b * b);
            continue;

          case BinaryFunctionType::MAX:
            a = std::max(a, b);
            continue;

          case BinaryFunctionType::MIN:
            a = std::min(a, b);
            continue;

          case BinaryFunctionType::PLOG:
            a = a < b ? std::log(b) + (a - b) / b - (a - b) * (a - b) / (2.0 * b * b) +
                            (a - b) * (a - b) * (a - b) / (3.0 * b * b * b)
                      : std::log(a);
            continue;

          case BinaryFunctionType::POW:
            a = std::pow(a, b);
            continue;

          case BinaryFunctionType::POLAR:
          default:
            fatalError("Function not implemented");
        }
      }

      case VMInstruction::INTEGER_POWER:
      {
        auto & a = _stack[sp - 1];
        bool neg = false;
        auto x = a;
        a = 1.0;
        int e = cur.second._int_value;

        if (e < 0)
        {
          neg = true;
          e = -e;
        }

        while (e)
        {
          // if bit 0 is set multiply the current power of two factor of the exponent
          if (e & 1)
            a *= x;

          // x is incrementally set to consecutive powers of powers of two
          x *= x;

          // bit shift the exponent down
          e >>= 1;
        }

        if (neg)
          a = 1.0 / a;
        continue;
      }
    }
  } while (++ip < _byte_code.size());

  // return result from top of stack
  return _stack[--sp];
}

template class CompiledByteCodeTempl<Real>;

} // namespace SymbolicMath
