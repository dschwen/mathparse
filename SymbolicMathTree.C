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

/********************************************************
 * Unary Operator
 ********************************************************/

Real
UnaryOperatorNode::value()
{
  switch (_type)
  {
    case UnaryOperatorNodeType::PLUS:
      return _argument->value();

    case UnaryOperatorNodeType::MINUS:
      return -_argument->value();

    default:
      fatalError("Unknown operator");
  }
}

virtual Node *
UnaryOperatorNode::simplify()
{
  switch (_type)
  {
    case UnaryOperatorNodeType::PLUS:
      return _argument->simplify();

    default:
      return this;
  }
}

virtual Node *
UnaryOperatorNode::D(unsigned int _id)
{
  switch (_type)
  {
    case UnaryOperatorNodeType::PLUS:
      return _argument->D(id);

    case UnaryOperatorNodeType::MINUS:
      return new UnaryOperatorNode(UnaryOperatorNodeType::MINUS, _argument->D());

    default:
      fatalError("Unknown operator");
  }
}

std::string
UnaryOperatorNode::format()
{
  std::string form = stringify(_type);

  if (_argument->precedence() > precedence())
    return ' ' + form + '(' + _argument->format() + ')';
  else
    return ' ' + form + _argument->format();
}

std::string
UnaryOperatorNode::formatTree()
{
  return indent + '{' + stringify(_type) + "}\n" + _argument->formatTree(indent + "  ");
}

/********************************************************
 * Binary Operator Node
 ********************************************************/

