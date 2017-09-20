#ifndef MATHPARSE_H
#define MATHPARSE_H

#include "MathParseAST.h"

#include <string>

class MathParse : public MathParseAST
{
public:
  MathParse(const std::string expression);

protected:
  std::string _expression;
};

#endif // MATHPARSE_H
