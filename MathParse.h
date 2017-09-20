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

  std::string getError(std::size_t pos, const std::string & message, std::size_t width = 80);
};

#endif // MATHPARSE_H
