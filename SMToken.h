///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include "SMSymbols.h"
#include "SMNode.h"

#include <string>
#include <memory>

namespace SymbolicMath
{
typedef double Real;

template <typename T>
class Token;
template <typename T>
using TokenPtr = std::shared_ptr<Token<T>>;

/**
 * Token base class emmitted by the Tokenizer
 */
template <typename T>
class Token
{
public:
  Token(std::size_t pos) : _pos(pos) {}
  virtual ~Token() {}

  virtual bool isInvalid() { return false; }
  virtual bool isOperator() { return false; }
  virtual bool isOpeningBracket() { return false; }
  virtual bool isClosingBracket() { return false; }
  virtual bool isNumber() { return false; }
  virtual bool isFunction() { return false; }
  virtual bool isSymbol() { return false; }
  virtual bool isComma() { return false; }
  virtual bool isEnd() { return false; }

  virtual bool is(BinaryOperatorType) { return false; }
  virtual bool is(MultinaryOperatorType) { return false; }

  virtual Real asNumber() { return 0.0; };
  virtual std::string asString() { return ""; };

  std::size_t pos() { return _pos; }

  virtual unsigned short arguments() { return 0; }
  virtual BracketType bracketType() { return BracketType::_INVALID; }
  virtual unsigned short precedence() { return 0; }
  virtual bool isUnary() { return false; }
  virtual bool isLeftAssociative() { return false; }

  virtual Node<T> node(std::stack<Node<T>> & stack) { fatalError("Cannot build node from token"); }

protected:
  std::size_t _pos;
};

template <typename T>
class EndToken : public Token<T>
{
public:
  EndToken(std::size_t pos) : Token<T>(pos) {}
  bool isEnd() override { return true; }
};

template <typename T>
class InvalidToken : public Token<T>
{
public:
  InvalidToken(std::size_t pos) : Token<T>(pos) {}
  bool isInvalid() override { return true; }
};

template <typename T>
class CommaToken : public Token<T>
{
public:
  CommaToken(std::size_t pos) : Token<T>(pos) {}
  bool isComma() override { return true; }
};

template <typename T>
class SymbolToken : public Token<T>
{
public:
  SymbolToken(const std::string & string, std::size_t pos) : Token<T>(pos), _string(string) {}
  bool isSymbol() override { return true; }
  std::string asString() override { return _string; };

protected:
  std::string _string;
};

template <typename T>
class NumberToken : public Token<T>
{
public:
  NumberToken(Real number, std::size_t pos) : Token<T>(pos), _number(number) {}
  bool isNumber() override { return true; }
  Real asNumber() override { return _number; };

protected:
  Real _number;
};

template <typename T>
class BracketToken : public Token<T>
{
public:
  BracketToken(char bracket, std::size_t pos);
  bool isOpeningBracket() override { return _opening; }
  bool isClosingBracket() override { return !_opening; }

  BracketType bracketType() override { return _type; }

  static BracketType opening(char c);
  static BracketType closing(char c);

protected:
  bool _opening;
  BracketType _type;
};

template <typename T>
class OperatorToken : public Token<T>
{
  using Token<T>::Token;

public:
  bool isOperator() override { return true; }

