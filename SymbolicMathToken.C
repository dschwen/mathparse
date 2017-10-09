#include "SymbolicMathToken.h"

#include <vector>
#include <iostream>
#include <cmath>
#include <exception>

namespace SymbolicMath
{

BracketToken::BracketToken(char bracket, std::size_t pos) : Token(pos), _opening(false)
{
  if ((_type = opening(bracket)) != BracketType::_INVALID)
    _opening = true;
  else
    _type = closing(bracket);
}

BracketType
BracketToken::opening(char c)
{
  switch (c)
  {
    case '(':
      return BracketType::ROUND;
    case '[':
      return BracketType::SQUARE;
    case '{':
      return BracketType::CURLY;
    default:
      return BracketType::_INVALID;
  }
}

BracketType
BracketToken::closing(char c)
{
  switch (c)
  {
    case ')':
      return BracketType::ROUND;
    case ']':
      return BracketType::SQUARE;
    case '}':
      return BracketType::CURLY;
    default:
      return BracketType::_INVALID;
  }
}

OperatorToken *
OperatorToken::build(const std::string & string, std::size_t pos)
{
  // search multinary and binary operators first
  for (auto & pair : _multinary_operators)
    if (pair.second._form == string)
      return new MultinaryOperatorToken(pair, pos);

  for (auto & pair : _binary_operators)
    if (pair.second._form == string)
      return new BinaryOperatorToken(pair, pos);

  // unary operators + and - are discriminated in the parser
  for (auto & pair : _unary_operators)
    if (pair.second._form == string)
      return new UnaryOperatorToken(pair, pos);

  // return an invalid operator token
  return new InvalidOperatorToken(pos);
}

OperatorToken *
UnaryOperatorToken::build(UnaryOperatorType type, std::size_t pos)
{
  // unary operators + and - are discriminated in the parser
  auto it = _unary_operators.find(type);

  if (it == _unary_operators.end())
    fatalError("Unknown unary operator");

  return new UnaryOperatorToken(*it, pos);
}

Node
UnaryOperatorToken::node(std::stack<Node> & stack)
{
  auto arg0 = stack.top();
  stack.pop();
  return Node(_type, arg0);
}

Node
BinaryOperatorToken::node(std::stack<Node> & stack)
{
  auto arg1 = stack.top();
  stack.pop();
  auto arg0 = stack.top();
  stack.pop();
  return Node(_type, arg0, arg1);
}

Node
MultinaryOperatorToken::node(std::stack<Node> & stack)
{
  auto arg1 = stack.top();
  stack.pop();
  auto arg0 = stack.top();
  stack.pop();
  return Node(_type, {arg0, arg1});
}

Node
UnaryFunctionToken::node(std::stack<Node> & stack)
{
  auto arg0 = stack.top();
  stack.pop();
  return Node(_type, arg0);
}

Node
BinaryFunctionToken::node(std::stack<Node> & stack)
{
  auto arg1 = stack.top();
  stack.pop();
  auto arg0 = stack.top();
  stack.pop();
  return Node(_type, arg0, arg1);
}

Node
ConditionalToken::node(std::stack<Node> & stack)
{
  auto arg2 = stack.top();
  stack.pop();
  auto arg1 = stack.top();
  stack.pop();
  auto arg0 = stack.top();
  stack.pop();
  return Node(_type, arg0, arg1, arg2);
}

FunctionToken *
FunctionToken::build(const std::string & string, std::size_t pos)
{
  if (string == "if")
    return new ConditionalToken(ConditionalType::IF, pos);

  for (auto & pair : _unary_functions)
    if (pair.second == string)
      return new UnaryFunctionToken(pair.first, pos);

  for (auto & pair : _binary_functions)
    if (pair.second == string)
      return new BinaryFunctionToken(pair.first, pos);

  // return an invalid function token
  return new FunctionToken(pos);
}

// end namespace SymbolicMath
}