Real
BinaryOperatorNode::value()
{
  const auto A = _left->value();
  const auto B = _right->value();

  switch (_type)
  {
    case BinaryOperatorNodeType::ADDITION:
      return A + B;

    case BinaryOperatorNodeType::SUBTRACTION:
      return A - B;

    case BinaryOperatorNodeType::MULTIPLICATION:
      return A * B;

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

  if (_left->precedence() > precedence())
    out = '(' + _left->format() + ')';
  else
    out = _left->format();

  out += ' ' + stringify(_type) + ' ';

  if (_right->precedence() > precedence() ||
      (_right->precedence() == precedence() &&
       (_operator_type == OperatorType::SUBTRACTION || _operator_type == OperatorType::DIVISION ||
        _operator_type == OperatorType::MODULO || _operator_type == OperatorType::POWER)))
    out += '(' + _right->format() + ')';
  else
    out += _right->format();

  return out;
}

std::string
BinaryOperatorNode::formatTree(std::string indent)
{
  return indent + '{' + stringify(_type) + "}\n" + _left->formatTree(indent + "  ") +
         _right->formatTree(indent + "  ");
}

virtual Node *
BinaryOperatorNode::simplify()
{
  // constant folding
  _left = _left->simplify();
  _right = _right->simplify();
  if (_left->is(NumberNodeType::_ANY) && _right->is(NumberNodeType::_ANY))
    return new RealNumberNode(value());

  switch (_type)
  {
    case BinaryOperatorNodeType::SUBTRACTION:
      // 0 - b = -b
      if (_left->is(0.0))
        return UnaryOperatorNode(UnaryOperatorNodeType::MINUS, _right.release());

      // a - 0 = a
      else if (_right->is(0.0))
        return _left.release();

      break;

    case BinaryOperatorNodeType::DIVISION:
      // a/1 = a
      if (_right->is(1.0))
        return _left.release();

      // 0/b = 0
      if (_left->is(0.0))
        return new RealNumberNode(0.0);

      break;

    case BinaryOperatorNodeType::POWER:
      //(a^b)^c = a^(b*c) (c00^c01) ^ c1 = c00 ^ (c01*c1)
      if (_left->is(BinaryOperatorNodeType::POWER)
      {
        return new BinaryOperatorNode(BinaryOperatorNodeType::POWER,
                                      _left->_left.release(),
                                      BinaryOperatorNode(BinaryOperatorNodeType::MULTIPLICATION,
                                                         _left->_right.release(),
                                                         _right.release())
                                          ->simplify());
      }

      // a^0 = 1
      if (_right->is(0.0))
      return new RealNumberNode(1.0);

      // a^1 = a
      else if (_right->is(1.0))
        return _left.release();

      break;

    default:
      return this;
  }
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
      for (auto & argument : _arguments)
        sum += argument->value();
      return sum;
    }

    case MultinaryOperatorNodeType::MULTIPLICATION:
    {
      Real product = 1.0;
      for (auto & argument : _arguments)
        product *= argument->value();
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

  for (auto & argument : _arguments)
  {
    if (!out.empty())
      out += ' ' + form + ' ';

    if (argument->precedence() > precedence())
      out += '(' + argument->format() + ')';
    else
      out += argument->format();
  }
}

std::string
MultinaryOperatorNode::formatTree(std::string indent)
{
  std::string out = indent + '{' + stringify(_type) + "}\n";
  for (auto & child : _children)
    out += child->formatTree(indent + "  ");
  return out;
}

/********************************************************
 * Unary Function Node
 ********************************************************/

Real
UnaryFunctionNode::value()
{
  const auto A = _argument->value();

  switch (_type)
  {
    case UnaryFunctionNodeType::ABS:
      return std::abs(A);

    case UnaryFunctionNodeType::ACOS:
      return std::acos(A);

    case UnaryFunctionNodeType::ASIN:
      return std::asin(A);

    case UnaryFunctionNodeType::ATAN:
      return std::atan(A);

    case UnaryFunctionNodeType::COS:
      return std::cos(A);

    case UnaryFunctionNodeType::SIN:
      return std::sin(A);

    default:
      fatalError("Unknown function");
  }
}

std::string
UnaryFunctionNode::format()
{
  return stringify(_type) + "(" + _argument->format() + ")";
}

/********************************************************
 * Binary Function Node
 ********************************************************/

Real
BinaryFunctionNode::value()
{
  const auto A = _left->value();
  const auto A = _right->value();

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

    case BinaryFunctionNodeType::POW:
      return std::pow(A, B);

    case BinaryFunctionNodeType::PLOG:
    case BinaryFunctionNodeType::POLAR:
    default:
      fatalError("Unknown function");
  }
}

std::string
BinaryFunctionNode::format()
{
  return stringify(_type) + "(" + _left->format() + ", " + _right->format() + ")";
}

/********************************************************
 *
 ********************************************************/

unsigned short
Tree::precedence()
{
  if (_type == TokenType::OPERATOR)
    return operatorProperty(_operator_type)._precedence;

  return 0;
}

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

bool
Tree::simplify()
{
  //
  // constant folding
  //
  bool all_constant = true;
  for (auto & child : _children)
  {
    const bool child_constant = child->simplify();
    all_constant = all_constant && child_constant;
  }
  if (all_constant)
  {
    _real = value();
    _type = TokenType::NUMBER;
    _children.clear();
    return true;
  }

  //
  // operator specific quick simplifications
  //
  if (_type == TokenType::OPERATOR)
  {
    // gather child summands
    if (_operator_type == OperatorType::ADDITION)
    {
      std::vector<std::unique_ptr<Tree>> arguments;
      for (auto & child : _children)
        if (child->_type == TokenType::OPERATOR && child->_operator_type == _operator_type)
        {
          for (auto & grandchild : child->_children)
            if (!grandchild->isNumber(0.0))
              arguments.push_back(std::move(grandchild));
        }
        else if (!child->isNumber(0.0))
          arguments.push_back(std::move(child));

      _children = std::move(arguments);
    }
    else if (_operator_type == OperatorType::MULTIPLICATION)
    {
      std::vector<std::unique_ptr<Tree>> arguments;
      for (auto & child : _children)
        if (child->_type == TokenType::OPERATOR && child->_operator_type == _operator_type)
        {
          for (auto & grandchild : child->_children)
          {
            if (grandchild->isNumber(0.0))
            {
              _real = 0.0;
              _type = TokenType::NUMBER;
              _children.clear();
              return true;
            }
            if (!grandchild->isNumber(1.0))
              arguments.push_back(std::move(grandchild));
          }
        }
        else
        {
          if (child->isNumber(0.0))
          {
            _real = 0.0;
            _type = TokenType::NUMBER;
            _children.clear();
            return true;
          }
          if (!child->isNumber(1.0))
            arguments.push_back(std::move(child));
        }

      _children = std::move(arguments);
    }

    // simplify
    switch (_operator_type)
    {
      case OperatorType::ADDITION:
        if (_children.size() == 1)
          become(std::move(_children[0]));
        else if (_children.size() == 0)
        {
          _real = 0.0;
          _type = TokenType::NUMBER;
          _children.clear();
          return true;
        }
        break;

      case OperatorType::MULTIPLICATION:
        if (_children.size() == 1)
          become(std::move(_children[0]));
        else if (_children.size() == 0)
        {
          _real = 1.0;
          _type = TokenType::NUMBER;
          _children.clear();
          return true;
        }
        break;

      default:
        return false;
    }
  }

  return false;
}

std::unique_ptr<Tree>
D(unsigned int _id)
{
}

// end namespace SymbolicMath
}
