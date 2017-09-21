#ifndef SYMBOLICMATH_PARSER_H
#define SYMBOLICMATH_PARSER_H

#include "SymbolicMathTokenizer.h"

namespace SymbolicMath
{

/**
 * Builds the abstract syntax tree of an expression using the SymbolicMathTokenizer
 */
class Parser : public Tokenizer
{
public:
  Parser(const std::string expression);

private:
  const std::string _mpa_expression;

  /// format a token for debugging purposes
  std::string formatToken(const Token & token);

  /// format a given error message with an ecpression excerpt and a visual pointer to the error position
  std::string formatError(std::size_t pos, const std::string & message, std::size_t width = 80);
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATH_PARSER_H
