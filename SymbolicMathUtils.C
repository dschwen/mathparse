#include "SymbolicMathUtils.h"

namespace SymbolicMath
{

std::string
stringify(NumberNodeType type)
{
  return stringifyHelper(type, {"real", "integer", "rational"});
}

std::string
stringify(UnaryOperatorNodeType type)
{
  return stringifyHelper(type, {"+", "-", "!", "^"});
}

std::string
stringify(BinaryOperatorNodeType type)
{
  return stringifyHelper(type, {"-", "/", "%", "^", "|", "&"});
}

std::string
stringify(MultinaryOperatorNodeType type)
{
  return stringifyHelper(type, {"+", "*", "[]"});
}

std::string
stringify(UnaryFunctionNodeType type)
{
  return stringifyHelper(type,
                         {"abs",  "acos",  "acosh", "arg",  "asin", "asinh", "atan", "atanh",
                          "cbrt", "ceil",  "conj",  "cos",  "cosh", "cot",   "csc",  "exp",
                          "exp2", "floor", "imag",  "int",  "log",  "log10", "log2", "real",
                          "sec",  "sin",   "sinh",  "sqrt", "T",    "tan",   "tanh", "trunc"});
}

std::string
stringify(BinaryFunctionNodeType type)
{
  return stringifyHelper(type, {"atan2", "hypot", "max", "min", "plog", "polar", "pow"});
}

std::string
stringify(ConditionalNodeType type)
{
  return stringifyHelper(type, {"if"});
}

// end namespace SymbolicMath
}
