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
    std::cout << type[token._type] << '\t' << token._data << '\n';
  } while (token._type != END);
}
