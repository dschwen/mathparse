#include "SymbolicMathNodeData.h"
#include "SymbolicMathUtils.h"

#include <cmath>
#include <iostream> // debug

namespace SymbolicMath
{

std::string
ValueProviderData::formatTree(std::string indent)
{
  return indent + "_val" + std::to_string(_id) + '\n';
}

Node
ValueProviderData::D(unsigned int id)
{
  if (id == _id)
    return new RealNumberData(1.0);
  else
    return new RealNumberData(0.0);
}

Node
NumberData::D(unsigned int /*id*/)
{
  return new RealNumberData(0.0);
}

/********************************************************
 * Real Number
 ********************************************************/

std::string
RealNumberData::formatTree(std::string indent)
{
  return indent + stringify(_value) + '\n';
}

bool
RealNumberData::is(NumberType type)
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

Node
UnaryOperatorData::simplify()
{
  _args[0].simplify();
  if (_args[0].is(NumberType::_ANY))
    return new RealNumberData(value());

  return this;
}

Node
UnaryOperatorData::D(unsigned int id)
{
  switch (_type)
  {
    case UnaryOperatorType::PLUS:
      return _args[0].D(id);

    case UnaryOperatorType::MINUS:
      return new UnaryOperatorData(_type, _args[0].D(id));

    default:
      fatalError("Unknown operator");
  }
}

std::string
UnaryOperatorData::format()
{
  std::string form = stringify(_type);

  if (_args[0].precedence() > precedence())
    return ' ' + form + '(' + _args[0].format() + ')';
  else
    return ' ' + form + _args[0].format();
}

std::string
UnaryOperatorData::formatTree(std::string indent)
{
  return indent + '{' + stringify(_type) + "}\n" + _args[0].data()->formatTree(indent + "  ");
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

    case BinaryOperatorType::LOGICAL_OR:
      return (A != 0.0 || B != 0.0) ? 1.0 : 0.0;

    case BinaryOperatorType::LOGICAL_AND:
      return (A != 0.0 && B != 0.0) ? 1.0 : 0.0;

    default:
      fatalError("Unknown operator");
  }
}

std::string
BinaryOperatorData::format()
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
BinaryOperatorData::formatTree(std::string indent)
{
  return indent + '{' + stringify(_type) + "}\n" + _args[0].data()->formatTree(indent + "  ") +
         _args[1].data()->formatTree(indent + "  ");
}

Node
BinaryOperatorData::simplify()
{
  // constant folding
  _args[0].simplify();
  _args[1].simplify();
  if (_args[0].is(NumberType::_ANY) && _args[1].is(NumberType::_ANY))
    return new RealNumberData(value());

  switch (_type)
  {
    case BinaryOperatorType::SUBTRACTION:
      // 0 - b = -b
      if (_args[0].is(0.0))
        return Node(new UnaryOperatorData(UnaryOperatorType::MINUS, _args[1]));

      // a - 0 = a
      else if (_args[1].is(0.0))
        return _args[0];

      return this;

    case BinaryOperatorType::DIVISION:
      // a/1 = a
      if (_args[1].is(1.0))
        return _args[0];

      // 0/b = 0
      if (_args[0].is(0.0))
        return Node(0.0);

      return this;

    case BinaryOperatorType::POWER:
      //(a^b)^c = a^(b*c) (c00^c01) ^ c1 = c00 ^ (c01*c1)
      if (_args[0].is(_type))
      {
        auto arg0 = std::static_pointer_cast<BinaryOperatorData>(_args[0].data());
        return (new BinaryOperatorData(
                    _type,
                    arg0->_args[0],
                    new MultinaryOperatorData(MultinaryOperatorType::MULTIPLICATION,
                                              {arg0->_args[1], _args[1]})))
            ->simplify();
      }

      // a^0 = 1
      if (_args[1].is(0.0))
        return Node(1.0);

      // a^1 = a
      else if (_args[1].is(1.0))
        return _args[0];

      return this;

    default:
      return this;
  }
}

