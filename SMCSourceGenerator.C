///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include "SMCSourceGenerator.h"

#include <stdio.h>
#include <fstream>
#include <cstdio>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <dlfcn.h>

#define CCODE_JIT_COMPILER "g++"

namespace SymbolicMath
{

template <>
const std::string
CSourceGenerator<Real>::typeName()
{
  return "double";
}

template <typename T>
CSourceGenerator<T>::CSourceGenerator(Function<T> & fb) : Transform<T>(fb)
{
  apply();
}

template <typename T>
void
CSourceGenerator<T>::operator()(SymbolData<T> * n)
{
  fatalError("Symbol in compiled function");
}

template <typename T>
void
CSourceGenerator<T>::operator()(UnaryOperatorData<T> * n)
{
  n->_args[0].apply(*this);

  switch (n->_type)
  {
    case UnaryOperatorType::PLUS:
      return;

    case UnaryOperatorType::MINUS:
      _source = "-" + _source;
      return;

    default:
      fatalError("Unknown operator");
  }
}

template <typename T>
void
CSourceGenerator<T>::operator()(BinaryOperatorData<T> * n)
{
  n->_args[0].apply(*this);
  std::string A;
  std::swap(_source, A);

  n->_args[1].apply(*this);
  const auto & B = _source;

  switch (n->_type)
  {
    case BinaryOperatorType::SUBTRACTION:
      _source = "(" + A + ") - (" + B + ")";
      return;

    case BinaryOperatorType::DIVISION:
      _source = "(" + A + ") / (" + B + ")";
      return;

    case BinaryOperatorType::MODULO:
      _source = "std::fmod(" + A + ", " + B + ")";
      return;

    case BinaryOperatorType::POWER:
      _source = "std::pow(" + A + ", " + B + ")";
      return;

    case BinaryOperatorType::LOGICAL_OR:
      _source = "static_cast<" + typeName() + ">(bool(" + A + ") || bool(" + B + "))";
      return;

    case BinaryOperatorType::LOGICAL_AND:
      _source = "static_cast<" + typeName() + ">(bool(" + A + ") && bool(" + B + "))";
      return;

    case BinaryOperatorType::LESS_THAN:
      _source = "static_cast<" + typeName() + ">((" + A + ") < (" + B + "))";
      return;

    case BinaryOperatorType::GREATER_THAN:
      _source = "static_cast<" + typeName() + ">((" + A + ") > (" + B + "))";
      return;

    case BinaryOperatorType::LESS_EQUAL:
      _source = "static_cast<" + typeName() + ">((" + A + ") <= (" + B + "))";
      return;

    case BinaryOperatorType::GREATER_EQUAL:
      _source = "static_cast<" + typeName() + ">((" + A + ") >= (" + B + "))";
      return;

    case BinaryOperatorType::EQUAL:
      _source = "static_cast<" + typeName() + ">((" + A + ") == (" + B + "))";
      return;

    case BinaryOperatorType::NOT_EQUAL:
      _source = "static_cast<" + typeName() + ">((" + A + ") != (" + B + "))";
      return;

    default:
      fatalError("Unknown operator");
  }
}

template <typename T>
void
CSourceGenerator<T>::operator()(MultinaryOperatorData<T> * n)
{
  auto nargs = n->_args.size();
  if (nargs == 0)
    fatalError("No child nodes in multinary operator");

  char op;
  switch (n->_type)
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

  if (nargs == 1)
    n->_args[0].apply(*this);
  else
  {
    std::string out;
    for (std::size_t i = 0; i < nargs; ++i)
    {
      n->_args[i].apply(*this);
      if (i)
        out += op;
      out += '(' + _source + ')';
      _source = "";
    }
    _source = out;
  }
}

template <typename T>
void
CSourceGenerator<T>::operator()(UnaryFunctionData<T> * n)
{
  n->_args[0].apply(*this);
  const auto & A = _source;

  switch (n->_type)
  {
    case UnaryFunctionType::ABS:
      _source = "std::abs(" + A + ")";
      return;

    case UnaryFunctionType::ACOS:
      _source = "std::acos(" + A + ")";
      return;

    case UnaryFunctionType::ACOSH:
      _source = "std::acosh(" + A + ")";
      return;

    case UnaryFunctionType::ARG:
      fatalError("Function not implemented");

    case UnaryFunctionType::ASIN:
      _source = "std::asin(" + A + ")";
      return;

    case UnaryFunctionType::ASINH:
      _source = "std::asinh(" + A + ")";
      return;

    case UnaryFunctionType::ATAN:
      _source = "std::atan(" + A + ")";
      return;

    case UnaryFunctionType::ATANH:
      _source = "std::atanh(" + A + ")";
      return;

    case UnaryFunctionType::CBRT:
      _source = "std::cbrt(" + A + ")";
      return;

    case UnaryFunctionType::CEIL:
      _source = "std::ceil(" + A + ")";
      return;

    case UnaryFunctionType::CONJ:
      fatalError("Function not implemented");

    case UnaryFunctionType::COS:
      _source = "std::cos(" + A + ")";
      return;

    case UnaryFunctionType::COSH:
      _source = "std::cosh(" + A + ")";
      return;

    case UnaryFunctionType::COT:
      _source = "1.0 / std::tan(" + A + ")";
      return;

    case UnaryFunctionType::CSC:
      _source = "1.0 / std::sin(" + A + ")";
      return;

    case UnaryFunctionType::ERF:
      _source = "std::erf(" + A + ")";
      return;

    case UnaryFunctionType::ERFC:
      _source = "std::erfc(" + A + ")";
      return;

    case UnaryFunctionType::EXP:
      _source = "std::exp(" + A + ")";
      return;

    case UnaryFunctionType::EXP2:
      _source = "std::exp2(" + A + ")";
      return;

    case UnaryFunctionType::FLOOR:
      _source = "std::floor(" + A + ")";
      return;

    case UnaryFunctionType::IMAG:
      fatalError("Function not implemented");

    case UnaryFunctionType::INT:
      _source = "std::round(" + A + ")";
      return;

    case UnaryFunctionType::LOG:
      _source = "std::log(" + A + ")";
      return;

    case UnaryFunctionType::LOG10:
      _source = "std::log10(" + A + ")";
      return;

    case UnaryFunctionType::LOG2:
      _source = "std::log2(" + A + ")";
      return;

    case UnaryFunctionType::REAL:
      fatalError("Function not implemented");

    case UnaryFunctionType::SEC:
      _source = "1.0 / std::cos(" + A + ")";
      return;

    case UnaryFunctionType::SIN:
      _source = "std::sin(" + A + ")";
      return;

    case UnaryFunctionType::SINH:
      _source = "std::sinh(" + A + ")";
      return;

    case UnaryFunctionType::SQRT:
      _source = "std::sqrt(" + A + ")";
      return;

    case UnaryFunctionType::T:
      fatalError("Function not implemented");

    case UnaryFunctionType::TAN:
      _source = "std::tan(" + A + ")";
      return;

    case UnaryFunctionType::TANH:
      _source = "std::tanh(" + A + ")";
      return;

    case UnaryFunctionType::TRUNC:
      _source = "static_cast<int>(" + A + ")";
      return;

    default:
      fatalError("Function not implemented");
  }
}

template <typename T>
void
CSourceGenerator<T>::operator()(BinaryFunctionData<T> * n)
{
  n->_args[0].apply(*this);
  std::string A;
  std::swap(_source, A);

  n->_args[1].apply(*this);
  const auto & B = _source;

  switch (n->_type)
  {
    case BinaryFunctionType::ATAN2:
      _source = "std::atan2(" + A + ", " + B + ")";
      return;

    case BinaryFunctionType::HYPOT:
      _source = "std::sqrt((" + A + ")*(" + A + ") + (" + B + ")*(" + B + "))";
      return;

    case BinaryFunctionType::MIN:
      _source = "((" + A + ") < (" + B + ") ? (" + A + ") : (" + B + "))";
      return;

    case BinaryFunctionType::MAX:
      _source = "((" + A + ") > (" + B + ") ? (" + A + ") : (" + B + "))";
      return;

    case BinaryFunctionType::PLOG:
      _source = "((" + A + ") < (" + B + ") ? (std::log(" + B + ") + (" + A + " - " + B + ") / " +
                B + " - (" + A + " - " + B + ") * (" + A + " - " + B + ") / (2.0 * " + B + " * " +
                B + ") + (" + A + " - " + B + ") * (" + A + " - " + B + ") * (" + A + " - " + B +
                ") / (3.0 * " + B + " * " + B + " * " + B + ")) : std::log(" + A + "))";
      return;

    case BinaryFunctionType::POW:
      _source = "std::pow(" + A + ", " + B + ")";
      return;

    case BinaryFunctionType::POLAR:
    default:
      fatalError("Function not implemented");
  }
}

template <typename T>
void
CSourceGenerator<T>::operator()(RealNumberData<T> * n)
{
  _source = stringify(n->_value);
}

template <typename T>
void
CSourceGenerator<T>::operator()(RealReferenceData<T> * n)
{
  // will need template specializations
  _source = "*(reinterpret_cast<" + typeName() + " *>(" +
            std::to_string(reinterpret_cast<long>(&n->_ref)) + "))";
}

template <typename T>
void
CSourceGenerator<T>::operator()(RealArrayReferenceData<T> * n)
{
  fatalError("Not implemented");
}

template <typename T>
void
CSourceGenerator<T>::operator()(LocalVariableData<T> * n)
{
  fatalError("Not implemented");
}

template <typename T>
void
CSourceGenerator<T>::operator()(ConditionalData<T> * n)
{
  n->_args[0].apply(*this);
  std::string A;
  std::swap(_source, A);

  n->_args[1].apply(*this);
  std::string B;
  std::swap(_source, B);

  n->_args[1].apply(*this);
  const auto & C = _source;

  _source = "((" + A + ") ? (" + B + ") : (" + C + "))";
}

template <typename T>
void
CSourceGenerator<T>::operator()(IntegerPowerData<T> * n)
{
  // replace this with a template
  n->_arg.apply(*this);
  _source = "std::pow(" + _source + ", " + stringify(n->_exponent) + ")";
}

template class CSourceGenerator<Real>;

} // namespace SymbolicMath
