#include "SymbolicMathNodeData.h"
#include "SymbolicMathUtils.h"

#include <cmath>
#include <iostream> // debug

namespace SymbolicMath
{

void
NodeData::stackDepth(std::pair<int, int> & current_max)
{
  fatalError("stackDepth not implemented");
}

std::string
ValueProvider::formatTree(std::string indent) const
{
  return indent + (_name != "" ? _name : "{V}") + '\n';
}

Node
SymbolData::D(const ValueProvider & vp)
{
  if (!vp.is(this))
    return Node(0.0);

  auto sd = static_cast<const SymbolData *>(&vp);
  if (sd->_name == "")
    fatalError("Cannot differentiate with respect to an anonymous value node");

  return _name == sd->_name ? Node(1.0) : Node(0.0);
}

/********************************************************
 * Real Number reference value provider
 ********************************************************/

Node
RealReferenceData::D(const ValueProvider & vp)
{
  if (!vp.is(this))
    return Node(0.0);

  auto rrd = static_cast<const RealReferenceData *>(&vp);

  // std::cout << "d(" << format() << ")/d(" << vp.format()
  //           << ") = " << ((rrd && &(rrd->_ref) == &_ref) ? 1 : 0) << "   (" << rrd << ")\n";

  // check if the references refer to identical memory locations
  return (rrd && &(rrd->_ref) == &_ref) ? Node(1.0) : Node(0.0);
}

/********************************************************
 * Real Number Array reference value provider
 ********************************************************/

Node
RealArrayReferenceData::D(const ValueProvider & vp)
{
  if (!vp.is(this))
    return Node(0.0);

  auto rard = static_cast<const RealArrayReferenceData *>(&vp);

  // check if the reference and the index refer to identical memory locations
  // TODO: We could dynamically make this evaluate to "rrd->_index == _index"!
  return (rard && &(rard->_ref) == &_ref) && &(rard->_index) == &_index ? Node(1.0) : Node(0.0);
}

/********************************************************
 * Real Number
 ********************************************************/

std::string
RealNumberData::formatTree(std::string indent) const
{
  return indent + stringify(_value) + '\n';
}

bool
RealNumberData::is(NumberType type) const
{
  return _type == NumberType::REAL || _type == NumberType::_ANY;
}

/********************************************************
 * Unary Operator
 ********************************************************/

Real
UnaryOperatorData::value()
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

std::string
UnaryOperatorData::format() const
{
  std::string form = stringify(_type);

  if (_args[0].precedence() > precedence())
    return ' ' + form + '(' + _args[0].format() + ')';
  else
    return ' ' + form + _args[0].format();
}

std::string
UnaryOperatorData::formatTree(std::string indent) const
{
  return indent + '{' + stringify(_type) + "}\n" + _args[0].formatTree(indent + "  ");
}

NodeDataPtr
UnaryOperatorData::clone()
{
  return std::make_shared<UnaryOperatorData>(_type, _args[0]);
}

Node
UnaryOperatorData::simplify()
{
  _args[0].simplify();
  if (_args[0].is(NumberType::_ANY))
    return Node(value());

  return Node();
}

Node
UnaryOperatorData::D(const ValueProvider & vp)
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

/********************************************************
 * Binary Operator Node
 ********************************************************/

Real
BinaryOperatorData::value()
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

    default:
      fatalError("Unknown operator");
  }
}

std::string
BinaryOperatorData::format() const
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

std::string
BinaryOperatorData::formatTree(std::string indent) const
{
  return indent + '{' + stringify(_type) + "}\n" + _args[0].formatTree(indent + "  ") +
         _args[1].formatTree(indent + "  ");
}

NodeDataPtr
BinaryOperatorData::clone()
{
  return std::make_shared<BinaryOperatorData>(_type, _args[0], _args[1]);
}

