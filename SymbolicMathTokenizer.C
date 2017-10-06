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
Tokenizer::isBracket()
{
  return BracketToken::opening(*_c) != BracketType::_INVALID ||
         BracketToken::closing(*_c) != BracketType::_INVALID;
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

Token *
Tokenizer::getToken()
{
  skipWhite();

  // remember the start of this token
  _token_start = std::distance(_mpt_expression.begin(), _c);

  // end of expression
  if (*_c == '\0')
    return new EndToken(pos());

  // comma
  if (*_c == ',')
  {
    _c++;
    return new CommaToken(pos());
  }

  if (isOperator())
  {
    std::string op(1, *(_c++));
    // parse >= <= == but NOT *-
    while (*_c == '=')
      op += *(_c++);
    return OperatorToken::build(op, pos());
  }

  if (isBracket())
    return new BracketToken(*(_c++), pos());

  // consume symbol
  if (isAlphaFirst())
  {
    std::string symbol(1, *(_c++));
    while (isAlphaCont())
      symbol += *(_c++);

    if (*_c == '(')
      return FunctionToken::build(symbol, pos());
    else
      return new SymbolToken(symbol, pos());
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
      return new NumberToken(integer, pos());
    else
      return new NumberToken((integer + decimals) * std::pow(10.0, exponent), pos());
  }

  // unable to parse
  return new InvalidToken(pos());
}

// end namespace SymbolicMath
}
