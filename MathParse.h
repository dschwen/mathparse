#ifndef MATHPARSE_H
#define MATHPARSE_H

#include "MathParseTokenizer.h"

#include <string>

class MathParse : public MathParseTokenizer
{
public:
  MathParse(const std::string expression);

protected:
  std::string _expression;
};

#endif // MATHPARSE_H
