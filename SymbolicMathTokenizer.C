#include "SymbolicMathTokenizer.h"

#include <vector>
#include <iostream>
#include <cmath>
#include <exception>

namespace SymbolicMath
{

Tokenizer::Tokenizer(const std::string expression)
  : _mpt_expression(expression + '\0'), _c(_mpt_expression.begin())
{
}

bool
Tokenizer::isDigit()
{
  const std::string digit("0123456789");
  return digit.find_first_of(*_c) != std::string::npos;
}

bool
Tokenizer::isOperator()
{
  const std::string operator_char("+-*/^!%<>=?:");
  return operator_char.find_first_of(*_c) != std::string::npos;
}

bool
Tokenizer::isOpeningBracket()
{
  const std::string parenthesis("([{");
  return parenthesis.find_first_of(*_c) != std::string::npos;
}

bool
Tokenizer::isClosingBracket()
{
  const std::string parenthesis(")]}");
  return parenthesis.find_first_of(*_c) != std::string::npos;
}

bool
Tokenizer::isAlphaFirst()
{
  return (*_c >= 'a' && *_c <= 'z') || (*_c >= 'A' && *_c <= 'Z');
}

bool
Tokenizer::isAlphaCont()
{
  return isAlphaFirst() || isDigit() || *_c == '_';
}

int
Tokenizer::getInteger()
{
  int integer = 0;
  while (isDigit())
    integer = 10 * integer + (*(_c++) - '0');
  return integer;
}

void
Tokenizer::skipWhite()
{
  // skip whitespace
  while (*_c == ' ')
    ++_c;
}

Token
Tokenizer::getToken()
{
  skipWhite();

  // remember the start of this token
  _token_start = std::distance(_mpt_expression.begin(), _c);

  // end of expression
  if (*_c == '\0')
    return makeToken(TokenType::END);

  // comma
  if (*_c == ',')
    return makeToken(TokenType::COMMA);

  if (isOperator())
  {
    std::string op(1, *(_c++));
    // parse >= <= == but NOT *-
    while (*_c == '=')
      op += *(_c++);
    return makeToken(identifyOperator(op));
  }

  if (isOpeningBracket())
    switch (*(_c++))
    {
      case '(':
        return makeToken(TokenType::OPENING_BRACKET, BracketType::ROUND);
      case '[':
        return makeToken(TokenType::OPENING_BRACKET, BracketType::SQUARE);
      case '{':
        return makeToken(TokenType::OPENING_BRACKET, BracketType::CURLY);
    }

  if (isClosingBracket())
    switch (*(_c++))
    {
      case ')':
        return makeToken(TokenType::CLOSING_BRACKET, BracketType::ROUND);
      case ']':
        return makeToken(TokenType::CLOSING_BRACKET, BracketType::SQUARE);
      case '}':
        return makeToken(TokenType::CLOSING_BRACKET, BracketType::CURLY);
    }

  // consume symbol
  if (isAlphaFirst())
  {
    std::string symbol(1, *(_c++));
    while (isAlphaCont())
      symbol += *(_c++);

    if (*_c == '(')
      return makeToken(identifyFunction(symbol));
    else
      return makeToken(TokenType::VARIABLE, symbol);
  }

  // consume number
  if (isDigit() || *_c == '.')
  {
    int integer = getInteger();
    int exponent = 0;
    Real decimals = 0.0;

    // decimal fraction
    if (*_c == '.')
    {
      ++_c;
      Real factor = 0.1;
      while (isDigit())
      {
        decimals += (*(_c++) - '0') * factor;
        factor *= 0.1;
      }
    }

    // scientific notation exponent
    if (*_c == 'e' || *_c == 'E')
    {
      ++_c;
      bool positive = true;
      if (*_c == '+')
        ++_c;
      else if (*_c == '-')
      {
        positive = false;
        ++_c;
      }

      exponent = getInteger() * (positive ? 1 : -1);
    }

    if (decimals == 0.0 && exponent == 0)
      return makeToken(integer);
    else
      return makeToken((integer + decimals) * std::pow(10.0, exponent));
  }

  // unable to parse
  return makeToken(TokenType::INVALID);
}

/**
 * Token building helpers
 */
Token
Tokenizer::makeToken(TokenType type)
{
  return Token(type, std::string(1, *(_c++)), _token_start);
}

Token
Tokenizer::makeToken(TokenType type, const std::string & data)
{
  return Token(type, data, _token_start);
}

Token
Tokenizer::makeToken(TokenType type, BracketType bracket_type)
{
  return Token(type, bracket_type, _token_start);
}

Token
Tokenizer::makeToken(OperatorType operator_type)
{
  return Token(TokenType::OPERATOR, operator_type, _token_start);
}

Token
Tokenizer::makeToken(FunctionType function_type)
{
  return Token(TokenType::FUNCTION, function_type, _token_start);
}

Token
Tokenizer::makeToken(int integer)
{
  return Token(TokenType::NUMBER, Real(integer), _token_start);
}

Token
Tokenizer::makeToken(Real real)
{
  return Token(TokenType::NUMBER, real, _token_start);
}

/**
 * Token constructors
 */
Token::Token(TokenType type, const std::string & string, std::size_t pos)
  : _type(type), _pos(pos), _string(string), _integer(0)
{
  // validate parameter for the given type
  switch (type)
  {
    case TokenType::VARIABLE:
    case TokenType::COMMA:
    case TokenType::INVALID:
    case TokenType::END:
      break;
    default:
      throw std::invalid_argument("string");
  }
}

Token::Token(TokenType type, OperatorType operator_type, std::size_t pos)
  : _type(type), _pos(pos), _operator_type(operator_type), _integer(0)
{
  if (type != TokenType::OPERATOR)
    throw std::invalid_argument("operator_type");
}

Token::Token(TokenType type, FunctionType function_type, std::size_t pos)
  : _type(type), _pos(pos), _function_type(function_type), _integer(0)
{
  if (type != TokenType::FUNCTION)
    throw std::invalid_argument("function_type");
}

Token::Token(TokenType type, BracketType bracket_type, std::size_t pos)
  : _type(type), _pos(pos), _bracket_type(bracket_type), _integer(0)
{
  if (type != TokenType::OPENING_BRACKET && type != TokenType::CLOSING_BRACKET)
    throw std::invalid_argument("bracket_type");
}

Token::Token(TokenType type, Real real, std::size_t pos)
  : _type(type), _pos(pos), _integer(0), _real(real)
{
  if (type != TokenType::NUMBER)
    throw std::invalid_argument("operator_type");
}

// end namespace SymbolicMath
}