Node
BinaryOperatorData::D(unsigned int id)
{
  switch (_type)
  {
    case BinaryOperatorType::SUBTRACTION: // d (A - B) = dA - dB
      return new BinaryOperatorData(_type, _args[0].D(id), _args[1].D(id));

    case BinaryOperatorType::DIVISION: // d (A / B) = dA/B - dB/(B*B)
      return new BinaryOperatorData(
          BinaryOperatorType::SUBTRACTION,
          new BinaryOperatorData(BinaryOperatorType::DIVISION, _args[0].D(id), _args[1]),
          new BinaryOperatorData(BinaryOperatorType::DIVISION,
                                 _args[1].D(id),
                                 new MultinaryOperatorData(MultinaryOperatorType::MULTIPLICATION,
                                                           {_args[1], _args[1]})));

    case BinaryOperatorType::POWER:
      if (_args[1].is(1.0))
        return _args[1].D(id);
      else if (_args[1].is(0.0))
        return Node(0.0);

      return new MultinaryOperatorData(
          MultinaryOperatorType::MULTIPLICATION,
          {new BinaryOperatorData(
               _type,
               _args[0],
               new BinaryOperatorData(BinaryOperatorType::SUBTRACTION, _args[1], Node(1.0))),
           _args[1],
           _args[0].D(id)});

    default:
      fatalError("Derivative not implemnted");
  }
}

