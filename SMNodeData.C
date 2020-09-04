///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include "SMNodeData.h"
#include "SMUtils.h"
#include "SMTransform.h"

#include <cmath>
#include <iostream> // debug

namespace SymbolicMath
{

template <typename T>
void
NodeData<T>::stackDepth(std::pair<int, int> & current_max) const
{
  fatalError("stackDepth not implemented");
}

/********************************************************
 * Empty Data
 ********************************************************/

template <typename T>
void
EmptyData<T>::apply(Transform<T> & transform)
{
  fatalError("Cannot apply transform to an empty / invalid node");
}

/********************************************************
 * Unspecified Symbol
 ********************************************************/

template <typename T>
Node<T>
SymbolData<T>::D(const ValueProvider<T> & vp)
{
  if (!vp.is(this))
    return Node<T>(0.0);

  auto sd = static_cast<const SymbolData *>(&vp);
  if (sd->_name == "")
    fatalError("Cannot differentiate with respect to an anonymous value node");

  return _name == sd->_name ? Node<T>(1.0) : Node<T>(0.0);
}

template <typename T>
void
SymbolData<T>::apply(Transform<T> & transform)
{
  transform(this);
}

/********************************************************
 * Local Variable
 ********************************************************/

template <typename T>
T
LocalVariableData<T>::value() const
{
  fatalError("LocalVariable::value not yet implemented");
}

template <typename T>
void
LocalVariableData<T>::apply(Transform<T> & transform)
{
  transform(this);
}

/********************************************************
 * Real Number reference value provider
 ********************************************************/

template <typename T>
Node<T>
RealReferenceData<T>::D(const ValueProvider<T> & vp)
{
  if (!vp.is(this))
    return Node<T>(0.0);

  auto rrd = static_cast<const RealReferenceData *>(&vp);

  // std::cout << "d(" << format() << ")/d(" << vp.format()
  //           << ") = " << ((rrd && &(rrd->_ref) == &_ref) ? 1 : 0) << "   (" << rrd << ")\n";

  // check if the references refer to identical memory locations
  return (rrd && &(rrd->_ref) == &_ref) ? Node<T>(1.0) : Node<T>(0.0);
}

template <typename T>
void
RealReferenceData<T>::apply(Transform<T> & transform)
{
  transform(this);
}

/********************************************************
 * Real Number Array reference value provider
 ********************************************************/

template <typename T>
Node<T>
RealArrayReferenceData<T>::D(const ValueProvider<T> & vp)
{
  if (!vp.is(this))
    return Node<T>(0.0);

  auto rard = static_cast<const RealArrayReferenceData *>(&vp);

  // check if the reference and the index refer to identical memory locations
  // TODO: We could dynamically make this evaluate to "rrd->_index == _index"!
  return (rard && &(rard->_ref) == &_ref) && &(rard->_index) == &_index ? Node<T>(1.0)
                                                                        : Node<T>(0.0);
}

template <typename T>
void
RealArrayReferenceData<T>::apply(Transform<T> & transform)
{
  transform(this);
}

/********************************************************
 * Real Number
 ********************************************************/

template <typename T>
bool
RealNumberData<T>::is(NumberType type) const
{
  return _type == NumberType::REAL || _type == NumberType::_ANY;
}

template <typename T>
void
RealNumberData<T>::apply(Transform<T> & transform)
{
  transform(this);
}

/********************************************************
 * Unary Operator
 ********************************************************/

template <typename T>
T
UnaryOperatorData<T>::value() const
{
  switch (_type)
  {
    case UnaryOperatorType::PLUS:
      return _args[0].value();

    case UnaryOperatorType::MINUS:
      return -_args[0].value();

    default:
      fatalError("Unknown operator");
  }
}

template <typename T>
std::string
UnaryOperatorData<T>::format() const
{
  std::string form = stringify(_type);

  if (_args[0].precedence() > precedence())
    return ' ' + form + '(' + _args[0].format() + ')';
  else
    return ' ' + form + _args[0].format();
}

template <typename T>
std::string
UnaryOperatorData<T>::formatTree(std::string indent) const
{
  return indent + '{' + stringify(_type) + "}\n" + _args[0].formatTree(indent + "  ");
}

template <typename T>
NodeDataPtr<T>
UnaryOperatorData<T>::clone()
{
  return std::make_shared<UnaryOperatorData>(_type, _args[0]);
}

template <typename T>
Node<T>
UnaryOperatorData<T>::D(const ValueProvider<T> & vp)
{
  auto dA = _args[0].D(vp);
  switch (_type)
  {
    case UnaryOperatorType::PLUS:
      return dA;

    case UnaryOperatorType::MINUS:
      return -dA;

    default:
      fatalError("Unknown operator");
  }
}

template <typename T>
void
UnaryOperatorData<T>::apply(Transform<T> & transform)
{
  transform(this);
}

/********************************************************
 * Binary Operator Node
 ********************************************************/

template <typename T>
T
BinaryOperatorData<T>::value() const
{
  const auto A = _args[0].value();
  const auto B = _args[1].value();

  switch (_type)
  {
    case BinaryOperatorType::SUBTRACTION:
      return A - B;

    case BinaryOperatorType::DIVISION:
      return A / B;

    case BinaryOperatorType::POWER:
      return std::pow(A, B);

    case BinaryOperatorType::MODULO:
      return std::fmod(A, B);

    case BinaryOperatorType::LOGICAL_OR:
      return (A != 0.0 || B != 0.0) ? 1.0 : 0.0;

    case BinaryOperatorType::LOGICAL_AND:
      return (A != 0.0 && B != 0.0) ? 1.0 : 0.0;

    case BinaryOperatorType::LESS_THAN:
      return A < B ? 1.0 : 0.0;

    case BinaryOperatorType::GREATER_THAN:
      return A > B ? 1.0 : 0.0;

    case BinaryOperatorType::LESS_EQUAL:
      return A <= B ? 1.0 : 0.0;

    case BinaryOperatorType::GREATER_EQUAL:
      return A >= B ? 1.0 : 0.0;

    case BinaryOperatorType::EQUAL:
      return A == B ? 1.0 : 0.0;

    case BinaryOperatorType::NOT_EQUAL:
      return A != B ? 1.0 : 0.0;

    case BinaryOperatorType::LIST:
      fatalError("Not implemented");

    default:
      fatalError("Unknown operator");
  }
}

template <typename T>
std::string
BinaryOperatorData<T>::format() const
{
  std::string out;

  if (_args[0].precedence() > precedence())
    out = '(' + _args[0].format() + ')';
  else
    out = _args[0].format();

  out += ' ' + stringify(_type) + ' ';

  if (_args[1].precedence() > precedence() ||
      (_args[1].precedence() == precedence() &&
       (is(BinaryOperatorType::SUBTRACTION) || is(BinaryOperatorType::DIVISION) ||
        is(BinaryOperatorType::MODULO) || is(BinaryOperatorType::POWER))))
    out += '(' + _args[1].format() + ')';
  else
    out += _args[1].format();

  return out;
}

template <typename T>
std::string
BinaryOperatorData<T>::formatTree(std::string indent) const
{
  return indent + '{' + stringify(_type) + "}\n" + _args[0].formatTree(indent + "  ") +
         _args[1].formatTree(indent + "  ");
}

template <typename T>
NodeDataPtr<T>
BinaryOperatorData<T>::clone()
{
  return std::make_shared<BinaryOperatorData>(_type, _args[0], _args[1]);
}

template <typename T>
Node<T>
BinaryOperatorData<T>::D(const ValueProvider<T> & vp)
{
  auto & A = _args[0];
  auto & B = _args[1];
  auto dA = _args[0].D(vp);
  auto dB = _args[1].D(vp);

  switch (_type)
  {
    case BinaryOperatorType::SUBTRACTION:
      return dA - dB;

    case BinaryOperatorType::DIVISION:
      return dA / B - dB / Node<T>(IntegerPowerType::_ANY, B, 2);

    case BinaryOperatorType::MODULO:
      return dA;

    case BinaryOperatorType::POWER:
    {
      auto pfunc = Node<T>(BinaryFunctionType::POW, A, B);
      return pfunc.D(vp);
    }

    case BinaryOperatorType::LOGICAL_OR:
    case BinaryOperatorType::LOGICAL_AND:
    case BinaryOperatorType::LESS_THAN:
    case BinaryOperatorType::GREATER_THAN:
    case BinaryOperatorType::LESS_EQUAL:
    case BinaryOperatorType::GREATER_EQUAL:
    case BinaryOperatorType::EQUAL:
    case BinaryOperatorType::NOT_EQUAL:
      return Node<T>(0.0);

    default:
      fatalError("Derivative not implemented");
  }
}

template <typename T>
unsigned short
BinaryOperatorData<T>::precedence() const
{
  auto it = _binary_operators.find(_type);
  if (it == _binary_operators.end())
    fatalError("Unknown operator");

  return it->second._precedence;
}

template <typename T>
void
BinaryOperatorData<T>::apply(Transform<T> & transform)
{
  transform(this);
}

/********************************************************
 * Multinary Operator Node
 ********************************************************/

template <typename T>
T
MultinaryOperatorData<T>::value() const
{
  switch (_type)
  {
    case MultinaryOperatorType::ADDITION:
    {
      T sum = 0.0;
      for (auto & arg : _args)
        sum += arg.value();
      return sum;
    }

    case MultinaryOperatorType::MULTIPLICATION:
    {
      T product = 1.0;
      for (auto & arg : _args)
        product *= arg.value();
      return product;
    }

    case MultinaryOperatorType::LIST:
    {
      T res = 0.0;
      // evaluate all arguments of the list, but return only the last
      for (auto & arg : _args)
        res = arg.value();
      return res;
    }

    default:
      fatalError("Unknown operator");
  }
}

template <typename T>
std::string
MultinaryOperatorData<T>::format() const
{
  const auto form = stringify(_type);
  std::string out;

  for (auto & arg : _args)
  {
    if (!out.empty())
      out += ' ' + form + ' ';

    if (arg.precedence() > precedence())
      out += '(' + arg.format() + ')';
    else
      out += arg.format();
  }

  return out;
}

template <typename T>
std::string
MultinaryOperatorData<T>::formatTree(std::string indent) const
{
  std::string out = indent + '{' + stringify(_type) + "}\n";
  for (auto & arg : _args)
    out += arg.formatTree(indent + "  ");
  return out;
}

template <typename T>
NodeDataPtr<T>
MultinaryOperatorData<T>::clone()
{
  std::vector<Node<T>> cloned_args;
  for (auto & arg : _args)
    cloned_args.push_back(arg);
  return std::make_shared<MultinaryOperatorData>(_type, cloned_args);
}

template <typename T>
Node<T>
MultinaryOperatorData<T>::D(const ValueProvider<T> & vp)
{
  std::vector<Node<T>> new_args;

  switch (_type)
  {
    case MultinaryOperatorType::ADDITION:
      for (auto & arg : _args)
        new_args.push_back(arg.D(vp));
      return Node<T>(_type, new_args);

    case MultinaryOperatorType::MULTIPLICATION:
    {
      const auto nargs = _args.size();
      std::vector<Node<T>> summands;
      for (std::size_t i = 0; i < nargs; ++i)
      {
        std::vector<Node<T>> factors;
        for (std::size_t j = 0; j < nargs; ++j)
          factors.push_back(i == j ? _args[j].D(vp) : _args[j]);
        summands.push_back(Node<T>(MultinaryOperatorType::MULTIPLICATION, factors));
      }

      return Node<T>(MultinaryOperatorType::ADDITION, summands);
    }

    default:
      fatalError("Derivative not implemented");
  }
}

template <typename T>
unsigned short
MultinaryOperatorData<T>::precedence() const
{
  auto it = _multinary_operators.find(_type);
  if (it == _multinary_operators.end())
    fatalError("Unknown operator");

  return it->second._precedence;
}

template <typename T>
void
MultinaryOperatorData<T>::apply(Transform<T> & transform)
{
  transform(this);
}

/********************************************************
 * Unary Function Node
 ********************************************************/

template <typename T>
T
UnaryFunctionData<T>::value() const
{
  const auto A = _args[0].value();

  switch (_type)
  {
    case UnaryFunctionType::ABS:
      return std::abs(A);

    case UnaryFunctionType::ACOS:
      return std::acos(A);

    case UnaryFunctionType::ACOSH:
      return std::acosh(A);

    case UnaryFunctionType::ASIN:
      return std::asin(A);

    case UnaryFunctionType::ASINH:
      return std::asinh(A);

    case UnaryFunctionType::ATAN:
      return std::atan(A);

    case UnaryFunctionType::ATANH:
      return std::atanh(A);

    case UnaryFunctionType::CBRT:
      return std::cbrt(A);

    case UnaryFunctionType::CEIL:
      return std::ceil(A);

    case UnaryFunctionType::COS:
      return std::cos(A);

    case UnaryFunctionType::COSH:
      return std::cosh(A);

    case UnaryFunctionType::COT:
      return 1.0 / std::tan(A);

    case UnaryFunctionType::CSC:
      return 1.0 / std::sin(A);

    case UnaryFunctionType::ERF:
      return std::erf(A);

    case UnaryFunctionType::ERFC:
      return std::erfc(A);

    case UnaryFunctionType::EXP:
      return std::exp(A);

    case UnaryFunctionType::EXP2:
      return std::exp2(A);

    case UnaryFunctionType::FLOOR:
      return std::floor(A);

    case UnaryFunctionType::INT:
      return std::round(A);

    case UnaryFunctionType::LOG:
      return std::log(A);

    case UnaryFunctionType::LOG10:
      return std::log10(A);

    case UnaryFunctionType::LOG2:
      return std::log2(A);

    case UnaryFunctionType::SEC:
      return 1.0 / std::cos(A);

    case UnaryFunctionType::SIN:
      return std::sin(A);

    case UnaryFunctionType::SINH:
      return std::sinh(A);

    case UnaryFunctionType::SQRT:
      return std::sqrt(A);

    case UnaryFunctionType::TAN:
      return std::tan(A);

    case UnaryFunctionType::TANH:
      return std::tanh(A);

    case UnaryFunctionType::TRUNC:
      return static_cast<int>(A);

    default:
      fatalError("Function not implemented");
  }
}

template <typename T>
std::string
UnaryFunctionData<T>::format() const
{
  return stringify(_type) + "(" + _args[0].format() + ")";
}

template <typename T>
std::string
UnaryFunctionData<T>::formatTree(std::string indent) const
{
  return indent + stringify(_type) + '\n' + _args[0].formatTree(indent + "  ");
}

template <typename T>
NodeDataPtr<T>
UnaryFunctionData<T>::clone()
{
  return std::make_shared<UnaryFunctionData>(_type, _args[0]);
}

template <typename T>
Node<T>
UnaryFunctionData<T>::D(const ValueProvider<T> & vp)
{
  auto & A = _args[0];
  auto dA = _args[0].D(vp);

  switch (_type)
  {
    case UnaryFunctionType::ABS:
      return dA * A / Node<T>(_type, A);

    case UnaryFunctionType::ACOS:
      return -dA * Node<T>(BinaryOperatorType::POWER,
                           Node<T>(1.0) - Node<T>(IntegerPowerType::_ANY, A, 2),
                           Node<T>(-0.5));

    case UnaryFunctionType::ACOSH:
      return dA * Node<T>(BinaryOperatorType::POWER,
                          Node<T>(IntegerPowerType::_ANY, A, 2) - Node<T>(1.0),
                          Node<T>(-0.5));

    case UnaryFunctionType::ARG:
      fatalError("Derivative not implemented");

    case UnaryFunctionType::ASIN:
      return dA * Node<T>(BinaryOperatorType::POWER,
                          Node<T>(1.0) - Node<T>(IntegerPowerType::_ANY, A, 2),
                          Node<T>(-0.5));

    case UnaryFunctionType::ASINH:
      return dA * Node<T>(BinaryOperatorType::POWER,
                          Node<T>(1.0) + Node<T>(IntegerPowerType::_ANY, A, 2),
                          Node<T>(-0.5));

    case UnaryFunctionType::ATAN:
      return dA / (Node<T>(IntegerPowerType::_ANY, A, 2) + Node<T>(1.0));

    case UnaryFunctionType::ATANH:
      return dA / (Node<T>(1.0) - Node<T>(IntegerPowerType::_ANY, A, 2));

    case UnaryFunctionType::CBRT:
      return Node<T>(1.0 / 3.0) * Node<T>(IntegerPowerType::_ANY, Node<T>(_type, A), -2);

    case UnaryFunctionType::CEIL:
      fatalError("Derivative not implemented");

    case UnaryFunctionType::CONJ:
      fatalError("Derivative not implemented");

    case UnaryFunctionType::COS:
      return -dA * Node<T>(UnaryFunctionType::SIN, A);

    case UnaryFunctionType::COSH:
      return dA * Node<T>(UnaryFunctionType::SINH, A);

    case UnaryFunctionType::COT:
      // 1 / tan
      return -dA * Node<T>(IntegerPowerType::_ANY, Node<T>(UnaryFunctionType::CSC, A), 2);

    case UnaryFunctionType::CSC:
      // 1 / sin
      return -dA * Node<T>(UnaryFunctionType::COT, A) / Node<T>(UnaryFunctionType::SIN, A);

    case UnaryFunctionType::ERF:
      return dA * Node<T>(2.0 / std::sqrt(Constant::pi)) *
             Node<T>(UnaryFunctionType::EXP, -Node<T>(IntegerPowerType::_ANY, A, 2));

    case UnaryFunctionType::ERFC: // erfc = 1 - erf -> d erfc = -d erf
      return -dA * Node<T>(2.0 / std::sqrt(Constant::pi)) *
             Node<T>(UnaryFunctionType::EXP, -Node<T>(IntegerPowerType::_ANY, A, 2));

    case UnaryFunctionType::EXP: // d exp(A) = dA*exp(A)
      return dA * Node<T>(_type, A);

    case UnaryFunctionType::EXP2:
      return dA * Node<T>(Constant::ln2) * Node<T>(_type, A);

    case UnaryFunctionType::FLOOR:
      fatalError("Derivative not implemented");

    case UnaryFunctionType::IMAG:
      fatalError("Derivative not implemented");

    case UnaryFunctionType::INT:
      fatalError("Derivative not implemented");

    case UnaryFunctionType::LOG: // d log(A) = dA/A
      return dA / A;

    case UnaryFunctionType::LOG2:
      return dA / (A * Node<T>(Constant::ln2));

    case UnaryFunctionType::LOG10:
      return dA / (A * Node<T>(Constant::ln10));

    case UnaryFunctionType::SEC:
      // 1 / cos
      return dA * Node<T>(UnaryFunctionType::TAN, A) / Node<T>(UnaryFunctionType::COS, A);

    case UnaryFunctionType::SIN: // d sin(A) = dA*cos(A)
      return dA * Node<T>(UnaryFunctionType::COS, A);

    case UnaryFunctionType::SINH:
      return dA * Node<T>(UnaryFunctionType::COSH, A);

    case UnaryFunctionType::SQRT:
      return Node<T>(0.5) / Node<T>(_type, A);

    case UnaryFunctionType::TAN:
      return dA * Node<T>(IntegerPowerType::_ANY, Node<T>(UnaryFunctionType::SEC, A), 2);

    case UnaryFunctionType::TANH:
      return dA * (Node<T>(1.0) -
                   Node<T>(IntegerPowerType::_ANY, Node<T>(UnaryFunctionType::TANH, A), 2));

    default:
      fatalError("Derivative not implemented");
  }
}

template <typename T>
void
UnaryFunctionData<T>::apply(Transform<T> & transform)
{
  transform(this);
}

/********************************************************
 * Binary Function Node
 ********************************************************/

template <typename T>
T
BinaryFunctionData<T>::value() const
{
  const auto A = _args[0].value();
  const auto B = _args[1].value();

  switch (_type)
  {
    case BinaryFunctionType::ATAN2:
      return std::atan2(A, B);

    case BinaryFunctionType::HYPOT:
      return std::sqrt(A * A + B * B);

    case BinaryFunctionType::MIN:
      return std::min(A, B);

    case BinaryFunctionType::MAX:
      return std::max(A, B);

    case BinaryFunctionType::PLOG:
      return A < B ? std::log(B) + (A - B) / B - (A - B) * (A - B) / (2.0 * B * B) +
                         (A - B) * (A - B) * (A - B) / (3.0 * B * B * B)
                   : std::log(A);

    case BinaryFunctionType::POW:
      return std::pow(A, B);

    case BinaryFunctionType::POLAR:
    default:
      fatalError("Function not implemented");
  }
}

template <typename T>
std::string
BinaryFunctionData<T>::format() const
{
  return stringify(_type) + "(" + _args[0].format() + ", " + _args[1].format() + ")";
}

template <typename T>
std::string
BinaryFunctionData<T>::formatTree(std::string indent) const
{
  return indent + stringify(_type) + '\n' + _args[0].formatTree(indent + "  ") +
         _args[1].formatTree(indent + "  ");
}

template <typename T>
NodeDataPtr<T>
BinaryFunctionData<T>::clone()
{
  return std::make_shared<BinaryFunctionData>(_type, _args[0], _args[1]);
}

template <typename T>
Node<T>
BinaryFunctionData<T>::D(const ValueProvider<T> & vp)
{
  auto & A = _args[0];
  auto & B = _args[1];
  auto dA = _args[0].D(vp);
  auto dB = _args[1].D(vp);

  switch (_type)
  {
    case BinaryFunctionType::ATAN2:
      return (B * dA - A * dB) /
             (Node<T>(IntegerPowerType::_ANY, A, 2) + Node<T>(IntegerPowerType::_ANY, B, 2));

    case BinaryFunctionType::MIN:
      return Node<T>(ConditionalType::IF, A < B, dA, dB);

    case BinaryFunctionType::MAX:
      return Node<T>(ConditionalType::IF, A < B, dB, dA);

    case BinaryFunctionType::PLOG:
      return dA * Node<T>(ConditionalType::IF,
                          A < B,
                          Node<T>(1.0) / B - (A - B) / Node<T>(IntegerPowerType::_ANY, B, 2) +
                              Node<T>(IntegerPowerType::_ANY, A - B, 2) /
                                  Node<T>(IntegerPowerType::_ANY, B, 3),
                          Node<T>(1.0) / A);

    case BinaryFunctionType::POW:
      if (B.is(NumberType::_ANY))
      {
        if (B.is(1.0))
          return dB;
        else if (B.is(0.0))
          return Node<T>(0.0);

        return Node<T>(_type, A, B - Node<T>(1.0)) * B * dA;
      }

      return Node<T>(_type, A, B) * (dB * Node<T>(UnaryFunctionType::LOG, A) + B * dA / A);

    default:
      fatalError("Derivative not implemented");
  }
}

template <typename T>
void
BinaryFunctionData<T>::apply(Transform<T> & transform)
{
  transform(this);
}

/********************************************************
 * Conditional Node
 ********************************************************/

template <typename T>
T
ConditionalData<T>::value() const
{
  if (_type != ConditionalType::IF)
    fatalError("Conditional not implemented");

  if (_args[0].value() != 0.0)
    // true expression
    return _args[1].value();
  else
    // false expression
    return _args[2].value();
}

template <typename T>
std::string
ConditionalData<T>::format() const
{
  return stringify(_type) + "(" + _args[0].format() + ", " + _args[1].format() + ", " +
         _args[2].format() + ")";
}

template <typename T>
std::string
ConditionalData<T>::formatTree(std::string indent) const
{
  return indent + stringify(_type) + '\n' + _args[0].formatTree(indent + "  ") + indent + "do\n" +
         _args[1].formatTree(indent + "  ") + indent + "otherwise\n" +
         _args[2].formatTree(indent + "  ");
}

template <typename T>
NodeDataPtr<T>
ConditionalData<T>::clone()
{
  return std::make_shared<ConditionalData>(_type, _args[0], _args[1], _args[2]);
}

template <typename T>
Node<T>
ConditionalData<T>::D(const ValueProvider<T> & vp)
{
  if (_type != ConditionalType::IF)
    fatalError("Conditional not implemented");

  return Node<T>(ConditionalType::IF, _args[0], _args[1].D(vp), _args[2].D(vp));
}

template <typename T>
void
ConditionalData<T>::stackDepth(std::pair<int, int> & current_max) const
{
  // condition
  _args[0].stackDepth(current_max);

  current_max.first--;
  auto true_branch = current_max;
  _args[1].stackDepth(true_branch);
  auto false_branch = current_max;
  _args[2].stackDepth(false_branch);

  // stack pointer needs to be at the same position after each branch
  if (true_branch.first != false_branch.first)
    fatalError("Malformed conditional subtrees");

  // find maximum stack depth the two branches
  current_max = true_branch;
  if (false_branch.second > true_branch.second)
    current_max.second = false_branch.second;
}

template <typename T>
void
ConditionalData<T>::apply(Transform<T> & transform)
{
  transform(this);
}

/********************************************************
 * Integer power Node
 ********************************************************/

template <typename T>
T
IntegerPowerData<T>::value() const
{
  auto x = _arg.value();
  T result = 1.0;

  int e = std::abs(_exponent);
  while (true)
  {
    if (e & 1)
      result *= x;
    x *= x;
    if (e == 0)
      break;
    e >>= 1;
  }

  if (_exponent < 0)
    return 1.0 / result;
  return result;
}

template <typename T>
std::string
IntegerPowerData<T>::format() const
{
  return "ipow(" + _arg.format() + ", " + std::to_string(_exponent) + ")";
}

template <typename T>
std::string
IntegerPowerData<T>::formatTree(std::string indent) const
{
  return indent + "ipow" + _arg.formatTree(indent + "  ") + indent + std::to_string(_exponent) +
         '\n';
}

template <typename T>
Node<T>
IntegerPowerData<T>::getArg(unsigned int i)
{
  if (i == 0)
    return _arg;
  if (i == 1)
    return Node<T>(_exponent);
  fatalError("Requesting invalid argument");
}

template <typename T>
Node<T>
IntegerPowerData<T>::D(const ValueProvider<T> & vp)
{
  auto & A = _arg;
  auto dA = _arg.D(vp);
  return dA * Node<T>(_exponent) * Node<T>(IntegerPowerType::_ANY, A, _exponent - 1);
}

template <typename T>
void
IntegerPowerData<T>::apply(Transform<T> & transform)
{
  transform(this);
}

template class SymbolData<Real>;
template class LocalVariableData<Real>;
template class RealReferenceData<Real>;
template class RealArrayReferenceData<Real>;
template class RealNumberData<Real>;
template class UnaryOperatorData<Real>;
template class BinaryOperatorData<Real>;
template class MultinaryOperatorData<Real>;
template class UnaryFunctionData<Real>;
template class BinaryFunctionData<Real>;
template class ConditionalData<Real>;
template class IntegerPowerData<Real>;

} // namespace SymbolicMath
