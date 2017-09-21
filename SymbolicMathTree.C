#include "SymbolicMathTree.h"

Tree::Tree(OperatorType operator_type, std::vector<std::unique_ptr<Tree>> children)
  : _type(TokenType::OPERATOR), _operator_type(operator_type), _children(children)
{
}

Tree::Tree(FunctionType function_type, std::vector<std::unique_ptr<Tree>> children)
  : _type(TokenType::FUNCTION), _function_type(function_type), _children(children)
{
}

Tree::Tree(Real real, std::vector<std::unique_ptr<Tree>> children)
  : _type(TokenType::NUMBER), _rel(real) _children(children)
{
}

Real
Tree::value()
{
}

std::string
Tree::format()
{
  switch (_type)
  {
    case TokenType::NUMBER:
      return std::to_string(_real);
    case TokenType::OPERATOR:
      if (_operator_type == OperatorType::UNARY_PLUS)
        return _children[0].format();
      if (_operator_type == OperatorType::UNARY_MINUS)
        return "-" + _children[0].format();
      return _children[0].format() + operatorProperty(_operator_type)._form + _children[0].format();
    case TokenType::Function:
      std::string out = functionProperty(_function_type)._form;
      unsigned int arguments = functionProperty(_function_type)._arguments;

      return out;
  }
}
