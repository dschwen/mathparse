#include "SymbolicMathTree.h"

#include <cmath>
#include <iostream> // debug

namespace SymbolicMath
{

Tree::Tree(OperatorType operator_type, std::vector<Tree *> children)
  : _type(TokenType::OPERATOR), _operator_type(operator_type)
{
  for (auto child : children)
    _children.emplace_back(std::unique_ptr<Tree>(child));
}

Tree::Tree(FunctionType function_type, std::vector<Tree *> children)
  : _type(TokenType::FUNCTION), _function_type(function_type)
{
  for (auto child : children)
    _children.emplace_back(std::unique_ptr<Tree>(child));
}

Tree::Tree(Real real) : _type(TokenType::NUMBER), _real(real), _children() {}

Real
Tree::value()
{
  switch (_type)
  {
    case TokenType::NUMBER:
      return _real;

    case TokenType::OPERATOR:
      switch (_operator_type)
      {
        case OperatorType::UNARY_PLUS:
          return _children[0]->value();
        case OperatorType::UNARY_MINUS:
          return -_children[0]->value();
        case OperatorType::ADDITION:
        {
          Real sum = 0.0;
          for (auto & child : _children)
            sum += child->value();
          return sum;
        }
        case OperatorType::SUBTRACTION:
          return _children[0]->value() - _children[1]->value();
        case OperatorType::MULTIPLICATION:
        {
          Real product = 1.0;
          for (auto & child : _children)
            product *= child->value();
          return product;
        }
        case OperatorType::DIVISION:
          return _children[0]->value() / _children[1]->value();
        default:
          return NAN;
      }

    //    {1, "acosh"}, {1, "arg"},   {1, "asinh"},
    // {1, "atanh"}, {1, "cbrt"},  {1, "ceil"},  {1, "conj"},    {1, "cosh"},
    // {1, "cot"},   {1, "csc"},   {1, "exp"},   {1, "exp2"},  {1, "floor"}, {2, "hypot"}, {3,
    // "if"},
    // {1, "imag"},  {1, "int"},   {1, "log"},   {1, "log10"}, {1, "log2"},
    // {2, "plog"},  {2, "polar"}, {2, "pow"},   {1, "real"},  {1, "sec"},   {1, "sinh"},
    // {1, "sqrt"},  {1, "tan"},   {1, "tanh"},  {1, "trunc"}};

    case TokenType::FUNCTION:
      switch (_function_type)
      {
        case FunctionType::ABS:
          return std::abs(_children[0]->value());
        case FunctionType::ACOS:
          return std::acos(_children[0]->value());
        case FunctionType::ASIN:
          return std::asin(_children[0]->value());
        case FunctionType::ATAN:
          return std::atan(_children[0]->value());
        case FunctionType::ATAN2:
          return std::atan2(_children[1]->value(), _children[0]->value());
        case FunctionType::COS:
          return std::cos(_children[0]->value());
        case FunctionType::MIN:
          return std::min(_children[1]->value(), _children[0]->value());
        case FunctionType::MAX:
          return std::max(_children[1]->value(), _children[0]->value());
        case FunctionType::SIN:
          return std::sin(_children[0]->value());
        default:
          return NAN;
      }

    default:
      return NAN;
  }
}

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
    case TokenType::NUMBER:
      return std::to_string(_real);

    case TokenType::OPERATOR:
      if (operatorProperty(_operator_type)._unary)
      {
        // unary operators
        const auto & form = operatorProperty(_operator_type)._form;
        if (_children[0]->precedence() > precedence())
          return ' ' + form + '(' + _children[0]->format() + ')';
        else
          return ' ' + form + _children[0]->format();
      }
      else
      {
        std::string out;
        // multinary operators
        if (_operator_type == OperatorType::ADDITION ||
            _operator_type == OperatorType::MULTIPLICATION)
        {
          for (auto & child : _children)
          {
            if (!out.empty())
              out += ' ' + operatorProperty(_operator_type)._form + ' ';

            if (child->precedence() > precedence())
              out += '(' + child->format() + ')';
            else
              out += child->format();
          }
        }
        else
        {
          // binary operators
          if (_children[0]->precedence() > precedence())
            out = '(' + _children[0]->format() + ')';
          else
            out = _children[0]->format();

          out += ' ' + operatorProperty(_operator_type)._form + ' ';

          if (_children[1]->precedence() > precedence() ||
              (_children[1]->precedence() == precedence() &&
               (_operator_type == OperatorType::SUBTRACTION ||
                _operator_type == OperatorType::DIVISION ||
                _operator_type == OperatorType::MODULO || _operator_type == OperatorType::POWER)))
            out += '(' + _children[1]->format() + ')';
          else
            out += _children[1]->format();
        }
        return out;
      }

    case TokenType::FUNCTION:
    {
      std::string out = functionProperty(_function_type)._form + '(';
      const unsigned int arguments = functionProperty(_function_type)._arguments;
      for (unsigned i = 0; i < arguments; ++i)
        out += (i ? ", " : "") + _children[arguments - 1 - i]->format();
      return out + ')';
    }

    default:
      return "[???]";
  }
}

std::string
Tree::formatTree(std::string indent)
{
  std::string out;
  switch (_type)
  {
    case TokenType::NUMBER:
      out = indent + std::to_string(_real) + '\n';
      break;

    case TokenType::OPERATOR:
      out = indent + '[' + operatorProperty(_operator_type)._form + "]\n";
      for (auto & child : _children)
        out += child->formatTree(indent + "  ");
      break;

    case TokenType::FUNCTION:
      out = indent + functionProperty(_function_type)._form + '\n';
      for (auto & child : _children)
        out += child->formatTree(indent + "  ");
      break;

    default:
      out = indent + "[???]\n";
  }

  return out;
}

void
Tree::become(std::unique_ptr<Tree> tree)
{
  _type = tree->_type;
  switch (_type)
  {
    case TokenType::NUMBER:
      _real = tree->_real;
      break;

    case TokenType::OPERATOR:
      _operator_type = tree->_operator_type;
      break;

    case TokenType::FUNCTION:
      _function_type = tree->_function_type;
      break;

    default:
      _value_provider_id = tree->_value_provider_id;
  }

  _children = std::move(tree->_children);
}

bool
Tree::isNumber(Real number)
{
  return _type == TokenType::NUMBER && _real == number;
}

bool
Tree::simplify()
{
  if (_type == TokenType::NUMBER)
    return true;

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

      case OperatorType::SUBTRACTION:
        // 0 - b = -b
        if (_children[0]->isNumber(0.0))
        {
          _operator_type = OperatorType::UNARY_MINUS;
          _children[0] = std::move(_children[1]);
          _children.resize(1);
        }
        // a - 0 = a
        else if (_children[1]->isNumber(0.0))
          become(std::move(_children[0]));
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

      case OperatorType::DIVISION:
        // a/1 = a
        if (_children[1]->isNumber(1.0))
          become(std::move(_children[0]));
        // 0/b = 0
        if (_children[0]->isNumber(0.0))
        {
          _real = 0.0;
          _type = TokenType::NUMBER;
          _children.clear();
          return true;
        }
        break;

      case OperatorType::POWER:
        // a^0 = 1
        if (_children[1]->isNumber(0.0))
        {
          _real = 1.0;
          _type = TokenType::NUMBER;
          _children.clear();
          return true;
        }
        // a^1 = a
        else if (_children[1]->isNumber(1.0))
          become(std::move(_children[0]));
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
