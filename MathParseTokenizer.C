#include "MathParseTokenizer.h"
#include <vector>
#include <iostream>
#include <cmath>
#include <exception>

MathParseTokenizer::MathParseTokenizer(const std::string expression)
  : _mpt_expression(expression + '\0'), _c(_mpt_expression.begin())
{
}

bool
MathParseTokenizer::isDigit()
{
  const std::string digit("0123456789");
  return digit.find_first_of(*_c) != std::string::npos;
}

bool
MathParseTokenizer::isOperator()
{
  const std::string operator_char("+-*/^!%<>=?:,");
  return operator_char.find_first_of(*_c) != std::string::npos;
}

bool
MathParseTokenizer::isOpenParenthesis()
{
  const std::string parenthesis("([{");
  return parenthesis.find_first_of(*_c) != std::string::npos;
}

bool
MathParseTokenizer::isCloseParenthesis()
{
  const std::string parenthesis(")]}");
  return parenthesis.find_first_of(*_c) != std::string::npos;
}

bool
MathParseTokenizer::isAlphaFirst()
{
  return (*_c >= 'a' && *_c <= 'z') || (*_c >= 'A' && *_c <= 'Z');
}

bool
MathParseTokenizer::isAlphaCont()
{
  return isAlphaFirst() || isDigit() || *_c == '_';
}

int
MathParseTokenizer::getInteger()
{
  int integer = 0;
  while (isDigit())
    integer = 10 * integer + (*(_c++) - '0');
  return integer;
}

void
MathParseTokenizer::skipWhite()
{
  // skip whitespace
  while (*_c == ' ')
    ++_c;
}

MathParseTokenizer::Token
MathParseTokenizer::getToken()
{
  skipWhite();

  // remember the start of this token
  _token_start = std::distance(_mpt_expression.begin(), _c);

  // end of expression
  if (*_c == '\0')
    return makeToken(TokenType::END);

  if (isOperator())
  {
    std::string op(1, *(_c++));
    // parse >= <= == but NOT *-
    while (*_c == '=')
      op += *(_c++);
    return makeToken(identifyOperator(op));
  }

  if (isOpenParenthesis())
    return makeToken(TokenType::OPEN_PARENS);
  if (isCloseParenthesis())
    return makeToken(TokenType::CLOSE_PARENS);

  // consume symbol
  if (isAlphaFirst())
  {
    std::string symbol(1, *(_c++));
    while (isAlphaCont())
      symbol += *(_c++);

    if (isOpenParenthesis())
      return makeToken(TokenType::FUNCTION, symbol);
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
MathParseTokenizer::Token
MathParseTokenizer::makeToken(TokenType type)
{
  return Token(type, std::string(1, *(_c++)), _token_start);
}

MathParseTokenizer::Token
MathParseTokenizer::makeToken(TokenType type, const std::string & data)
{
  return Token(type, data, _token_start);
}

MathParseTokenizer::Token
MathParseTokenizer::makeToken(OperatorType operator_type)
{
  return Token(TokenType::OPERATOR, operator_type, _token_start);
}

MathParseTokenizer::Token
MathParseTokenizer::makeToken(int integer)
{
  return Token(TokenType::NUMBER, Real(integer), _token_start);
}

MathParseTokenizer::Token
MathParseTokenizer::makeToken(Real real)
{
  return Token(TokenType::NUMBER, real, _token_start);
}

/**
 * Token constructors
 */
MathParseTokenizer::Token::Token(TokenType type, const std::string & string, std::size_t pos)
  : _type(type), _string(string), _pos(pos)
{
  // validate parameter for the given type
  switch (type)
  {
    case TokenType::FUNCTION:
    case TokenType::VARIABLE:
    case TokenType::OPEN_PARENS:
    case TokenType::CLOSE_PARENS:
    case TokenType::INVALID:
    case TokenType::END:
      break;
    default:
      throw std::invalid_argument("string");
  }
}

MathParseTokenizer::Token::Token(TokenType type, OperatorType operator_type, std::size_t pos)
  : _type(type), _operator_type(operator_type), _pos(pos)
{
  if (type != TokenType::OPERATOR)
    throw std::invalid_argument("operator_type");
}

MathParseTokenizer::Token::Token(TokenType type, Real real, std::size_t pos)
  : _type(type), _real(real), _pos(pos)
{
  if (type != TokenType::NUMBER)
    throw std::invalid_argument("operator_type");
}
