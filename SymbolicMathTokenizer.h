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

  bool isInvalid() { return false; }
  bool isOperator() { return false; }
  bool isOpeningBracket() { return false; }
  bool isClosingBracket() { return false; }
  bool isNumber() { return false; }
  bool isFunction() { return false; }
  bool isSymbol() { return false; }
  bool isComma() { return false; }
  bool isEnd() { return false; }

  Real asNumber() { return NAN; };
  std::string asString() { return ""; };

  unsigned short arguments() { return 0; }
  Node * node(std::stack<Node *> & stack) { return nullptr; }

protected:
  std::size_t _pos;
};

class EndToken : public Token
{
public:
  EndToken(std::size_t pos) : Token(pos) {}
  bool isEnd() { return true; }
};

class InvalidToken : public Token
{
public:
  InvalidToken(std::size_t pos) : Token(pos) {}
  bool isInvalid() { return true; }
};

class SymbolToken : public Token
{
public:
  SymbolToken(const std::string & string, std::size_t pos) : Token(pos), _string(string) {}
  bool isSymbol() { return true; }
  std::string asString() { return _string; };

protected:
  std::string _string;
};

class NumberToken : public Token
{
  StringToken(Real number, std::size_t pos) : Token(pos), _number(number) {}
  bool isNumber() { return true; }
  Real asNumber() { return _number; };

protected:
  Real _number;
};

class BracketToken : public Token
{
  StringToken(char bracket, std::size_t pos);
  bool isOpeningBracket() { return _opening; }
  bool isClosingBracket() { return !_opening; }

protected:
  bool _opening;
  BracketType _type;
};

class OperatorToken : public Token
{
  using Token::Token;

public:
  bool isOperator() { return true; }
  static build(const std::string & string, std::size_t pos);
};

class UnaryOperatorToken : public OperatorToken
{
public:
  UnaryOperatorToken(UnaryOperatorNodeType type, std::size_t pos) : OperatorToken(pos), _type(type)
  {
  }
  bool isInvalid() { return _type == UnaryOperatorNodeType::_INVALID; }
  unsigned short arguments() { return 1; }
  Node * node(std::stack<Node *> & stack) { return new UnaryOperatorNode(_type, stack); }

protected:
  UnaryOperatorNodeType _type;
};

class BinaryOperatorToken : public OperatorToken
{
public:
  BinaryOperatorToken(BinaryOperatorNodeType type, std::size_t pos)
    : OperatorToken(pos), _type(type)
  {
  }
  bool isInvalid() { return _type == BinaryOperatorNodeType::_INVALID; }
  unsigned short arguments() { return 2; }
  Node * node(std::stack<Node *> & stack) { return new BinaryOperatorNode(_type, stack); }

protected:
  BinaryOperatorNodeType _type;
};

class MultinaryOperatorToken : public OperatorToken
{
public:
  MultinaryOperatorToken(MultinaryOperatorNodeType type, std::size_t pos)
    : OperatorToken(pos), _type(type)
  {
  }
  bool isInvalid() { return _type == MultinaryOperatorNodeType::_INVALID; }
  unsigned short arguments() { return 2; }
  Node * node(std::stack<Node *> & stack) { return new MultinaryOperatorNode(_type, stack); }

protected:
  MultinaryOperatorNodeType _type;
};

class FunctionToken : public Token
{
  using Token::Token;

public:
  static build(const std::string & string, std::size_t pos);
  bool isFunction() { return true; }
};

class UnaryFunctionToken : public FunctionToken
{
public:
  UnaryFunctionToken(UnaryFunctionNodeType type, std::size_t pos) : FunctionToken(pos), _type(type)
  {
  }
  bool isInvalid() { return _type == UnaryFunctionNodeType::_INVALID; }
  unsigned short arguments() { return 1; }
  Node * node(std::stack<Node *> & stack) { return new UnaryFunctionNode(_type, stack); }

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
  bool isInvalid() { return _type == BinaryFunctionNodeType::_INVALID; }
  unsigned short arguments() { return 2; }
  Node * node(std::stack<Node *> & stack) { return new BinaryFunctionNode(_type, stack); }

protected:
  BinaryFunctionNodeType _type;
};

class ConditionalToken : public FunctionToken
{
public:
  ConditionalToken(ConditionalNodeType type, std::size_t pos) : FunctionToken(pos), _type(type) {}
  bool isInvalid() { return _type == ConditionalNodeType::_INVALID; }
  unsigned short arguments() { return 3; }
  Node * node(std::stack<Node *> & stack) { return new ConditionalNode(_type, stack); }

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
  Token getToken();

private:
  const std::string _mpt_expression;
  std::string::const_iterator _c;
  std::size_t _token_start;

  ///@{ classification functions for the next expression character
  bool isDigit();
  bool isOperator();
  bool isOpeningBracket();
  bool isClosingBracket();
  bool isAlphaFirst();
  bool isAlphaCont();
  ///@}

  /// fetch and assemble a sequence of characters that constitute a valid integer number
  int getInteger();

  /// skip the next whitespace characters
  void skipWhite();

  Token makeToken(TokenType type);
  Token makeToken(TokenType type, const std::string & data);
  Token makeToken(TokenType type, BracketType function_type);
  Token makeToken(OperatorType operator_type);
  Token makeToken(FunctionType function_type);
  Token makeToken(int integer);
  Token makeToken(Real real);
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHTOKENIZER_H
