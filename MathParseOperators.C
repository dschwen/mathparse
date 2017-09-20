#include "MathParseOperators.h"

const std::vector<MathParseOperators::OperatorProperties> MathParseOperators::_operators = {
    {3, false, "+"},
    {3, false, "-"},
    {3, false, "!"},
    {3, false, "~"},
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

MathParseOperators::OperatorType
MathParseOperators::identifyOperator(const std::string & op)
{
  // we search from the back to avoid matching with the unary operators
  int i = static_cast<int>(OperatorType::INVALID) - 1;
  for (; i >= 0; --i)
    if (_operators[i]._form == op)
      return static_cast<OperatorType>(i);

  return OperatorType::INVALID;
}
