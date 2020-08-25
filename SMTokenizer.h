///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include "SMToken.h"

#include <string>

namespace SymbolicMath
{
typedef double Real;

/**
 * Tokenizer that parses an expression string
 */
template <typename T>
class Tokenizer
{
public:
  Tokenizer(const std::string expression);

  /// gets the next complete token from the expression
  TokenPtr<T> getToken();

private:
  const std::string _mpt_expression;
  std::string::const_iterator _c;
  std::size_t _token_start;

  ///@{ classification functions for the next expression character
  bool isDigit();
  bool isOperator();
  bool isBracket();
  bool isAlphaFirst();
  bool isAlphaCont();
  ///@}

  /// fetch and assemble a sequence of characters that constitute a valid integer number
  int getInteger();

  /// skip the next whitespace characters
  void skipWhite();

  /// curent tokenizer position
  std::size_t pos() { return std::distance(_mpt_expression.begin(), _c); }
};

} // namespace SymbolicMath
