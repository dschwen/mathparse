#ifndef SYMBOLICMATH_PARSER_H
#define SYMBOLICMATH_PARSER_H

#include "SymbolicMathTokenizer.h"
#include "SymbolicMathNode.h"

#include <string>
#include <stack>
#include <map>

namespace SymbolicMath
{

/**
 * Builds the abstract syntax tree of an expression using the SymbolicMathTokenizer
 */
class Parser
{
public:
  Parser();

  Node parse(const std::string & expression);

  unsigned int registerValueProvider(std::string name);

  void registerQPIndex(const unsigned int & qp) { _qp_ptr = &qp; }

  /*
  int registerValueProvider(std::string name, const VariableValue & var);
  int registerValueProvider(std::string name, const VariableGradient & grad);
  int registerValueProvider(std::string name, const MaterialProperty<Real> & real_prop);
  int registerValueProvider(std::string name, const MaterialProperty<RankTwoTensor> & r2t_prop);
  int registerValueProvider(std::string name, const MaterialProperty<RankThreeTensor> & r3t_prop);
  int registerValueProvider(std::string name, const MaterialProperty<RankFourTensor> & r4t_prop);

  or

  int registerValueProvider(std::string name, const ValueProvider & vp);
  (where ValueProvider is a base class that provides a virtual clone() method to build the tree
  nodes)
  */

protected:
  void pushToOutput(TokenPtr token);
  void pushFunctionToOutput(TokenPtr token, unsigned int num_arguments);

  void preprocessToken();
  void validateToken();

private:
  /// current token
  TokenPtr _token;

  /// previous token (no ownership, this token may be on the stack)
  TokenPtr _last_token;

  /// output stack where the Tree is formed
  std::stack<Node> _output_stack;

  /// value provider ID map
  std::map<std::string, unsigned int> _value_providers;

  /// pointer to the quadrature point index (_qp)
  const unsigned int * _qp_ptr;

  /// currently parsed expression
  std::string _expression;

  /// format current token for debugging purposes
  std::string formatToken();

  /// format a given token for debugging purposes
  std::string formatToken(TokenPtr token);

  /// format a given error message with an expression excerpt and a visual pointer to the current token
  std::string formatError(const std::string & message, std::size_t width = 80);

  /// format a given error message with an expression excerpt and a visual pointer to the error position
  std::string formatError(std::size_t pos, const std::string & message, std::size_t width = 80);
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATH_PARSER_H
