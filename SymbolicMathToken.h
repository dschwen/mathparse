#ifndef SYMBOLICMATHTOKEN_H
#define SYMBOLICMATHTOKEN_H

#include "SymbolicMathSymbols.h"
#include "SymbolicMathNode.h"

#include <string>

namespace SymbolicMath
{
typedef double Real;

/**
 * Token base class emmitted by the Tokenizer
 */
class Token
{
public:
  Token(std::size_t pos) : _pos(pos) {}

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

  virtual NodeData * node(std::stack<NodeData *> & stack) { return nullptr; }

protected:
  std::size_t _pos;
};

using TokenPtr = std::shared_ptr<Token>;

class EndToken : public Token
{
public:
  EndToken(std::size_t pos) : Token(pos) {}
  bool isEnd() override { return true; }
};

class InvalidToken : public Token
{
public:
  InvalidToken(std::size_t pos) : Token(pos) {}
  bool isInvalid() override { return true; }
};

class CommaToken : public Token
{
public:
  CommaToken(std::size_t pos) : Token(pos) {}
  bool isComma() override { return true; }
};

class SymbolToken : public Token
{
public:
  SymbolToken(const std::string & string, std::size_t pos) : Token(pos), _string(string) {}
  bool isSymbol() override { return true; }
  std::string asString() override { return _string; };

protected:
  std::string _string;
};

class NumberToken : public Token
{
public:
  NumberToken(Real number, std::size_t pos) : Token(pos), _number(number) {}
  bool isNumber() override { return true; }
  Real asNumber() override { return _number; };

protected:
  Real _number;
};

class BracketToken : public Token
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

class OperatorToken : public Token
{
  using Token::Token;

public:
  bool isOperator() override { return true; }

  static OperatorToken * build(const std::string & string, std::size_t pos);
};

class InvalidOperatorToken : public OperatorToken
{
  using OperatorToken::OperatorToken;

public:
  bool isInvalid() override { return true; }
};

class OperatorTokenBase : public OperatorToken
{
public:
  OperatorTokenBase(OperatorProperties prop, std::size_t pos) : OperatorToken(pos), _prop(prop) {}
  bool isOperator() override { return true; }
  bool isInvalid() override { return true; }
  unsigned short precedence() override { return _prop._precedence; }
  bool isLeftAssociative() override { return _prop._left_associative; }
  std::string asString() override { return _prop._form; };

protected:
  OperatorProperties _prop;
};

class UnaryOperatorToken : public OperatorTokenBase
{
public:
  UnaryOperatorToken(std::pair<UnaryOperatorType, OperatorProperties> type_prop, std::size_t pos)
    : OperatorTokenBase(type_prop.second, pos), _type(type_prop.first)
  {
  }
  bool isInvalid() override { return _type == UnaryOperatorType::_INVALID; }
  bool isUnary() override { return true; }
  unsigned short arguments() override { return 1; }
  static OperatorToken * build(UnaryOperatorType type, std::size_t pos);
  NodeData * node(std::stack<NodeData *> & stack) override
  {
    return new UnaryOperatorNode(_type, stack);
  }

protected:
  UnaryOperatorType _type;
};

class BinaryOperatorToken : public OperatorTokenBase
{
public:
  BinaryOperatorToken(std::pair<BinaryOperatorType, OperatorProperties> type_prop, std::size_t pos)
    : OperatorTokenBase(type_prop.second, pos), _type(type_prop.first)
  {
  }
  bool isInvalid() override { return _type == BinaryOperatorType::_INVALID; }
  unsigned short arguments() override { return 2; }
  NodeData * node(std::stack<NodeData *> & stack) override
  {
    return new BinaryOperatorNode(_type, stack);
  }
  bool is(BinaryOperatorType type) override { return type == _type; }

protected:
  BinaryOperatorType _type;
};

class MultinaryOperatorToken : public OperatorTokenBase
{
public:
  MultinaryOperatorToken(std::pair<MultinaryOperatorType, OperatorProperties> type_prop,
                         std::size_t pos)
    : OperatorTokenBase(type_prop.second, pos), _type(type_prop.first)
  {
  }
  bool isInvalid() override { return _type == MultinaryOperatorType::_INVALID; }
  unsigned short arguments() override { return 2; }
  NodeData * node(std::stack<NodeData *> & stack) override
  {
    return new MultinaryOperatorNode(_type, stack, 2);
  }
  bool is(MultinaryOperatorType type) override { return type == _type; }

protected:
  MultinaryOperatorType _type;
};

class FunctionToken : public Token
{
  using Token::Token;

public:
  static FunctionToken * build(const std::string & string, std::size_t pos);
  bool isFunction() override { return true; }
  virtual bool isInvalid() override { return true; }
};

class UnaryFunctionToken : public FunctionToken
{
public:
  UnaryFunctionToken(UnaryFunctionType type, std::size_t pos) : FunctionToken(pos), _type(type) {}
  bool isInvalid() override { return _type == UnaryFunctionType::_INVALID; }
  unsigned short arguments() override { return 1; }
  NodeData * node(std::stack<NodeData *> & stack) override
  {
    return new UnaryFunctionNode(_type, stack);
  }
  std::string asString() override { return stringify(_type); };

protected:
  UnaryFunctionType _type;
};

class BinaryFunctionToken : public FunctionToken
{
public:
  BinaryFunctionToken(BinaryFunctionType type, std::size_t pos) : FunctionToken(pos), _type(type) {}
  bool isInvalid() override { return _type == BinaryFunctionType::_INVALID; }
  unsigned short arguments() override { return 2; }
  NodeData * node(std::stack<NodeData *> & stack) override
  {
    return new BinaryFunctionNode(_type, stack);
  }
  std::string asString() override { return stringify(_type); };

protected:
  BinaryFunctionType _type;
};

class ConditionalToken : public FunctionToken
{
public:
  ConditionalToken(ConditionalType type, std::size_t pos) : FunctionToken(pos), _type(type) {}
  bool isInvalid() override { return _type == ConditionalType::_INVALID; }
  unsigned short arguments() override { return 3; }
  NodeData * node(std::stack<NodeData *> & stack) override
  {
    return new ConditionalNode(_type, stack);
  }
  std::string asString() override { return stringify(_type); };

protected:
  ConditionalType _type;
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHTOKENIZER_H