Node
BinaryOperatorData::simplify()
{
  // constant folding
  _args[0].simplify();
  _args[1].simplify();
  if (_args[0].is(NumberType::_ANY) && _args[1].is(NumberType::_ANY))
    return Node(value());

  switch (_type)
  {
    case BinaryOperatorType::SUBTRACTION:
      // 0 - b = -b
      if (_args[0].is(0.0))
        return Node(UnaryOperatorType::MINUS, _args[1]);

      // a - 0 = a
      else if (_args[1].is(0.0))
        return _args[0];

      return Node();

    case BinaryOperatorType::DIVISION:
      // a/1 = a
      if (_args[1].is(1.0))
        return _args[0];

      // 0/b = 0
      if (_args[0].is(0.0))
        return Node(0.0);

      return Node();

    case BinaryOperatorType::POWER:
    {
      // turn operator into function and simplify
      auto pfunc = Node(BinaryFunctionType::POW, _args[0], _args[1]);
      pfunc.simplify();
      return pfunc;
    }

      // for this to be an optimization we'll need to make those operators multinary and only
      // replace size>2 instances
      //
      // case BinaryOperatorType::LOGICAL_OR:
      //   if (_args[0].is(NumberType::_ANY))
      //   {
      //     if (_args[0].value() != 0)
      //       return Node(1.0);
      //     else
      //       return _args[1] != Node(0.0);
      //   }
      //   else if (_args[1].is(NumberType::_ANY))
      //   {
      //     if (_args[1].value() != 0)
      //       return Node(1.0);
      //     else
      //       return _args[0] != Node(0.0);
      //   }
      //
      // case BinaryOperatorType::LOGICAL_AND:
      //   if (_args[0].is(NumberType::_ANY))
      //   {
      //     if (_args[0].value() != 0)
      //       return _args[1] != Node(0.0);
      //     else
      //       return Node(0.0);
      //   }
      //   else if (_args[1].is(NumberType::_ANY))
      //   {
      //     if (_args[1].value() != 0)
      //       return _args[0] != Node(0.0);
      //     else
      //       return Node(0.0);
      //   }

    default:
      return Node();
  }
}

Node
BinaryOperatorData::D(const ValueProvider & vp)
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
      return dA / B - dB / Node(IntegerPowerType::_ANY, B, 2);

    case BinaryOperatorType::MODULO:
      return dA;

    case BinaryOperatorType::POWER:
    {
      auto pfunc = Node(BinaryFunctionType::POW, A, B);
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
      return Node(0.0);

    default:
      fatalError("Derivative not implemented");
  }
}

unsigned short
BinaryOperatorData::precedence() const
{
  auto it = _binary_operators.find(_type);
  if (it == _binary_operators.end())
    fatalError("Unknown operator");

  return it->second._precedence;
}

/********************************************************
 * Multinary Operator Node
 ********************************************************/

Real
MultinaryOperatorData::value()
{
  switch (_type)
  {
    case MultinaryOperatorType::ADDITION:
    {
      Real sum = 0.0;
      for (auto & arg : _args)
        sum += arg.value();
      return sum;
    }

    case MultinaryOperatorType::MULTIPLICATION:
    {
      Real product = 1.0;
      for (auto & arg : _args)
        product *= arg.value();
      return product;
    }

    default:
      fatalError("Unknown operator");
  }
}

std::string
MultinaryOperatorData::format() const
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

std::string
MultinaryOperatorData::formatTree(std::string indent) const
{
  std::string out = indent + '{' + stringify(_type) + "}\n";
  for (auto & arg : _args)
    out += arg.formatTree(indent + "  ");
  return out;
}

NodeDataPtr
MultinaryOperatorData::clone()
{
  std::vector<Node> cloned_args;
  for (auto & arg : _args)
    cloned_args.push_back(arg);
  return std::make_shared<MultinaryOperatorData>(_type, cloned_args);
}

void
MultinaryOperatorData::simplifyHelper(std::vector<Node> & new_args, Node arg)
{
  if (arg.is(NumberType::_ANY))
  {
    if (new_args.empty() || !new_args[0].is(NumberType::_ANY))
      new_args.insert(new_args.begin(), arg);
    else
    {
      auto val = new_args[0].value();
      switch (_type)
      {
        case MultinaryOperatorType::ADDITION:
          val += arg.value();
          break;

        case MultinaryOperatorType::MULTIPLICATION:
          val *= arg.value();
          break;

        default:
          fatalError("Unknown multinary operator");
      }
      new_args[0] = Node(val);
    }
  }
  else if (arg.is(_type))
  {
    for (std::size_t i = 0; i < arg.size(); ++i)
      simplifyHelper(new_args, arg[i]);
  }
  else
    new_args.push_back(arg);
}

