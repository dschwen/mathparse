#ifndef SYMBOLICMATH_PARSER_H
#define SYMBOLICMATH_PARSER_H

#include "SymbolicMathTokenizer.h"
#include "SymbolicMathTree.h"

#include <string>
#include <stack>

namespace SymbolicMath
{

/**
 * Builds the abstract syntax tree of an expression using the SymbolicMathTokenizer
 */
class Parser
{
public:
  Parser();

  std::unique_ptr<Tree> parse(const std::string & expression);

protected:
  void pushToOutput(const Token & token);
  void pushFunctionToOutput(const Token & token, unsigned int num_arguments);

  void preprocessToken();
  void validateToken();

private:
  /// current token
  Token _token;

  /// previous token
  Token _last_token;

  /// output stack where the Tree is formed
  std::stack<std::unique_ptr<Tree>> _output_stack;

  /// currently parsed expression
  std::string _expression;

  /// format current token for debugging purposes
  std::string formatToken();

  /// format a given token for debugging purposes
  std::string formatToken(const Token & token);

  /// format a given error message with an expression excerpt and a visual pointer to the current token
  std::string formatError(const std::string & message, std::size_t width = 80);

  /// format a given error message with an expression excerpt and a visual pointer to the error position
  std::string formatError(std::size_t pos, const std::string & message, std::size_t width = 80);
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATH_PARSER_H
