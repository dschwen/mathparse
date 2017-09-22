#include "SymbolicMathTree.h"

#include <cmath>

namespace SymbolicMath
{

Tree::Tree(OperatorType operator_type, std::vector<std::unique_ptr<Tree>> children)
  : _type(TokenType::OPERATOR), _operator_type(operator_type), _children(std::move(children))
{
}

Tree::Tree(FunctionType function_type, std::vector<std::unique_ptr<Tree>> children)
  : _type(TokenType::FUNCTION), _function_type(function_type), _children(std::move(children))
{
}

Tree::Tree(Real real, std::vector<std::unique_ptr<Tree>> children)
  : _type(TokenType::NUMBER), _real(real), _children(std::move(children))
{
}

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

  return 100;
}

std::string
Tree::format()
{
  switch (_type)
  {
    case TokenType::NUMBER:
      return std::to_string(_real);

    case TokenType::OPERATOR:
      switch (_operator_type)
      {
        case OperatorType::UNARY_PLUS:
          if (_children[0]->precedence() > precedence())
            return '(' + _children[0]->format() + ')';
          else
            return _children[0]->format();

        case OperatorType::UNARY_MINUS:
          if (_children[0]->precedence() > precedence())
            return " -(" + _children[0]->format() + ')';
          else
            return " -" + _children[0]->format();

        default:
        {
          std::string out;
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

          return out;
        }
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
