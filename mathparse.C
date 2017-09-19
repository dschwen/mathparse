#include "mathparse.h"
#include <vector>
#include <iostream>
#include <cmath>

typedef double Real;

MathParse::MathParse(const std::string expression) : _expression(expression + '\0'), _c(_expression.begin())
{
  const std::vector<std::string> type = {"OPERATOR", "FUNCTION", "COMMA   ", "OPEN_PARENS", "CLOSE_PARENS",
                                         "NUMBER  ", "VARIABLE", "INVALID ", "END"};

  Token token;
  do
  {
    token = getToken();
    std::cout << type[token._type] << '\t' << token._data << '\n';
  } while (token._type != END);
}

bool MathParse::isDigit()
{
  const std::string digit("0123456789");
  return digit.find_first_of(*_c) != std::string::npos;
}

bool MathParse::isOperator()
{
  const std::string operator_char("+-*/^!%<>=?:");
  return operator_char.find_first_of(*_c) != std::string::npos;
}

bool MathParse::isOpenParenthesis()
{
  const std::string parenthesis("([{");
  return parenthesis.find_first_of(*_c) != std::string::npos;
}

bool MathParse::isCloseParenthesis()
{
  const std::string parenthesis(")]}");
  return parenthesis.find_first_of(*_c) != std::string::npos;
}

bool MathParse::isAlphaFirst()
{
  return (*_c >= 'a' && *_c <= 'z') || (*_c >= 'A' && *_c <= 'Z');
}

bool MathParse::isAlphaCont()
{
  return isAlphaFirst() || isDigit() || *_c == '_';
}

int MathParse::getInteger()
{
  int integer = 0;
  while (isDigit())
    integer = 10 * integer + (*(_c++) - '0');
  return integer;
}

void MathParse::skipWhite()
{
  // skip whitespace
  while (*_c == ' ')
    ++_c;
}

MathParse::Token MathParse::getToken()
{
  skipWhite();

  // end of expression
  if (*_c == '\0')
    return Token(TokenType::END, "");

  if (isOperator())
  {
    std::string op(1, *(_c++));
    // parse >= <= == but NOT *-
    while (*_c == '=')
      op += *(_c++);
    return Token(TokenType::OPERATOR, op);
  }

  if (isOpenParenthesis())
    return Token(TokenType::OPEN_PARENS, *(_c++));
  if (isCloseParenthesis())
    return Token(TokenType::CLOSE_PARENS, *(_c++));

  if (*_c == ',')
  {
    _c++;
    return Token(TokenType::COMMA, "");
  }

  // consume symbol
  if (isAlphaFirst())
  {
    std::string symbol(1, *(_c++));
    while (isAlphaCont())
      symbol += *(_c++);

    if (isOpenParenthesis())
      return Token(TokenType::FUNCTION, symbol);
    else
      return Token(TokenType::VARIABLE, symbol);
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
      return Token(TokenType::NUMBER, std::to_string(integer));
    else
      return Token(TokenType::NUMBER, std::to_string((integer + decimals) * std::pow(10.0, exponent)));
  }

  // unable to parse
  return Token(TokenType::INVALID, *(_c++));
}

int main(int argc, char *argv[])
{
  if (argc != 2)
    return 1;

  MathParse parser(argv[1]);
  return 0;
}
