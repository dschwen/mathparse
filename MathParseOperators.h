#ifndef MATHPARSEOPERATORS_H
#define MATHPARSEOPERATORS_H

#include <string>
#include <vector>
#include <type_traits>

class MathParseOperators
{
protected:
  enum OperatorType
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

    COMMA,

    INVALID
  };
  using OperatorIterator = std::underlying_type<OperatorType>::type;

  struct OperatorProperties
  {
    const unsigned short _precedence;
    const bool _left_associative;
    const std::string _form;
  };

  static const std::vector<OperatorProperties> _operators;

  OperatorType identifyOperator(const std::string & op);
};

#endif // MATHPARSEOPERATORS_H
