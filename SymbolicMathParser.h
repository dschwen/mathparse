#ifndef SYMBOLICMATHAST_H
#define SYMBOLICMATHAST_H

#include "SymbolicMathTokenizer.h"

/**
 * Builds the abstract syntax tree of an expression using the SymbolicMathTokenizer
 */
class SymbolicMathParser : public SymbolicMathTokenizer
{
public:
  SymbolicMathParser(const std::string expression);

private:
  const std::string _mpa_expression;

  /// format a token for debugging purposes
  std::string formatToken(const Token & token);

  /// format a given error message with an ecpression excerpt and a visual pointer to the error position
  std::string formatError(std::size_t pos, const std::string & message, std::size_t width = 80);
};

#endif // SYMBOLICMATHAST_H
