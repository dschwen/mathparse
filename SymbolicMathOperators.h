#ifndef SYMBOLICMATH_OPERATORS_H
#define SYMBOLICMATH_OPERATORS_H

#include <string>
#include <vector>

namespace SymbolicMath
{

enum class OperatorType
{
  UNARY_PLUS,
  UNARY_MINUS,
  FACULTY,
  LOGICAL_NOT,

  POWER,

  MULTIPLICATION,
  DIVISION,
  MODULO,

  ADDITION,
  SUBTRACTION,

  LESS_THAN,
  GREATER_THAN,
  LESS_EQUAL,
  GREATER_EQUAL,

  EQUAL,
  NOT_EQUAL,

  LOGICAL_AND,
  LOGICAL_OR,

  INVALID
};

struct OperatorProperties
{
  const unsigned short _precedence;
  const bool _left_associative;
  const std::string _form;
};

const std::vector<OperatorProperties> _operators = {{3, true, "+"},
                                                    {3, true, "-"},
                                                    {3, true, "!"},
                                                    {3, true, "~"},
                                                    {4, true, "^"},
                                                    {5, true, "*"},
                                                    {5, true, "/"},
                                                    {5, true, "%"},
                                                    {6, true, "+"},
                                                    {6, true, "-"},
                                                    {8, true, "<"},
                                                    {8, true, ">"},
                                                    {8, true, "<="},
                                                    {8, true, ">="},
                                                    {9, true, "=="},
                                                    {9, true, "!="},
                                                    {13, true, "&"},
                                                    {14, true, "|"}};

OperatorType identifyOperator(const std::string & op);

inline const OperatorProperties &
operatorProperty(OperatorType op)
{
  return _operators[static_cast<int>(op)];
}

// end namespace SymbolicMath
}

#endif // SYMBOLICMATH_OPERATORS_H
