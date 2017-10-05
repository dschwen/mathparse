#ifndef SYMBOLICMATHTOKENIZER_H
#define SYMBOLICMATHTOKENIZER_H

#include "SymbolicMathSymbols.h"
#include "SymbolicMathTree.h"

#include <string>

namespace SymbolicMath
{
typedef double Real;

/***************************************************
 * Token classes emmitted by the Tokenizer
 ***************************************************/

// enum class TokenType
// {
//   OPERATOR,
//   FUNCTION,
//   OPENING_BRACKET,
//   CLOSING_BRACKET,
//   NUMBER,
//   VARIABLE,
//   COMMA,
//   COMPONENT,
//   INVALID,
//   END
// };

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

  virtual bool is(BinaryOperatorNodeType) { return false; }
  virtual bool is(MultinaryOperatorNodeType) { return false; }

  virtual Real asNumber() { return 0.0; };
  virtual std::string asString() { return ""; };

  std::size_t pos() { return _pos; }

  virtual unsigned short arguments() { return 0; }
  virtual BracketType bracketType() { return BracketType::_INVALID; }
  virtual unsigned short precedence() { return 0; }
  virtual bool isUnary() { return false; }
  virtual bool isLeftAssociative() { return false; }

  virtual Node * node(std::stack<Node *> & stack) { return nullptr; }

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

class InvalidOperatorToken : public Token
{
  using Token::Token;

public:
  bool isOperator() override { return true; }
  bool isInvalid() override { return true; }
};

class OperatorToken : public Token
{
public:
  OperatorToken(OperatorProperties prop, std::size_t pos) : Token(pos), _prop(prop) {}
  bool isOperator() override { return true; }
  bool isInvalid() override { return true; }
  unsigned short precedence() override { return _prop._precedence; }
  bool isLeftAssociative() override { return _prop._left_associative; }
  static OperatorToken * build(const std::string & string, std::size_t pos);

protected:
  OperatorProperties _prop;
};

class UnaryOperatorToken : public OperatorToken
{
public:
  UnaryOperatorToken(std::pair<UnaryOperatorNodeType, OperatorProperties> type_prop,
                     std::size_t pos)
    : OperatorToken(type_prop.second, pos), _type(type_prop.first)
  {
  }
  bool isInvalid() override { return _type == UnaryOperatorNodeType::_INVALID; }
  bool isUnary() override { return true; }
  unsigned short arguments() override { return 1; }
  Node * node(std::stack<Node *> & stack) override { return new UnaryOperatorNode(_type, stack); }

protected:
  UnaryOperatorNodeType _type;
};

class BinaryOperatorToken : public OperatorToken
{
public:
  BinaryOperatorToken(std::pair<BinaryOperatorNodeType, OperatorProperties> type_prop,
                      std::size_t pos)
    : OperatorToken(type_prop.second, pos), _type(type_prop.first)
  {
  }
  bool isInvalid() override { return _type == BinaryOperatorNodeType::_INVALID; }
  unsigned short arguments() override { return 2; }
  Node * node(std::stack<Node *> & stack) override { return new BinaryOperatorNode(_type, stack); }
  bool is(BinaryOperatorNodeType type) override { return type == _type; }

protected:
  BinaryOperatorNodeType _type;
};

class MultinaryOperatorToken : public OperatorToken
{
public:
  MultinaryOperatorToken(std::pair<MultinaryOperatorNodeType, OperatorProperties> type_prop,
                         std::size_t pos)
    : OperatorToken(type_prop.second, pos), _type(type_prop.first)
  {
  }
  bool isInvalid() override { return _type == MultinaryOperatorNodeType::_INVALID; }
  unsigned short arguments() override { return 2; }
  Node * node(std::stack<Node *> & stack) override
  {
    return new MultinaryOperatorNode(_type, stack, 2);
  }
  bool is(MultinaryOperatorNodeType type) override { return type == _type; }

protected:
  MultinaryOperatorNodeType _type;
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
  UnaryFunctionToken(UnaryFunctionNodeType type, std::size_t pos) : FunctionToken(pos), _type(type)
  {
  }
  bool isInvalid() override { return _type == UnaryFunctionNodeType::_INVALID; }
  unsigned short arguments() override { return 1; }
  Node * node(std::stack<Node *> & stack) override { return new UnaryFunctionNode(_type, stack); }

protected:
  UnaryFunctionNodeType _type;
};

class BinaryFunctionToken : public FunctionToken
{
public:
  BinaryFunctionToken(BinaryFunctionNodeType type, std::size_t pos)
    : FunctionToken(pos), _type(type)
  {
  }
  bool isInvalid() override { return _type == BinaryFunctionNodeType::_INVALID; }
  unsigned short arguments() override { return 2; }
  Node * node(std::stack<Node *> & stack) override { return new BinaryFunctionNode(_type, stack); }

protected:
  BinaryFunctionNodeType _type;
};

class ConditionalToken : public FunctionToken
{
public:
  ConditionalToken(ConditionalNodeType type, std::size_t pos) : FunctionToken(pos), _type(type) {}
  bool isInvalid() override { return _type == ConditionalNodeType::_INVALID; }
  unsigned short arguments() override { return 3; }
  Node * node(std::stack<Node *> & stack) override { return new ConditionalNode(_type, stack); }

protected:
  ConditionalNodeType _type;
};

/***************************************************
 * Tokenizer that parses an expression string
 ***************************************************/

class Tokenizer
{
public:
  Tokenizer(const std::string expression);

  /// gets the next complete token from the expression
  Token * getToken();

private:
  const std::string _mpt_expression;
  std::string::const_iterator _c;
  std::size_t _token_start;

  ///@{ classification functions for the next expression character
  bool isDigit();
  bool isOperator();
  bool isBracket();
  bool isAlphaFirst();
  bool isAlphaCont();
  ///@}

  /// fetch and assemble a sequence of characters that constitute a valid integer number
  int getInteger();

  /// skip the next whitespace characters
  void skipWhite();

  /// curent tokenizer position
  std::size_t pos() { return std::distance(_mpt_expression.begin(), _c); }
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHTOKENIZER_H
