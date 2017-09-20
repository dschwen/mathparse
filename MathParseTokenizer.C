#include "MathParseTokenizer.h"
#include <vector>
#include <iostream>
#include <cmath>

typedef double Real;

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
  const std::string operator_char("+-*/^!%<>=?:");
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

  // end of expression
  if (*_c == '\0')
    return makeToken(TokenType::END, "");

  if (isOperator())
  {
    std::string op(1, *(_c++));
    // parse >= <= == but NOT *-
    while (*_c == '=')
      op += *(_c++);
    return makeToken(TokenType::OPERATOR, op);
  }

  if (isOpenParenthesis())
    return makeToken(TokenType::OPEN_PARENS);
  if (isCloseParenthesis())
    return makeToken(TokenType::CLOSE_PARENS);

  if (*_c == ',')
    return makeToken(TokenType::COMMA);

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
      return makeToken(TokenType::NUMBER, std::to_string(integer));
    else
      return makeToken(TokenType::NUMBER,
                       std::to_string((integer + decimals) * std::pow(10.0, exponent)));
  }

  // unable to parse
  return makeToken(TokenType::INVALID);
}

MathParseTokenizer::Token
MathParseTokenizer::makeToken(TokenType type)
{
  return Token(type, std::string(1, *(_c++)), std::distance(_mpt_expression.begin(), _c) - 1);
}

MathParseTokenizer::Token
MathParseTokenizer::makeToken(TokenType type, const std::string & data)
{
  return Token(type, data, std::distance(_mpt_expression.begin(), _c) - data.size());
}