unsigned short
BinaryOperatorData::precedence()
{
  const auto index = static_cast<int>(_type);
  //                                        -  /  %  ^  |   &
  const std::vector<unsigned short> list = {6, 5, 5, 4, 14, 13};

  if (index >= list.size())
    fatalError("Unknown operator");

  return list[index];
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
MultinaryOperatorData::format()
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
MultinaryOperatorData::formatTree(std::string indent)
{
  std::string out = indent + '{' + stringify(_type) + "}\n";
  for (auto & arg : _args)
    out += arg.data()->formatTree(indent + "  ");
  return out;
}

Node
MultinaryOperatorData::clone()
{
  std::vector<Node> cloned_args;
  for (auto & arg : _args)
    cloned_args.push_back(arg);
  return new MultinaryOperatorData(_type, cloned_args);
}

void
MultinaryOperatorData::simplifyHelper(RealNumberData *& constant,
                                      std::vector<Node> & new_args,
                                      Node arg)
{
  if (arg.is(NumberType::_ANY))
  {
    if (!constant)
      constant = new RealNumberData(arg.value());
    else
    {
      auto val = constant->value();
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
      constant->setValue(val);
    }
  }
  else if (arg.is(_type))
  {
    auto multi_arg = static_cast<MultinaryOperatorData *>(arg.get());
    for (auto & child_arg : multi_arg->_args)
      simplifyHelper(constant, new_args, child_arg);
  }
  else
    new_args.push_back(arg.release());
}

Node
MultinaryOperatorData::simplify()
{
  for (auto & arg : _args)
    arg.simplify();

  std::vector<Node> new_args;
  RealNumberData * constant = nullptr;

  switch (_type)
  {
    case MultinaryOperatorType::ADDITION:
      for (auto & arg : _args)
        simplifyHelper(constant, new_args, arg);

      if (constant && constant->value() != 0.0)
        new_args.push_back(constant);

      if (new_args.size() == 1)
        return new_args[0];
      else
        return new MultinaryOperatorData(MultinaryOperatorType::ADDITION, new_args);

    case MultinaryOperatorType::MULTIPLICATION:
      for (auto & arg : _args)
        simplifyHelper(constant, new_args, arg);

      if (constant && constant->value() != 1.0)
        new_args.push_back(constant);

      if (new_args.size() == 1)
        return new_args[0];
      else
        return new MultinaryOperatorData(MultinaryOperatorType::MULTIPLICATION, new_args);

    default:
      fatalError("Operator not implemented");
  }

  return this;
}

Node
MultinaryOperatorData::D(unsigned int id)
{
  std::vector<Node> new_args;
  switch (_type)
  {
    case MultinaryOperatorType::ADDITION:
      for (auto & arg : _args)
        new_args.push_back(arg.D(id));
      return new MultinaryOperatorData(_type, new_args);

    default:
      fatalError("Derivative not implemented");
  }
}

unsigned short
MultinaryOperatorData::precedence()
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

    case UnaryFunctionType::EXP:
      return std::exp(A);

    case UnaryFunctionType::EXP2:
      return std::exp2(A);

    case UnaryFunctionType::FLOOR:
      return std::floor(A);

    case UnaryFunctionType::INT:
      return A < 0 ? std::ceil(A - 0.5) : std::floor(A + 0.5);

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
UnaryFunctionData::format()
{
  return stringify(_type) + "(" + _args[0].format() + ")";
}

std::string
UnaryFunctionData::formatTree(std::string indent)
{
  return indent + stringify(_type) + '\n' + _args[0].data()->formatTree(indent + "  ");
}

Node
UnaryFunctionData::simplify()
{
  _args[0].simplify();
  return this;
}

Node
UnaryFunctionData::D(unsigned int id)
{
  switch (_type)
  {
    case UnaryFunctionType::COS:
      return new UnaryOperatorData(
          UnaryOperatorType::MINUS,
          new UnaryFunctionNode(UnaryFunctionType::SIN, _args[0].release()));

    case UnaryFunctionType::EXP: // d exp(A) = dA*exp(A)
      return new MultinaryOperatorData(MultinaryOperatorType::MULTIPLICATION,
                                       {_args[0].D(id), this->clone()});

    case UnaryFunctionType::LOG: // d log(A) = dA/A
      return new BinaryOperatorData(BinaryOperatorType::DIVISION, _args[0].D(id), _args[0].clone());

    case UnaryFunctionType::SIN: // d sin(A) = dA*cos(A)
      return new MultinaryOperatorData(
          MultinaryOperatorType::MULTIPLICATION,
          {_args[0].D(id), new UnaryFunctionNode(UnaryFunctionType::COS, _args[0].release())});

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
      return A < B
                 ? std::log(B) + (A - B) / B - (A - B) * (A - B) / (2.0 * B * B) +
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
BinaryFunctionData::format()
{
  return stringify(_type) + "(" + _args[0].format() + ", " + _args[1].format() + ")";
}

std::string
BinaryFunctionData::formatTree(std::string indent)
{
  return indent + stringify(_type) + '\n' + _args[0].data()->formatTree(indent + "  ") +
         _args[1].data()->formatTree(indent + "  ");
}

Node
BinaryFunctionData::clone()
{
  return new BinaryFunctionNode(_type, _args[0].clone(), _args[1].clone());
}

Node
BinaryFunctionData::simplify()
{
  _args[0].simplify();
  _args[1].simplify();
  if (_args[0].is(NumberType::_ANY) && _args[1].is(NumberType::_ANY))
    return new RealNumberData(value());

  switch (_type)
  {
    default:
      return this;
  }
}

Node
BinaryFunctionData::D(unsigned int id)
{
  switch (_type)
  {
    case BinaryFunctionType::POW:
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
ConditionalData::format()
{
  return stringify(_type) + "(" + _args[0].format() + ", " + _args[1].format() + ", " +
         _args[2].format() + ")";
}

std::string
ConditionalData::formatTree(std::string indent)
{
  return indent + stringify(_type) + '\n' + _args[0].data()->formatTree(indent + "  ") + indent +
         "do\n" + _args[1].data()->formatTree(indent + "  ") + indent + "otherwise\n" +
         _args[2].data()->formatTree(indent + "  ");
}

Node
ConditionalData::clone()
{
  return new ConditionalNode(_type, _args[0].clone(), _args[1].clone(), _args[2].clone());
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
      return _args[1].release();
    else
      return _args[2].release();
  }

  return this;
}

Node
ConditionalData::D(unsigned int id)
{
  if (_type != ConditionalType::IF)
    fatalError("Conditional not implemented");

  return new ConditionalNode(
      ConditionalType::IF, _args[0].release(), _args[1].D(id), _args[2]->D(id));
}

/********************************************************
 *
 ********************************************************/

#if 0

std::string
Tree::format()
{
  switch (_type)
  {
    case TokenType::VARIABLE:
      return "val" + std::to_string(_value_provider_id);
  }
}

std::string
Tree::formatTree(std::string indent)
{
  std::string out;
  switch (_type)
  {
    case TokenType::VARIABLE:
      out = indent + "val" + std::to_string(_value_provider_id) + '\n';
      break;

    case TokenType::COMPONENT:
      out = indent + "[]\n";
      for (auto & child : _children)
        out += child->formatTree(indent + "  ");
      break;
  }

  return out;
}

#endif

// end namespace SymbolicMath
}
