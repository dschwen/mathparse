#include "SymbolicMathTokenizer.h"
#include <vector>
#include <iostream>
#include <cmath>
#include <exception>

SymbolicMathTokenizer::SymbolicMathTokenizer(const std::string expression)
  : _mpt_expression(expression + '\0'), _c(_mpt_expression.begin())
{
}

bool
SymbolicMathTokenizer::isDigit()
{
  const std::string digit("0123456789");
  return digit.find_first_of(*_c) != std::string::npos;
}

bool
SymbolicMathTokenizer::isOperator()
{
  const std::string operator_char("+-*/^!%<>=?:");
  return operator_char.find_first_of(*_c) != std::string::npos;
}

bool
SymbolicMathTokenizer::isOpenParenthesis()
{
  const std::string parenthesis("([{");
  return parenthesis.find_first_of(*_c) != std::string::npos;
}

bool
SymbolicMathTokenizer::isCloseParenthesis()
{
  const std::string parenthesis(")]}");
  return parenthesis.find_first_of(*_c) != std::string::npos;
}

bool
SymbolicMathTokenizer::isAlphaFirst()
{
  return (*_c >= 'a' && *_c <= 'z') || (*_c >= 'A' && *_c <= 'Z');
}

bool
SymbolicMathTokenizer::isAlphaCont()
{
  return isAlphaFirst() || isDigit() || *_c == '_';
}

int
SymbolicMathTokenizer::getInteger()
{
  int integer = 0;
  while (isDigit())
    integer = 10 * integer + (*(_c++) - '0');
  return integer;
}

void
SymbolicMathTokenizer::skipWhite()
{
  // skip whitespace
  while (*_c == ' ')
    ++_c;
}

SymbolicMathTokenizer::Token
SymbolicMathTokenizer::getToken()
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
SymbolicMathTokenizer::Token
SymbolicMathTokenizer::makeToken(TokenType type)
{
  return Token(type, std::string(1, *(_c++)), _token_start);
}

SymbolicMathTokenizer::Token
SymbolicMathTokenizer::makeToken(TokenType type, const std::string & data)
{
  return Token(type, data, _token_start);
}

SymbolicMathTokenizer::Token
SymbolicMathTokenizer::makeToken(OperatorType operator_type)
{
  return Token(TokenType::OPERATOR, operator_type, _token_start);
}

SymbolicMathTokenizer::Token
SymbolicMathTokenizer::makeToken(FunctionType function_type)
{
  return Token(TokenType::FUNCTION, function_type, _token_start);
}

SymbolicMathTokenizer::Token
SymbolicMathTokenizer::makeToken(int integer)
{
  return Token(TokenType::NUMBER, Real(integer), _token_start);
}

SymbolicMathTokenizer::Token
SymbolicMathTokenizer::makeToken(Real real)
{
  return Token(TokenType::NUMBER, real, _token_start);
}

/**
 * Token constructors
 */
SymbolicMathTokenizer::Token::Token(TokenType type, const std::string & string, std::size_t pos)
  : _type(type), _string(string), _integer(0), _pos(pos)
{
  // validate parameter for the given type
  switch (type)
  {
    case TokenType::VARIABLE:
    case TokenType::OPEN_PARENS:
    case TokenType::CLOSE_PARENS:
    case TokenType::COMMA:
    case TokenType::INVALID:
    case TokenType::END:
      break;
    default:
      throw std::invalid_argument("string");
  }
}

SymbolicMathTokenizer::Token::Token(TokenType type, OperatorType operator_type, std::size_t pos)
  : _type(type), _operator_type(operator_type), _integer(0), _pos(pos)
{
  if (type != TokenType::OPERATOR)
    throw std::invalid_argument("operator_type");
}

SymbolicMathTokenizer::Token::Token(TokenType type, FunctionType function_type, std::size_t pos)
  : _type(type), _function_type(function_type), _integer(0), _pos(pos)
{
  if (type != TokenType::FUNCTION)
    throw std::invalid_argument("function_type");
}

SymbolicMathTokenizer::Token::Token(TokenType type, Real real, std::size_t pos)
  : _type(type), _integer(0), _real(real), _pos(pos)
{
  if (type != TokenType::NUMBER)
    throw std::invalid_argument("operator_type");
}
