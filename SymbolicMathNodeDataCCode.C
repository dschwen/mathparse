#ifdef SYMBOLICMATH_USE_CCODE

#include "SymbolicMathNodeData.h"
#include "SymbolicMathUtils.h"

#include <cmath>
#include <iostream> // debug

namespace SymbolicMath
{

/********************************************************
 * Real Number immediate
 ********************************************************/

JITReturnValue
RealNumberData::jit(JITStateValue & func)
{
  return stringify(_value);
}

/********************************************************
 * Real Number reference value provider
 ********************************************************/

JITReturnValue
RealReferenceData::jit(JITStateValue & func)
{
  return "*(reinterpret_cast<double *>(" + std::to_string(reinterpret_cast<long>(&_ref)) + "))";
}

/********************************************************
 * Real Number Array reference value provider
 ********************************************************/

JITReturnValue
RealArrayReferenceData::jit(JITStateValue & func)
{
  fatalError("Not implemented");
}

/********************************************************
 * Unary Operator
 ********************************************************/

JITReturnValue
UnaryOperatorData::jit(JITStateValue & func)
{
  switch (_type)
  {
    case UnaryOperatorType::PLUS:
      return _args[0].jit(func);

    case UnaryOperatorType::MINUS:
      return "-" + _args[0].jit(func);

    default:
      fatalError("Unknown operator");
  }
}

/********************************************************
 * Binary Operator Node
 ********************************************************/

JITReturnValue
BinaryOperatorData::jit(JITStateValue & func)
{
  auto A = _args[0].jit(func);
  auto B = _args[1].jit(func);

  switch (_type)
  {
    case BinaryOperatorType::SUBTRACTION:
      return "(" + A + ") - (" + B + ")";

    case BinaryOperatorType::DIVISION:
      return "(" + A + ") / (" + B + ")";

    case BinaryOperatorType::MODULO:
      return "std::fmod(" + A + ", " + B + ")";

    case BinaryOperatorType::POWER:
      return "std::pow(" + A + ", " + B + ")";

    case BinaryOperatorType::LOGICAL_OR:
      return "static_cast<double>(bool(" + A + ") || bool(" + B + "))";

    case BinaryOperatorType::LOGICAL_AND:
      return "static_cast<double>(bool(" + A + ") && bool(" + B + "))";

    case BinaryOperatorType::LESS_THAN:
      return "static_cast<double>((" + A + ") < (" + B + "))";

    case BinaryOperatorType::GREATER_THAN:
      return "static_cast<double>((" + A + ") > (" + B + "))";

    case BinaryOperatorType::LESS_EQUAL:
      return "static_cast<double>((" + A + ") <= (" + B + "))";

    case BinaryOperatorType::GREATER_EQUAL:
      return "static_cast<double>((" + A + ") >= (" + B + "))";

    case BinaryOperatorType::EQUAL:
      return "static_cast<double>((" + A + ") == (" + B + "))";

    case BinaryOperatorType::NOT_EQUAL:
      return "static_cast<double>((" + A + ") != (" + B + "))";

    default:
      fatalError("Unknown operator");
  }
}

/********************************************************
 * Multinary Operator Node
 ********************************************************/

JITReturnValue
MultinaryOperatorData::jit(JITStateValue & func)
{
  if (_args.size() == 0)
    fatalError("No child nodes in multinary operator");

  char op;
  switch (_type)
  {
    case MultinaryOperatorType::ADDITION:
      op = '+';
      break;

    case MultinaryOperatorType::MULTIPLICATION:
      op = '*';
      break;

    default:
      fatalError("Unknown operator");
  }

  if (_args.size() == 1)
    return _args[0].jit(func);
  else
  {
    std::string temp = "(" + _args[0].jit(func) + ')';
    for (std::size_t i = 1; i < _args.size(); ++i)
      temp += op + ("(" + _args[i].jit(func) + ')');
    return temp;
  }
}

/********************************************************
 * Unary Function Node
 ********************************************************/

JITReturnValue
UnaryFunctionData::jit(JITStateValue & func)
{
  const auto A = _args[0].jit(func);

  switch (_type)
  {
    case UnaryFunctionType::ABS:
      return "std::abs(" + A + ")";

    case UnaryFunctionType::ACOS:
      return "std::acos(" + A + ")";

    case UnaryFunctionType::ACOSH:
      return "std::acosh(" + A + ")";

    case UnaryFunctionType::ARG:
      fatalError("Function not implemented");

    case UnaryFunctionType::ASIN:
      return "std::asin(" + A + ")";

    case UnaryFunctionType::ASINH:
      return "std::asinh(" + A + ")";

    case UnaryFunctionType::ATAN:
      return "std::atan(" + A + ")";

    case UnaryFunctionType::ATANH:
      return "std::atanh(" + A + ")";

    case UnaryFunctionType::CBRT:
      return "std::cbrt(" + A + ")";

    case UnaryFunctionType::CEIL:
      return "std::ceil(" + A + ")";

    case UnaryFunctionType::CONJ:
      fatalError("Function not implemented");

    case UnaryFunctionType::COS:
      return "std::cos(" + A + ")";

    case UnaryFunctionType::COSH:
      return "std::cosh(" + A + ")";

    case UnaryFunctionType::COT:
      return "1.0 / std::tan(" + A + ")";

    case UnaryFunctionType::CSC:
      return "1.0 / std::sin(" + A + ")";

    case UnaryFunctionType::ERF:
      return "std::erf(" + A + ")";

    case UnaryFunctionType::EXP:
      return "std::exp(" + A + ")";

    case UnaryFunctionType::EXP2:
      return "std::exp2(" + A + ")";

    case UnaryFunctionType::FLOOR:
      return "std::floor(" + A + ")";

    case UnaryFunctionType::IMAG:
      fatalError("Function not implemented");

    case UnaryFunctionType::INT:
      return "std::round(" + A + ")";

    case UnaryFunctionType::LOG:
      return "std::log(" + A + ")";

    case UnaryFunctionType::LOG10:
      return "std::log10(" + A + ")";

    case UnaryFunctionType::LOG2:
      return "std::log2(" + A + ")";

    case UnaryFunctionType::REAL:
      fatalError("Function not implemented");

    case UnaryFunctionType::SEC:
      return "1.0 / std::cos(" + A + ")";

    case UnaryFunctionType::SIN:
      return "std::sin(" + A + ")";

    case UnaryFunctionType::SINH:
      return "std::sinh(" + A + ")";

    case UnaryFunctionType::SQRT:
      return "std::sqrt(" + A + ")";

    case UnaryFunctionType::T:
      fatalError("Function not implemented");

    case UnaryFunctionType::TAN:
      return "std::tan(" + A + ")";

    case UnaryFunctionType::TANH:
      return "std::tanh(" + A + ")";

    case UnaryFunctionType::TRUNC:
      return "static_cast<int>(" + A + ")";

    default:
      fatalError("Function not implemented");
  }
}

/********************************************************
 * Binary Function Node
 ********************************************************/

JITReturnValue
BinaryFunctionData::jit(JITStateValue & func)
{
  const auto A = _args[0].jit(func);
  const auto B = _args[1].jit(func);

  switch (_type)
  {
    case BinaryFunctionType::ATAN2:
      return "std::atan2(" + A + ", " + B + ")";

    case BinaryFunctionType::HYPOT:
      return "std::sqrt((" + A + ")*(" + A + ") + (" + B + ")*(" + B + "))";

    case BinaryFunctionType::MIN:
      return "((" + A + ") < (" + B + ") ? (" + A + ") : (" + B + "))";

    case BinaryFunctionType::MAX:
      return "((" + A + ") > (" + B + ") ? (" + A + ") : (" + B + "))";

    case BinaryFunctionType::PLOG:
    {
      fatalError("Function not implemented");
      // return A < B
      //            ? std::log(B) + (A - B) / B - (A - B) * (A - B) / (2.0 * B * B) +
      //                  (A - B) * (A - B) * (A - B) / (3.0 * B * B * B)
      //            : std::log(A);
    }

    case BinaryFunctionType::POW:
      return "std::pow(" + A + ", " + B + ")";

    case BinaryFunctionType::POLAR:
    default:
      fatalError("Function not implemented");
  }
}

/********************************************************
 * Conditional Node
 ********************************************************/

JITReturnValue
ConditionalData::jit(JITStateValue & func)
{
  return "((" + _args[0].jit(func) + ") ? (" + _args[1].jit(func) + ") : (" + _args[2].jit(func) +
         "))";
}

/********************************************************
 * Integer power Node
 ********************************************************/

JITReturnValue
IntegerPowerData::jit(JITStateValue & func)
{
  // teplace this with a template
  return "std::pow(" + _arg.jit(func) + ", " + stringify(_exponent) + ")";
}

} // namespace SymbolicMath

#endif // SYMBOLICMATH_USE_CCODE
