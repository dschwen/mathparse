///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include "SMTokenizer.h"
#include "SMNode.h"
#include "SMFunction.h"

// ttodo: remove when introducing value provider manager
#include "SMNodeData.h"

#include <string>
#include <stack>
#include <map>

namespace SymbolicMath
{

/**
 * Builds the abstract syntax tree of an expression string using the SymbolicMathTokenizer
 */
template <typename T>
class Parser
{
public:
  Parser();

  Function<T> parse(const std::string & expression);

  std::shared_ptr<ValueProvider<T>> registerValueProvider(std::string name);
  void registerValueProvider(std::shared_ptr<ValueProvider<T>> vp);
  void registerConstant(const std::string & name, T value);

  void registerQPIndex(const unsigned int & qp) { _qp_ptr = &qp; }

protected:
  void pushToOutput(TokenPtr<T> token);
  void pushFunctionToOutput(TokenPtr<T> token, unsigned int num_arguments);

  void preprocessToken();
  void validateToken();

private:
  /// current token
  TokenPtr<T> _token;

  /// previous token (no ownership, this token may be on the stack)
  TokenPtr<T> _last_token;

  /// output stack where the Tree is formed
  std::stack<Node<T>> _output_stack;

  /// constants map
  std::map<std::string, T> _constants;

  /// value provider ID map
  std::map<std::string, std::shared_ptr<ValueProvider<T>>> _value_providers;

  /// local variable ID map
  std::map<std::string, std::shared_ptr<LocalVariableData<T>>> _local_variables;

  /// pointer to the quadrature point index (_qp)
  const unsigned int * _qp_ptr;

  /// currently parsed expression
  std::string _expression;

  /// format current token for debugging purposes
  std::string formatToken();

  /// format a given token for debugging purposes
  std::string formatToken(TokenPtr<T> token);

  /// format a given error message with an expression excerpt and a visual pointer to the current token
  std::string formatError(const std::string & message, std::size_t width = 80);

  /// format a given error message with an expression excerpt and a visual pointer to the error position
  std::string formatError(std::size_t pos, const std::string & message, std::size_t width = 80);
};

} // namespace SymbolicMath
