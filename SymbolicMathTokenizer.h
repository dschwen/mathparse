#ifndef SYMBOLICMATHTOKENIZER_H
#define SYMBOLICMATHTOKENIZER_H

#include "SymbolicMathSymbols.h"

#include <string>

namespace SymbolicMath
{
typedef double Real;

struct Token
{
  Token() : _type(TokenType::INVALID), _pos(0) {}
  Token(TokenType type, const std::string & string, std::size_t pos);
  Token(TokenType type, OperatorType operator_type, std::size_t pos);
  Token(TokenType type, FunctionType operator_type, std::size_t pos);
  Token(TokenType type, Real real, std::size_t pos);
  ~Token() {}

  TokenType _type;
  std::size_t _pos;

  // might drop this later and have the tokenizer look up variable IDs
  std::string _string;

  union {
    OperatorType _operator_type;
    FunctionType _function_type;
  };

  // for OPEN_PARENS this holds the argument counter
  int _integer;
  Real _real;
};

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
  bool isOpenParenthesis();
  bool isCloseParenthesis();
  bool isAlphaFirst();
  bool isAlphaCont();
  ///@}

  /// fetch and assemble a sequence of characters that constitute a valid integer number
  int getInteger();

  /// skip the next whitespace characters
  void skipWhite();

  Token makeToken(TokenType type);
  Token makeToken(TokenType type, const std::string & data);
  Token makeToken(OperatorType operator_type);
  Token makeToken(FunctionType function_type);
  Token makeToken(int integer);
  Token makeToken(Real real);
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHTOKENIZER_H
