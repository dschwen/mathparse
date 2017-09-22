#include "SymbolicMathTree.h"

#include <cmath>

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
        default:
          return NAN;
      }

    case TokenType::FUNCTION:
      switch (_function_type)
      {
        case FunctionType::ABS:
          return std::abs(_children[0]->value());
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
        // binary operators
        std::string out;
        if (_children[0]->precedence() > precedence())
          out = '(' + _children[0]->format() + ')';
        else
          out = _children[0]->format();

        out += ' ' + operatorProperty(_operator_type)._form + ' ';

        if (_children[1]->precedence() > precedence() ||
            (_children[1]->precedence() == precedence() &&
             (_operator_type == OperatorType::SUBTRACTION ||
              _operator_type == OperatorType::DIVISION || _operator_type == OperatorType::MODULO ||
              _operator_type == OperatorType::POWER)))
          out += '(' + _children[1]->format() + ')';
        else
          out += _children[1]->format();

        return out;
      }

    case TokenType::FUNCTION:
    {
      std::string out = functionProperty(_function_type)._form + '(';
      unsigned int arguments = functionProperty(_function_type)._arguments;
      for (unsigned i = 0; i < arguments; ++i)
        out += (i ? ", " : "");
      return out + ')';
    }

    default:
      return "[???]";
  }
}

// end namespace SymbolicMath
}