Node
MultinaryOperatorData::simplify()
{
  for (auto & arg : _args)
    arg.simplify();

  std::vector<Node> new_args;

  switch (_type)
  {
    case MultinaryOperatorType::ADDITION:
      for (auto & arg : _args)
        simplifyHelper(new_args, arg);

      if (new_args.size() > 1 && new_args[0].is(0.0))
        new_args.erase(new_args.begin());

      if (new_args.size() == 1)
        return new_args[0];
      else
        return Node(MultinaryOperatorType::ADDITION, new_args);

    case MultinaryOperatorType::MULTIPLICATION:
      for (auto & arg : _args)
        simplifyHelper(new_args, arg);

      if (new_args.size() > 1 && new_args[0].is(1.0))
        new_args.erase(new_args.begin());

      if (new_args.size() == 1 || (new_args.size() > 1 && new_args[0].is(0.0)))
        return new_args[0];
      else
        return Node(MultinaryOperatorType::MULTIPLICATION, new_args);

    default:
      fatalError("Operator not implemented");
  }

  return Node();
}

Node
MultinaryOperatorData::D(const ValueProvider & vp)
{
  std::vector<Node> new_args;

  switch (_type)
  {
    case MultinaryOperatorType::ADDITION:
      for (auto & arg : _args)
        new_args.push_back(arg.D(vp));
      return Node(_type, new_args);

    case MultinaryOperatorType::MULTIPLICATION:
    {
      const auto nargs = _args.size();
      std::vector<Node> summands;
      for (std::size_t i = 0; i < nargs; ++i)
      {
        std::vector<Node> factors;
        for (std::size_t j = 0; j < nargs; ++j)
          factors.push_back(i == j ? _args[j].D(vp) : _args[j]);
        summands.push_back(Node(MultinaryOperatorType::MULTIPLICATION, factors));
      }

      return Node(MultinaryOperatorType::ADDITION, summands);
    }

    default:
      fatalError("Derivative not implemented");
  }
}

unsigned short
MultinaryOperatorData::precedence() const
{
  switch (_type)
  {
    case MultinaryOperatorType::ADDITION:
      return 6;

    case MultinaryOperatorType::MULTIPLICATION:
      return 5;

    default:
      fatalError("Unknown operator");
  }
}

/********************************************************
 * Unary Function Node
 ********************************************************/

Real
UnaryFunctionData::value()
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

std::string
UnaryFunctionData::format() const
{
  return stringify(_type) + "(" + _args[0].format() + ")";
}

std::string
UnaryFunctionData::formatTree(std::string indent) const
{
  return indent + stringify(_type) + '\n' + _args[0].formatTree(indent + "  ");
}

NodeDataPtr
UnaryFunctionData::clone()
{
  return std::make_shared<UnaryFunctionData>(_type, _args[0]);
}

Node
UnaryFunctionData::simplify()
{
  _args[0].simplify();
  if (_args[0].is(NumberType::_ANY))
    return Node(value());

  switch (_type)
  {
    default:
      return Node();
  }
}

