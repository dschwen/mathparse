#include "SymbolicMathTree.h"
#include "SymbolicMathUtils.h"

#include <cmath>
#include <iostream> // debug

namespace SymbolicMath
{

void
Node::checkIndex(const std::vector<unsigned int> & index)
{
  auto s = shape();

  if (index.size() > s.size())
    fatalError("Index exceeds object dimensions");

  for (std::size_t i = 0; i < index.size(); ++i)
    if (index[i] >= s[i])
      fatalError("Index out of range");
}

Node *
NumberNode::D(unsigned int /*_id*/)
{
  return new RealNumberNode(0.0);
}

/********************************************************
 * Real Number
 ********************************************************/

std::string
RealNumberNode::formatTree(std::string indent)
{
  return indent + std::to_string(_value) + '\n';
}

bool
RealNumberNode::is(NumberNodeType type)
{
  return _type == NumberNodeType::REAL || _type == NumberNodeType::_ANY;
}

/********************************************************
 * Unary Operator
 ********************************************************/

Real
UnaryOperatorNode::value()
{
  switch (_type)
  {
    case UnaryOperatorNodeType::PLUS:
      return _args[0]->value();

    case UnaryOperatorNodeType::MINUS:
      return -_args[0]->value();

    default:
      fatalError("Unknown operator");
  }
}

Node *
UnaryOperatorNode::simplify()
{
  switch (_type)
  {
    case UnaryOperatorNodeType::PLUS:
      return _args[0]->simplify();

    default:
      return this;
  }
}

Node *
UnaryOperatorNode::D(unsigned int id)
{
  switch (_type)
  {
    case UnaryOperatorNodeType::PLUS:
      return _args[0]->D(id);

    case UnaryOperatorNodeType::MINUS:
      return new UnaryOperatorNode(UnaryOperatorNodeType::MINUS, _args[0]->D(id));

    default:
      fatalError("Unknown operator");
  }
}

std::string
UnaryOperatorNode::format()
{
  std::string form = stringify(_type);

  if (_args[0]->precedence() > precedence())
    return ' ' + form + '(' + _args[0]->format() + ')';
  else
    return ' ' + form + _args[0]->format();
}

std::string
UnaryOperatorNode::formatTree(std::string indent)
{
  return indent + '{' + stringify(_type) + "}\n" + _args[0]->formatTree(indent + "  ");
}

/********************************************************
 * Binary Operator Node
 ********************************************************/

Real
BinaryOperatorNode::value()
{
  const auto A = _args[0]->value();
  const auto B = _args[1]->value();

  switch (_type)
  {
    case BinaryOperatorNodeType::SUBTRACTION:
      return A - B;

    case BinaryOperatorNodeType::DIVISION:
      return A / B;

    case BinaryOperatorNodeType::LOGICAL_OR:
      return (A != 0.0 || B != 0.0) ? 1.0 : 0.0;

    case BinaryOperatorNodeType::LOGICAL_AND:
      return (A != 0.0 && B != 0.0) ? 1.0 : 0.0;

    default:
      fatalError("Unknown operator");
  }
}

std::string
BinaryOperatorNode::format()
{
  std::string out;

  if (_args[0]->precedence() > precedence())
    out = '(' + _args[0]->format() + ')';
  else
    out = _args[0]->format();

  out += ' ' + stringify(_type) + ' ';

  if (_args[1]->precedence() > precedence() ||
      (_args[1]->precedence() == precedence() &&
       (is(BinaryOperatorNodeType::SUBTRACTION) || is(BinaryOperatorNodeType::DIVISION) ||
        is(BinaryOperatorNodeType::MODULO) || is(BinaryOperatorNodeType::POWER))))
    out += '(' + _args[1]->format() + ')';
  else
    out += _args[1]->format();

  return out;
}

std::string
BinaryOperatorNode::formatTree(std::string indent)
{
  return indent + '{' + stringify(_type) + "}\n" + _args[0]->formatTree(indent + "  ") +
         _args[1]->formatTree(indent + "  ");
}

Node *
BinaryOperatorNode::simplify()
{
  // constant folding
  _args[0].reset(_args[0]->simplify());
  _args[1].reset(_args[1]->simplify());
  if (_args[0]->is(NumberNodeType::_ANY) && _args[1]->is(NumberNodeType::_ANY))
    return new RealNumberNode(value());

  switch (_type)
  {
    case BinaryOperatorNodeType::SUBTRACTION:
      // 0 - b = -b
      if (_args[0]->is(0.0))
        return new UnaryOperatorNode(UnaryOperatorNodeType::MINUS, _args[1].release());

      // a - 0 = a
      else if (_args[1]->is(0.0))
        return _args[0].release();

      return this;

    case BinaryOperatorNodeType::DIVISION:
      // a/1 = a
      if (_args[1]->is(1.0))
        return _args[0].release();

      // 0/b = 0
      if (_args[0]->is(0.0))
        return new RealNumberNode(0.0);

      return this;

    case BinaryOperatorNodeType::POWER:
      //(a^b)^c = a^(b*c) (c00^c01) ^ c1 = c00 ^ (c01*c1)
      if (_args[0]->is(BinaryOperatorNodeType::POWER))
      {
        auto arg0 = static_cast<BinaryOperatorNode *>(_args[0].get());
        return (new BinaryOperatorNode(
                    BinaryOperatorNodeType::POWER,
                    arg0->_args[0].release(),
                    new MultinaryOperatorNode(MultinaryOperatorNodeType::MULTIPLICATION,
                                              {arg0->_args[1].release(), _args[1].release()})))
            ->simplify();
      }

      // a^0 = 1
      if (_args[1]->is(0.0))
        return new RealNumberNode(1.0);

      // a^1 = a
      else if (_args[1]->is(1.0))
        return _args[0].release();

      return this;

    default:
      return this;
  }
}

unsigned short
BinaryOperatorNode::precedence()
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
MultinaryOperatorNode::value()
{
  switch (_type)
  {
    case MultinaryOperatorNodeType::ADDITION:
    {
      Real sum = 0.0;
      for (auto & arg : _args)
        sum += arg->value();
      return sum;
    }

    case MultinaryOperatorNodeType::MULTIPLICATION:
    {
      Real product = 1.0;
      for (auto & arg : _args)
        product *= arg->value();
      return product;
    }

    default:
      fatalError("Unknown operator");
  }
}

std::string
MultinaryOperatorNode::format()
{
  const auto form = stringify(_type);
  std::string out;

  for (auto & arg : _args)
  {
    if (!out.empty())
      out += ' ' + form + ' ';

    if (arg->precedence() > precedence())
      out += '(' + arg->format() + ')';
    else
      out += arg->format();
  }

  return out;
}

std::string
MultinaryOperatorNode::formatTree(std::string indent)
{
  std::string out = indent + '{' + stringify(_type) + "}\n";
  for (auto & arg : _args)
    out += arg->formatTree(indent + "  ");
  return out;
}

Node *
MultinaryOperatorNode::simplify()
{
  for (auto & arg : _args)
    arg.reset(arg->simplify());

  std::vector<Node *> new_args;
  RealNumberNode * constant = nullptr;

  switch (_type)
  {
    case MultinaryOperatorNodeType::ADDITION:
      for (auto & arg : _args)
      {
        if (arg->is(NumberNodeType::_ANY))
        {
          if (!constant)
            constant = new RealNumberNode(arg->value());
          else
            constant->setValue(constant->value() + arg->value());
        }
        else if (arg->is(MultinaryOperatorNodeType::ADDITION))
        {
          auto multi_arg = static_cast<MultinaryOperatorNode *>(arg.get());
          for (auto & child_arg : multi_arg->_args)
            new_args.push_back(child_arg.release());
        }
        else
          new_args.push_back(arg.release());
      }

      if (constant && constant->value() != 0.0)
        new_args.push_back(constant);

      return new MultinaryOperatorNode(MultinaryOperatorNodeType::ADDITION, new_args);

    case MultinaryOperatorNodeType::MULTIPLICATION:
      for (auto & arg : _args)
      {
        if (arg->is(NumberNodeType::_ANY))
        {
          if (!constant)
            constant = new RealNumberNode(arg->value());
          else
            constant->setValue(constant->value() * arg->value());
        }
        else if (arg->is(MultinaryOperatorNodeType::MULTIPLICATION))
        {
          auto multi_arg = static_cast<MultinaryOperatorNode *>(arg.get());
          for (auto & child_arg : multi_arg->_args)
            new_args.push_back(child_arg.release());
        }
        else
          new_args.push_back(arg.release());
      }

      if (constant && constant->value() != 1.0)
        new_args.push_back(constant);

      return new MultinaryOperatorNode(MultinaryOperatorNodeType::MULTIPLICATION, new_args);

    default:
      fatalError("Operator not implemented");
  }

  return this;
}

Node *
MultinaryOperatorNode::D(unsigned int id)
{
  fatalError("Derivative not implemented");
}

unsigned short
MultinaryOperatorNode::precedence()
{
  switch (_type)
  {
    case MultinaryOperatorNodeType::ADDITION:
      return 6;

    case MultinaryOperatorNodeType::MULTIPLICATION:
      return 5;

    default:
      fatalError("Unknown operator");
  }
}

/********************************************************
 * Unary Function Node
 ********************************************************/

Real
UnaryFunctionNode::value()
{
  const auto A = _args[0]->value();

  switch (_type)
  {
    case UnaryFunctionNodeType::ABS:
      return std::abs(A);

    case UnaryFunctionNodeType::ACOS:
      return std::acos(A);

    case UnaryFunctionNodeType::ACOSH:
      return std::acosh(A);

    case UnaryFunctionNodeType::ASIN:
      return std::asin(A);

    case UnaryFunctionNodeType::ASINH:
      return std::asinh(A);

    case UnaryFunctionNodeType::ATAN:
      return std::atan(A);

    case UnaryFunctionNodeType::ATANH:
      return std::atanh(A);

    case UnaryFunctionNodeType::CBRT:
      return std::cbrt(A);

    case UnaryFunctionNodeType::CEIL:
      return std::ceil(A);

    case UnaryFunctionNodeType::COS:
      return std::cos(A);

    case UnaryFunctionNodeType::COSH:
      return std::cosh(A);

    case UnaryFunctionNodeType::COT:
      return 1.0 / std::tan(A);

    case UnaryFunctionNodeType::CSC:
      return 1.0 / std::sin(A);

    case UnaryFunctionNodeType::EXP:
      return std::exp(A);

    case UnaryFunctionNodeType::EXP2:
      return std::exp2(A);

    case UnaryFunctionNodeType::FLOOR:
      return std::floor(A);

    case UnaryFunctionNodeType::INT:
      return A < 0 ? std::ceil(A - 0.5) : std::floor(A + 0.5);

    case UnaryFunctionNodeType::LOG:
      return std::log(A);

    case UnaryFunctionNodeType::LOG10:
      return std::log10(A);

    case UnaryFunctionNodeType::LOG2:
      return std::log2(A);

    case UnaryFunctionNodeType::SEC:
      return 1.0 / std::cos(A);

    case UnaryFunctionNodeType::SIN:
      return std::sin(A);

    case UnaryFunctionNodeType::SINH:
      return std::sinh(A);

    case UnaryFunctionNodeType::TAN:
      return std::tan(A);

    case UnaryFunctionNodeType::TANH:
      return std::tanh(A);

    case UnaryFunctionNodeType::TRUNC:
      return static_cast<int>(A);

    default:
      fatalError("Function not implemented");
  }
}

std::string
UnaryFunctionNode::format()
{
  return stringify(_type) + "(" + _args[0]->format() + ")";
}

std::string
UnaryFunctionNode::formatTree(std::string indent)
{
  return indent + stringify(_type) + '\n' + _args[0]->formatTree(indent + "  ");
}

Node *
UnaryFunctionNode::simplify()
{
  _args[0].reset(_args[0]->simplify());
  return this;
}

Node *
UnaryFunctionNode::D(unsigned int id)
{
  switch (_type)
  {
    case UnaryFunctionNodeType::COS:
      return new UnaryOperatorNode(
          UnaryOperatorNodeType::MINUS,
          new UnaryFunctionNode(UnaryFunctionNodeType::SIN, _args[0].release()));

    case UnaryFunctionNodeType::EXP:
      return new BinaryOperatorNode(
          BinaryOperatorNodeType::MULTIPLICATION, _args[0]->D(id), this->clone());

    case UnaryFunctionNodeType::SIN:
      return new UnaryFunctionNode(UnaryFunctionNodeType::COS, _args[0].release());

    default:
      fatalError("Derivative not implemented");
  }
}

/********************************************************
 * Binary Function Node
 ********************************************************/

Real
BinaryFunctionNode::value()
{
  const auto A = _args[0]->value();
  const auto B = _args[1]->value();

  switch (_type)
  {
    case BinaryFunctionNodeType::ATAN2:
      return std::atan2(A, B);

    case BinaryFunctionNodeType::HYPOT:
      return std::sqrt(A * A + B * B);

    case BinaryFunctionNodeType::MIN:
      return std::min(A, B);

    case BinaryFunctionNodeType::MAX:
      return std::max(A, B);

    case BinaryFunctionNodeType::PLOG:
      return A < B
                 ? std::log(B) + (A - B) / B - (A - B) * (A - B) / (2.0 * B * B) +
                       (A - B) * (A - B) * (A - B) / (3.0 * B * B * B)
                 : std::log(A);

    case BinaryFunctionNodeType::POW:
      return std::pow(A, B);

    case BinaryFunctionNodeType::POLAR:
    default:
      fatalError("Function not implemented");
  }
}

std::string
BinaryFunctionNode::format()
{
  return stringify(_type) + "(" + _args[0]->format() + ", " + _args[1]->format() + ")";
}

std::string
BinaryFunctionNode::formatTree(std::string indent)
{
  return indent + stringify(_type) + '\n' + _args[0]->formatTree(indent + "  ") +
         _args[1]->formatTree(indent + "  ");
}

Node *
BinaryFunctionNode::clone()
{
  return new BinaryFunctionNode(_type, _args[0]->clone(), _args[1]->clone());
}

Node *
BinaryFunctionNode::simplify()
{
  _args[0].reset(_args[0]->simplify());
  _args[1].reset(_args[1]->simplify());

  switch (_type)
  {
    default:
      return this;
  }
}

/********************************************************
 * Conditional Node
 ********************************************************/

Real
ConditionalNode::value()
{
  if (_type != ConditionalNodeType::IF)
    fatalError("Conditional not implemented");

  if (_args[0]->value() != 0.0)
    // true expression
    return _args[1]->value();
  else
    // false expression
    return _args[2]->value();
}

std::string
ConditionalNode::format()
{
  return stringify(_type) + "(" + _args[0]->format() + ", " + _args[1]->format() + ", " +
         _args[2]->format() + ")";
}

std::string
ConditionalNode::formatTree(std::string indent)
{
  return indent + stringify(_type) + '\n' + _args[0]->formatTree(indent + "  ") + indent + "do\n" +
         _args[1]->formatTree(indent + "  ") + indent + "otherwise\n" +
         _args[2]->formatTree(indent + "  ");
}

Node *
ConditionalNode::clone()
{
  return new ConditionalNode(_type, _args[0]->clone(), _args[1]->clone(), _args[2]->clone());
}

Node *
ConditionalNode::simplify()
{
  if (_type != ConditionalNodeType::IF)
    fatalError("Conditional not implemented");

  _args[0].reset(_args[0]->simplify());
  _args[1].reset(_args[1]->simplify());
  _args[2].reset(_args[2]->simplify());

  // if the conditional is constant we can drop a branch
  if (_args[0]->is(NumberNodeType::_ANY))
  {
    if (_args[0]->value() != 0.0)
      return _args[1].release();
    else
      return _args[2].release();
  }

  return this;
}

Node *
ConditionalNode::D(unsigned int id)
{
  if (_type != ConditionalNodeType::IF)
    fatalError("Conditional not implemented");

  return new ConditionalNode(
      ConditionalNodeType::IF, _args[0].release(), _args[1]->D(id), _args[2]->D(id));
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
