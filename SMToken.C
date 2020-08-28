///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include "SMToken.h"

#include <vector>
#include <iostream>
#include <cmath>
#include <exception>

namespace SymbolicMath
{

template <typename T>
BracketToken<T>::BracketToken(char bracket, std::size_t pos) : Token<T>(pos), _opening(false)
{
  if ((_type = opening(bracket)) != BracketType::_INVALID)
    _opening = true;
  else
    _type = closing(bracket);
}

template <typename T>
BracketType
BracketToken<T>::opening(char c)
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

template <typename T>
BracketType
BracketToken<T>::closing(char c)
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

template <typename T>
TokenPtr<T>
OperatorToken<T>::build(const std::string & string, std::size_t pos)
{
  // search multinary and binary operators first
  for (auto & pair : _multinary_operators)
    if (pair.second._form == string)
      return TokenPtr<T>(new MultinaryOperatorToken<T>(pair, pos));

  for (auto & pair : _binary_operators)
    if (pair.second._form == string)
      return TokenPtr<T>(new BinaryOperatorToken<T>(pair, pos));

  // unary operators + and - are discriminated in the parser
  for (auto & pair : _unary_operators)
    if (pair.second._form == string)
      return TokenPtr<T>(new UnaryOperatorToken<T>(pair, pos));

  // return an invalid operator token
  return TokenPtr<T>(new InvalidOperatorToken<T>(pos));
}

template <typename T>
TokenPtr<T>
UnaryOperatorToken<T>::build(UnaryOperatorType type, std::size_t pos)
{
  // unary operators + and - are discriminated in the parser
  auto it = _unary_operators.find(type);

  if (it == _unary_operators.end())
    fatalError("Unknown unary operator");

  return TokenPtr<T>(new UnaryOperatorToken<T>(*it, pos));
}

template <typename T>
Node<T>
UnaryOperatorToken<T>::node(std::stack<Node<T>> & stack)
{
  auto arg0 = stack.top();
  stack.pop();
  return Node<T>(_type, arg0);
}

template <typename T>
Node<T>
BinaryOperatorToken<T>::node(std::stack<Node<T>> & stack)
{
  auto arg1 = stack.top();
  stack.pop();
  auto arg0 = stack.top();
  stack.pop();
  return Node<T>(_type, arg0, arg1);
}

template <typename T>
Node<T>
MultinaryOperatorToken<T>::node(std::stack<Node<T>> & stack)
{
  auto arg1 = stack.top();
  stack.pop();
  auto arg0 = stack.top();
  stack.pop();
  return Node<T>(_type, {arg0, arg1});
}

template <typename T>
Node<T>
UnaryFunctionToken<T>::node(std::stack<Node<T>> & stack)
{
  auto arg0 = stack.top();
  stack.pop();
  return Node<T>(_type, arg0);
}

template <typename T>
Node<T>
BinaryFunctionToken<T>::node(std::stack<Node<T>> & stack)
{
  auto arg1 = stack.top();
  stack.pop();
  auto arg0 = stack.top();
  stack.pop();
  return Node<T>(_type, arg0, arg1);
}

template <typename T>
Node<T>
ConditionalToken<T>::node(std::stack<Node<T>> & stack)
{
  auto arg2 = stack.top();
  stack.pop();
  auto arg1 = stack.top();
  stack.pop();
  auto arg0 = stack.top();
  stack.pop();
  return Node<T>(_type, arg0, arg1, arg2);
}

template <typename T>
TokenPtr<T>
FunctionToken<T>::build(const std::string & string, std::size_t pos)
{
  if (string == "if")
    return TokenPtr<T>(new ConditionalToken<T>(ConditionalType::IF, pos));

  for (auto & pair : _unary_functions)
    if (pair.second == string)
      return TokenPtr<T>(new UnaryFunctionToken<T>(pair.first, pos));

  for (auto & pair : _binary_functions)
    if (pair.second == string)
      return TokenPtr<T>(new BinaryFunctionToken<T>(pair.first, pos));

  // return an invalid function token
  return TokenPtr<T>(new FunctionToken<T>(pos));
}

template class BracketToken<Real>;
template class OperatorToken<Real>;
template class UnaryOperatorToken<Real>;
template class BinaryOperatorToken<Real>;
template class MultinaryOperatorToken<Real>;
template class FunctionToken<Real>;
template class UnaryFunctionToken<Real>;
template class BinaryFunctionToken<Real>;
template class ConditionalToken<Real>;

} // namespace SymbolicMath