Node
UnaryFunctionData::D(const ValueProvider & vp)
{
  auto & A = _args[0];
  auto dA = _args[0].D(vp);

  switch (_type)
  {
    case UnaryFunctionType::ABS:
      return dA * A / Node(_type, A);

    case UnaryFunctionType::ACOS:
      return -dA * Node(BinaryOperatorType::POWER,
                        Node(1.0) - Node(IntegerPowerType::_ANY, A, 2),
                        Node(-0.5));

    case UnaryFunctionType::ACOSH:
      return dA * Node(BinaryOperatorType::POWER,
                       Node(IntegerPowerType::_ANY, A, 2) - Node(1.0),
                       Node(-0.5));

    case UnaryFunctionType::ARG:
      fatalError("Derivative not implemented");

    case UnaryFunctionType::ASIN:
      return dA * Node(BinaryOperatorType::POWER,
                       Node(1.0) - Node(IntegerPowerType::_ANY, A, 2),
                       Node(-0.5));

    case UnaryFunctionType::ASINH:
      return dA * Node(BinaryOperatorType::POWER,
                       Node(1.0) + Node(IntegerPowerType::_ANY, A, 2),
                       Node(-0.5));

    case UnaryFunctionType::ATAN:
      return dA / (Node(IntegerPowerType::_ANY, A, 2) + Node(1.0));

    case UnaryFunctionType::ATANH:
      return dA / (Node(1.0) - Node(IntegerPowerType::_ANY, A, 2));

    case UnaryFunctionType::CBRT:
      return Node(1.0 / 3.0) * Node(IntegerPowerType::_ANY, Node(_type, A), -2);

    case UnaryFunctionType::CEIL:
      fatalError("Derivative not implemented");

    case UnaryFunctionType::CONJ:
      fatalError("Derivative not implemented");

    case UnaryFunctionType::COS:
      return -dA * Node(UnaryFunctionType::SIN, A);

    case UnaryFunctionType::COSH:
      return dA * Node(UnaryFunctionType::SINH, A);

    case UnaryFunctionType::COT:
      // 1 / tan
      return -dA * Node(IntegerPowerType::_ANY, Node(UnaryFunctionType::CSC, A), 2);

    case UnaryFunctionType::CSC:
      // 1 / sin
      return -dA * Node(UnaryFunctionType::COT, A) / Node(UnaryFunctionType::SIN, A);

    case UnaryFunctionType::ERF: // d exp(A) = dA*exp(A)
      return dA * Node(2.0 / std::sqrt(Constant::pi)) *
             Node(UnaryFunctionType::EXP, -Node(IntegerPowerType::_ANY, A, 2));

    case UnaryFunctionType::EXP: // d exp(A) = dA*exp(A)
      return dA * Node(_type, A);

    case UnaryFunctionType::EXP2:
      return dA * Node(Constant::ln2) * Node(_type, A);

    case UnaryFunctionType::FLOOR:
      fatalError("Derivative not implemented");

    case UnaryFunctionType::IMAG:
      fatalError("Derivative not implemented");

    case UnaryFunctionType::INT:
      fatalError("Derivative not implemented");

    case UnaryFunctionType::LOG: // d log(A) = dA/A
      return dA / A;

    case UnaryFunctionType::LOG2:
      return dA / (A * Node(Constant::ln2));

    case UnaryFunctionType::LOG10:
      return dA / (A * Node(Constant::ln10));

    case UnaryFunctionType::SEC:
      // 1 / cos
      return dA * Node(UnaryFunctionType::TAN, A) / Node(UnaryFunctionType::COS, A);

    case UnaryFunctionType::SIN: // d sin(A) = dA*cos(A)
      return dA * Node(UnaryFunctionType::COS, A);

    case UnaryFunctionType::SINH:
      return dA * Node(UnaryFunctionType::COSH, A);

    case UnaryFunctionType::SQRT:
      return Node(0.5) / Node(_type, A);

    case UnaryFunctionType::TAN:
      return dA * Node(IntegerPowerType::_ANY, Node(UnaryFunctionType::SEC, A), 2);

    case UnaryFunctionType::TANH:
      return dA * (Node(1.0) - Node(IntegerPowerType::_ANY, Node(UnaryFunctionType::TANH, A), 2));

    default:
      fatalError("Derivative not implemented");
  }
}

/********************************************************
 * Binary Function Node
 ********************************************************/

Real
BinaryFunctionData::value()
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

std::string
BinaryFunctionData::format() const
{
  return stringify(_type) + "(" + _args[0].format() + ", " + _args[1].format() + ")";
}

std::string
BinaryFunctionData::formatTree(std::string indent) const
{
  return indent + stringify(_type) + '\n' + _args[0].formatTree(indent + "  ") +
         _args[1].formatTree(indent + "  ");
}

NodeDataPtr
BinaryFunctionData::clone()
{
  return std::make_shared<BinaryFunctionData>(_type, _args[0], _args[1]);
}

Node
BinaryFunctionData::simplify()
{
  _args[0].simplify();
  _args[1].simplify();
  if (_args[0].is(NumberType::_ANY) && _args[1].is(NumberType::_ANY))
    return Node(value());

  switch (_type)
  {
    case BinaryFunctionType::POW:
      //(a^b)^c = a^(b*c) (c00^c01) ^ c1 = c00 ^ (c01*c1)
      if (_args[0].is(_type))
      {
        auto p = Node(_type,
                      _args[0][0],
                      Node(MultinaryOperatorType::MULTIPLICATION, {_args[0][1], _args[1]}));
        p.simplify();
        return p;
      }

      if (_args[1].is(NumberType::_ANY))
      { // a^0 = 1
        if (_args[1].is(0.0))
          return Node(1.0);

        // a^1 = a
        else if (_args[1].is(1.0))
          return _args[0];

        // the exponent is an integer number
        else
        {
          auto e = _args[1].value();
          if (e == std::round(e))
            return Node(IntegerPowerType::_ANY, _args[0], static_cast<int>(e));
        }
      }
      return Node();

    default:
      return Node();
  }
}

