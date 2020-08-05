///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include "SymbolicMathSymbols.h"

namespace SymbolicMath
{

std::string
stringify(UnaryOperatorType type)
{
  auto it = _unary_operators.find(type);
  if (it == _unary_operators.end())
    fatalError("Unknown operator");

  return it->second._form;
}

std::string
stringify(BinaryOperatorType type)
{
  auto it = _binary_operators.find(type);
  if (it == _binary_operators.end())
    fatalError("Unknown operator");

  return it->second._form;
}

std::string
stringify(MultinaryOperatorType type)
{
  switch (type)
  {
    case MultinaryOperatorType::ADDITION:
      return "+";

    case MultinaryOperatorType::MULTIPLICATION:
      return "*";

    case MultinaryOperatorType::LIST:
      return "; ";

    default:
      fatalError("Unknown operator");
  }
}

std::string
stringify(UnaryFunctionType type)
{
  auto it = _unary_functions.find(type);
  if (it == _unary_functions.end())
    fatalError("Unknown function");

  return it->second;
}

std::string
stringify(BinaryFunctionType type)
{
  auto it = _binary_functions.find(type);
  if (it == _binary_functions.end())
    fatalError("Unknown function");

  return it->second;
}

std::string
stringify(ConditionalType type)
{
  if (type == ConditionalType::IF)
    return "if";

  fatalError("Unknown conditional");
}

} // namespace SymbolicMath
