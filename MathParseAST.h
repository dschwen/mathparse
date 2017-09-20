#ifndef MATHPARSEAST_H
#define MATHPARSEAST_H

#include "MathParseTokenizer.h"

/**
 * Builds the abstract syntax tree of an expression using the MathParseTokenizer
 */
class MathParseAST : public MathParseTokenizer
{
public:
  MathParseAST(const std::string expression);

private:
  const std::string _mpa_expression;

  /// format a given error message with an ecpression excerpt and a visual pointer to the error position
  std::string formatError(std::size_t pos, const std::string & message, std::size_t width = 80);
};

#endif // MATHPARSEAST_H
