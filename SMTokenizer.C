///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include "SMTokenizer.h"

#include <vector>
#include <iostream>
#include <cmath>
#include <exception>

namespace SymbolicMath
{

template <typename T>
Tokenizer<T>::Tokenizer(const std::string expression)
  : _mpt_expression(expression + '\0'), _c(_mpt_expression.begin())
{
}

template <typename T>
bool
Tokenizer<T>::isDigit()
{
  const std::string digit("0123456789");
  return digit.find_first_of(*_c) != std::string::npos;
}

template <typename T>
bool
Tokenizer<T>::isOperator()
{
  const std::string operator_char("+-*/^!%<>=?:&|;");
  return operator_char.find_first_of(*_c) != std::string::npos;
}

template <typename T>
bool
Tokenizer<T>::isBracket()
{
  return BracketToken<T>::opening(*_c) != BracketType::_INVALID ||
         BracketToken<T>::closing(*_c) != BracketType::_INVALID;
}

template <typename T>
bool
Tokenizer<T>::isAlphaFirst()
{
  return (*_c >= 'a' && *_c <= 'z') || (*_c >= 'A' && *_c <= 'Z');
}

template <typename T>
bool
Tokenizer<T>::isAlphaCont()
{
  return isAlphaFirst() || isDigit() || *_c == '_';
}

template <typename T>
int
Tokenizer<T>::getInteger()
{
  int integer = 0;
  while (isDigit())
    integer = 10 * integer + (*(_c++) - '0');
  return integer;
}

template <typename T>
void
Tokenizer<T>::skipWhite()
{
  // skip whitespace
  while (*_c == ' ')
    ++_c;
}

template <typename T>
TokenPtr<T>
Tokenizer<T>::getToken()
{
  skipWhite();

  // remember the start of this token
  _token_start = std::distance(_mpt_expression.begin(), _c);

  // end of expression
  if (*_c == '\0')
    return TokenPtr<T>(new EndToken<T>(pos()));

  // comma
  if (*_c == ',')
  {
    _c++;
    return TokenPtr<T>(new CommaToken<T>(pos()));
  }

  if (isOperator())
  {
    std::string op(1, *(_c++));
    // parse >= <= == but NOT *-
    while (*_c == '=')
      op += *(_c++);
    return OperatorToken<T>::build(op, pos());
  }

  if (isBracket())
    return TokenPtr<T>(new BracketToken<T>(*(_c++), pos()));

  // consume symbol
  if (isAlphaFirst())
  {
    std::string symbol(1, *(_c++));
    while (isAlphaCont())
      symbol += *(_c++);

    if (*_c == '(')
      return FunctionToken<T>::build(symbol, pos());
    else
      return TokenPtr<T>(new SymbolToken<T>(symbol, pos()));
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
      return TokenPtr<T>(new NumberToken<T>(integer, pos()));
    else
      return TokenPtr<T>(
          new NumberToken<T>((integer + decimals) * std::pow(10.0, exponent), pos()));
  }

  // unable to parse
  return TokenPtr<T>(new InvalidToken<T>(pos()));
}

template class Tokenizer<Real>;

} // namespace SymbolicMath
