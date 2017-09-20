#include "MathParse.h"

#include <vector>
#include <iostream>
#include <cmath>

typedef double Real;

MathParse::MathParse(const std::string expression)
  : MathParseAST(expression), _expression(expression)
{
}