Node
BinaryFunctionData::D(const ValueProvider & vp)
{
  auto & A = _args[0];
  auto & B = _args[1];
  auto dA = _args[0].D(vp);
  auto dB = _args[1].D(vp);

  switch (_type)
  {
    case BinaryFunctionType::ATAN2:
      return (B * dA - A * dB) /
             (Node(IntegerPowerType::_ANY, A, 2) + Node(IntegerPowerType::_ANY, B, 2));

    case BinaryFunctionType::MIN:
      return Node(ConditionalType::IF, A < B, dA, dB);

    case BinaryFunctionType::MAX:
      return Node(ConditionalType::IF, A < B, dB, dA);

    case BinaryFunctionType::PLOG:
      return dA *
             Node(ConditionalType::IF,
                  A < B,
                  Node(1.0) / B - (A - B) / Node(IntegerPowerType::_ANY, B, 2) +
                      Node(IntegerPowerType::_ANY, A - B, 2) / Node(IntegerPowerType::_ANY, B, 3),
                  Node(1.0) / A);

    case BinaryFunctionType::POW:
      if (B.is(NumberType::_ANY))
      {
        if (B.is(1.0))
          return dB;
        else if (B.is(0.0))
          return Node(0.0);

        return Node(_type, A, B - Node(1.0)) * B * dA;
      }

      return Node(_type, A, B) * (dB * Node(UnaryFunctionType::LOG, A) + B * dA / A);

    default:
      fatalError("Derivative not implemented");
  }
}

/********************************************************
 * Conditional Node
 ********************************************************/

Real
ConditionalData::value()
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

std::string
ConditionalData::format() const
{
  return stringify(_type) + "(" + _args[0].format() + ", " + _args[1].format() + ", " +
         _args[2].format() + ")";
}

std::string
ConditionalData::formatTree(std::string indent) const
{
  return indent + stringify(_type) + '\n' + _args[0].formatTree(indent + "  ") + indent + "do\n" +
         _args[1].formatTree(indent + "  ") + indent + "otherwise\n" +
         _args[2].formatTree(indent + "  ");
}

NodeDataPtr
ConditionalData::clone()
{
  return std::make_shared<ConditionalData>(_type, _args[0], _args[1], _args[2]);
}

Node
ConditionalData::simplify()
{
  if (_type != ConditionalType::IF)
    fatalError("Conditional not implemented");

  _args[0].simplify();
  _args[1].simplify();
  _args[2].simplify();

  // if the conditional is constant we can drop a branch
  if (_args[0].is(NumberType::_ANY))
  {
    if (_args[0].value() != 0.0)
      return _args[1];
    else
      return _args[2];
  }

  return Node();
}

Node
ConditionalData::D(const ValueProvider & vp)
{
  if (_type != ConditionalType::IF)
    fatalError("Conditional not implemented");

  return Node(ConditionalType::IF, _args[0], _args[1].D(vp), _args[2].D(vp));
}

void
ConditionalData::stackDepth(std::pair<int, int> & current_max)
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

/********************************************************
 * Integer power Node
 ********************************************************/

std::string
IntegerPowerData::format() const
{
  return "ipow(" + _arg.format() + ", " + std::to_string(_exponent) + ")";
}

std::string
IntegerPowerData::formatTree(std::string indent) const
{
  return indent + "ipow" + _arg.formatTree(indent + "  ") + indent + std::to_string(_exponent) +
         '\n';
}

Node
IntegerPowerData::getArg(unsigned int i)
{
  if (i == 0)
    return _arg;
  if (i == 1)
    return Node(_exponent);
  fatalError("Requesting invalid argument");
}

Node
IntegerPowerData::simplify()
{
  //(a^b)^c = a^(b*c) (c00^c01) ^ c1 = c00 ^ (c01*c1)
  if (_arg.is(IntegerPowerType::_ANY))
  {
    auto p = Node(IntegerPowerType::_ANY, _arg[0], _arg[1].value() * _exponent);
    p.simplify();
    return p;
  }

  if (_arg.is(NumberType::_ANY))
    return Node(value());

  if (_exponent == 1)
    return _arg;

  if (_exponent == 0)
    return Node(1.0);

  return Node();
}

Node
IntegerPowerData::D(const ValueProvider & vp)
{
  auto & A = _arg;
  auto dA = _arg.D(vp);
  return dA * Node(_exponent) * Node(IntegerPowerType::_ANY, A, _exponent - 1);
}

} // namespace SymbolicMath
