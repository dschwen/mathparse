#ifndef SYMBOLICMATH_SYMBOLS_H
#define SYMBOLICMATH_SYMBOLS_H

#include <string>
#include <vector>
#include <type_traits>

namespace SymbolicMath
{

//// New symbols

enum class NumberNodeType
{
  REAL,
  INTEGER,
  RATIONAL,
  _ANY
};

enum class UnaryOperatorNodeType
{
  PLUS,
  MINUS,
  FACULTY,
  NOT,
  _ANY
};

enum class BinaryOperatorNodeType
{
  SUBTRACTION,
  DIVISION,
  MODULO,
  POWER,
  LOGICAL_OR,
  LOGICAL_AND,
  _ANY
};

enum class MultinaryOperatorNodeType
{
  ADDITION,
  MULTIPLICATION,
  COMPONENT,
  _ANY
};

enum class UnaryFunctionNodeType
{
  ABS,
  ACOS,
  ACOSH,
  ARG,
  ASIN,
  ASINH,
  ATAN,
  ATANH,
  CBRT,
  CEIL,
  CONJ,
  COS,
  COSH,
  COT,
  CSC,
  EXP,
  EXP2,
  FLOOR,
  IMAG,
  INT,
  LOG,
  LOG10,
  LOG2,
  REAL,
  SEC,
  SIN,
  SINH,
  SQRT,
  T,
  TAN,
  TANH,
  TRUNC,
  _ANY
};

enum class BinaryFunctionNodeType
{
  ATAN2,
  HYPOT,
  MAX,
  MIN,
  PLOG,
  POLAR,
  POW,
  _ANY
};

enum class ConditionalNodeType
{
  IF,
  _ANY
};

//// Old symbols

enum class TokenType
{
  OPERATOR,
  FUNCTION,
  OPENING_BRACKET,
  CLOSING_BRACKET,
  NUMBER,
  VARIABLE,
  COMMA,
  COMPONENT,
  INVALID,
  END
};

enum class BracketType
{
  ROUND,
  SQUARE,
  CURLY
};

enum class FunctionType
{
  ABS,
  ACOS,
  ACOSH,
  ARG,
  ASIN,
  ASINH,
  ATAN,
  ATAN2,
  ATANH,
  CBRT,
  CEIL,
  CONJ,
  COS,
  COSH,
  COT,
  CSC,
  EXP,
  EXP2,
  FLOOR,
  HYPOT,
  IF,
  IMAG,
  INT,
  LOG,
  LOG10,
  LOG2,
  MAX,
  MIN,
  PLOG,
  POLAR,
  POW,
  REAL,
  SEC,
  SIN,
  SINH,
  SQRT,
  T,
  TAN,
  TANH,
  TRUNC,

  INVALID
};

struct FunctionProperties
{
  const unsigned short _arguments;
  const std::string _form;
};

static const std::vector<FunctionProperties> _functions = {
    {1, "abs"},   {1, "acos"},  {1, "acosh"}, {1, "arg"},   {1, "asin"},  {1, "asinh"}, {1, "atan"},
    {2, "atan2"}, {1, "atanh"}, {1, "cbrt"},  {1, "ceil"},  {1, "conj"},  {1, "cos"},   {1, "cosh"},
    {1, "cot"},   {1, "csc"},   {1, "exp"},   {1, "exp2"},  {1, "floor"}, {2, "hypot"}, {3, "if"},
    {1, "imag"},  {1, "int"},   {1, "log"},   {1, "log10"}, {1, "log2"},  {2, "max"},   {2, "min"},
    {2, "plog"},  {2, "polar"}, {2, "pow"},   {1, "real"},  {1, "sec"},   {1, "sin"},   {1, "sinh"},
    {1, "sqrt"},  {1, "T"},     {1, "tan"},   {1, "tanh"},  {1, "trunc"}};

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
  const bool _unary;
  const std::string _form;
};

const std::vector<OperatorProperties> _operators = {{3, false, true, "+"},
                                                    {3, false, true, "-"},
                                                    {3, false, true, "!"},
                                                    {3, false, true, "~"},
                                                    {4, true, false, "^"},
                                                    {5, true, false, "*"},
                                                    {5, true, false, "/"},
                                                    {5, true, false, "%"},
                                                    {6, true, false, "+"},
                                                    {6, true, false, "-"},
                                                    {8, true, false, "<"},
                                                    {8, true, false, ">"},
                                                    {8, true, false, "<="},
                                                    {8, true, false, ">="},
                                                    {9, true, false, "=="},
                                                    {9, true, false, "!="},
                                                    {13, true, false, "&"},
                                                    {14, true, false, "|"}};

FunctionType identifyFunction(const std::string & op);

inline const FunctionProperties &
functionProperty(FunctionType op)
{
  return _functions[static_cast<int>(op)];
}

OperatorType identifyOperator(const std::string & op);

inline const OperatorProperties &
operatorProperty(OperatorType op)
{
  return _operators[static_cast<int>(op)];
}

// end namespace SymbolicMath
}

#endif // SYMBOLICMATH_SYMBOLS_H
