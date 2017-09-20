#include "MathParse.h"

#include <vector>
#include <iostream>
#include <cmath>

typedef double Real;

MathParse::MathParse(const std::string expression)
  : MathParseTokenizer(expression), _expression(expression)
{
  const std::vector<std::string> type = {"OPERATOR",
                                         "FUNCTION",
                                         "COMMA   ",
                                         "OPEN_PARENS",
                                         "CLOSE_PARENS",
                                         "NUMBER  ",
                                         "VARIABLE",
                                         "INVALID ",
                                         "END"};

  Token token;
  do
  {
    token = getToken();
    std::cout << type[token._type] << "\t'" << token._data << "'\n";

    if (token._data == "ERR")
      std::cout << getError(token._pos, "Invalid token BLAH");

  } while (token._type != END);
}

std::string
MathParse::getError(std::size_t pos, const std::string & message, std::size_t width)
{
  // pad the expression to allow displaying pos 0 error markers
  const std::string padded = "  " + _expression;
  pos += 2;

  // half the width is where we center the error
  const std::size_t half = width / 2;

  // center the error
  const std::size_t head = std::min(half, pos);

  // align the error message
  const std::size_t indent = message.size() / 2 > head ? 0 : head - message.size() / 2;

  // build the error
  std::string error = padded.substr(pos - head, width) + '\n';
  error += std::string(head - 2, ' ') + "~~^~~\n";
  error += std::string(indent, ' ') + message + '\n';

  return error;
}