  static TokenPtr<T> build(const std::string & string, std::size_t pos);
};

template <typename T>
class InvalidOperatorToken : public OperatorToken<T>
{
  using OperatorToken<T>::OperatorToken;

public:
  bool isInvalid() override { return true; }
};

template <typename T>
class OperatorTokenBase : public OperatorToken<T>
{
public:
  OperatorTokenBase(OperatorProperties prop, std::size_t pos) : OperatorToken<T>(pos), _prop(prop)
  {
  }
  bool isOperator() override { return true; }
  bool isInvalid() override { return true; }
  unsigned short precedence() override { return _prop._precedence; }
  bool isLeftAssociative() override { return _prop._left_associative; }
  std::string asString() override { return _prop._form; };

protected:
  OperatorProperties _prop;
};

template <typename T>
class UnaryOperatorToken : public OperatorTokenBase<T>
{
public:
  UnaryOperatorToken(std::pair<UnaryOperatorType, OperatorProperties> type_prop, std::size_t pos)
    : OperatorTokenBase<T>(type_prop.second, pos), _type(type_prop.first)
  {
  }
  bool isInvalid() override { return _type == UnaryOperatorType::_INVALID; }
  bool isUnary() override { return true; }
  unsigned short arguments() override { return 1; }
  static TokenPtr<T> build(UnaryOperatorType type, std::size_t pos);
  Node<T> node(std::stack<Node<T>> & stack) override;

protected:
  UnaryOperatorType _type;
};

template <typename T>
class BinaryOperatorToken : public OperatorTokenBase<T>
{
public:
  BinaryOperatorToken(std::pair<BinaryOperatorType, OperatorProperties> type_prop, std::size_t pos)
    : OperatorTokenBase<T>(type_prop.second, pos), _type(type_prop.first)
  {
  }
  bool isInvalid() override { return _type == BinaryOperatorType::_INVALID; }
  unsigned short arguments() override { return 2; }
  Node<T> node(std::stack<Node<T>> & stack) override;
  bool is(BinaryOperatorType type) override { return type == _type; }

protected:
  BinaryOperatorType _type;
};

template <typename T>
class MultinaryOperatorToken : public OperatorTokenBase<T>
{
public:
  MultinaryOperatorToken(std::pair<MultinaryOperatorType, OperatorProperties> type_prop,
                         std::size_t pos)
    : OperatorTokenBase<T>(type_prop.second, pos), _type(type_prop.first)
  {
  }
  bool isInvalid() override { return _type == MultinaryOperatorType::_INVALID; }
  unsigned short arguments() override { return 2; }
  Node<T> node(std::stack<Node<T>> & stack) override;
  bool is(MultinaryOperatorType type) override { return type == _type; }

protected:
  MultinaryOperatorType _type;
};

template <typename T>
class FunctionToken : public Token<T>
{
  using Token<T>::Token;

public:
  static TokenPtr<T> build(const std::string & string, std::size_t pos);
  bool isFunction() override { return true; }
  virtual bool isInvalid() override { return true; }
};

template <typename T>
class UnaryFunctionToken : public FunctionToken<T>
{
public:
  UnaryFunctionToken(UnaryFunctionType type, std::size_t pos) : FunctionToken<T>(pos), _type(type)
  {
  }
  bool isInvalid() override { return _type == UnaryFunctionType::_INVALID; }
  unsigned short arguments() override { return 1; }
  Node<T> node(std::stack<Node<T>> & stack) override;
  std::string asString() override { return stringify(_type); };

protected:
  UnaryFunctionType _type;
};

template <typename T>
class BinaryFunctionToken : public FunctionToken<T>
{
public:
  BinaryFunctionToken(BinaryFunctionType type, std::size_t pos) : FunctionToken<T>(pos), _type(type)
  {
  }
  bool isInvalid() override { return _type == BinaryFunctionType::_INVALID; }
  unsigned short arguments() override { return 2; }
  Node<T> node(std::stack<Node<T>> & stack) override;
  std::string asString() override { return stringify(_type); };

protected:
  BinaryFunctionType _type;
};

template <typename T>
class ConditionalToken : public FunctionToken<T>
{
public:
  ConditionalToken(ConditionalType type, std::size_t pos) : FunctionToken<T>(pos), _type(type) {}
  bool isInvalid() override { return _type == ConditionalType::_INVALID; }
  unsigned short arguments() override { return 3; }
  Node<T> node(std::stack<Node<T>> & stack) override;
  std::string asString() override { return stringify(_type); };

protected:
  ConditionalType _type;
};

} // namespace SymbolicMath
