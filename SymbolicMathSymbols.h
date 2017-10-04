#ifndef SYMBOLICMATH_SYMBOLS_H
#define SYMBOLICMATH_SYMBOLS_H

#include <string>
#include <vector>
#include <type_traits>

namespace SymbolicMath
{

struct OperatorProperties
{
  const unsigned short _precedence;
  const bool _left_associative;
  const std::string _form;
};

enum class NumberNodeType
{
  REAL,
  INTEGER,
  RATIONAL,
  _ANY,
  _INVALID
};

enum class UnaryOperatorNodeType
{
  PLUS,
  MINUS,
  FACULTY,
  NOT,
  _ANY,
  _INVALID
};

const std::map<UnaryOperatorNodeType, OperatorProperties> _unary_operators = {
    {UnaryOperatorNodeType::PLUS, {3, false, "+"}},
    {UnaryOperatorNodeType::MINUS, {3, false, "-"}},
    {UnaryOperatorNodeType::FACULTY, {3, false, "!"}},
    {UnaryOperatorNodeType::NOT, {3, false, "~"}}};

enum class BinaryOperatorNodeType
{
  SUBTRACTION,
  DIVISION,
  MODULO,
  POWER,
  LOGICAL_OR,
  LOGICAL_AND,
  LESS_THAN,
  GREATER_THAN,
  LESS_EQUAL,
  GREATER_EQUAL,
  EQUAL,
  NOT_EQUAL,
  _ANY,
  _INVALID
};

const std::map<UnaryOperatorNodeType, OperatorProperties> _binary_operators = {
    {BinaryOperatorNodeType::SUBTRACTION, {6, true, "-"}},
    {BinaryOperatorNodeType::DIVISION, {5, true, "/"}},
    {BinaryOperatorNodeType::POWER, {4, true, "^"}},
    {BinaryOperatorNodeType::LOGICAL_OR, {14, true, "|"}},
    {BinaryOperatorNodeType::LOGICAL_AND, {13, true, "&"}},
    {BinaryOperatorNodeType::LESS_THAN, {8, true, "<"}},
    {BinaryOperatorNodeType::GREATER_THAN, {8, true, ">"}},
    {BinaryOperatorNodeType::LESS_EQUAL, {8, true, "<="}},
    {BinaryOperatorNodeType::GREATER_EQUAL, {8, true, ">="}},
    {BinaryOperatorNodeType::EQUAL, {9, true, "=="}},
    {BinaryOperatorNodeType::NOT_EQUAL, {9, true, "!="}}};

enum class MultinaryOperatorNodeType
{
  ADDITION,
  MULTIPLICATION,
  COMPONENT,
  _ANY,
  _INVALID
};

const std::map<MultinaryOperatorNodeType, OperatorProperties> _multinary_operators = {
    {MultinaryOperatorNodeType::ADDITION, {6, true, "+"}},
    {MultinaryOperatorNodeType::MULTIPLICATION, {5, true, "*"}}};

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
  _ANY,
  _INVALID
};

const std::map<UnaryFunctionNodeType, std::string> _unary_functions = {
    {UnaryFunctionNodeType::ABS, "abs"},     {UnaryFunctionNodeType::ACOS, "acos"},
    {UnaryFunctionNodeType::ACOSH, "acosh"}, {UnaryFunctionNodeType::ARG, "arg"},
    {UnaryFunctionNodeType::ASIN, "asin"},   {UnaryFunctionNodeType::ASINH, "asinh"},
    {UnaryFunctionNodeType::ATAN, "atan"},   {UnaryFunctionNodeType::ATANH, "atanh"},
    {UnaryFunctionNodeType::CBRT, "cbrt"},   {UnaryFunctionNodeType::CEIL, "ceil"},
    {UnaryFunctionNodeType::CONJ, "conj"},   {UnaryFunctionNodeType::COS, "cos"},
    {UnaryFunctionNodeType::COSH, "cosh"},   {UnaryFunctionNodeType::COT, "cot"},
    {UnaryFunctionNodeType::CSC, "csc"},     {UnaryFunctionNodeType::EXP, "exp"},
    {UnaryFunctionNodeType::EXP2, "exp2"},   {UnaryFunctionNodeType::FLOOR, "floor"},
    {UnaryFunctionNodeType::IMAG, "imag"},   {UnaryFunctionNodeType::INT, "int"},
    {UnaryFunctionNodeType::LOG, "log"},     {UnaryFunctionNodeType::LOG10, "log10"},
    {UnaryFunctionNodeType::LOG2, "log2"},   {UnaryFunctionNodeType::REAL, "real"},
    {UnaryFunctionNodeType::SEC, "sec"},     {UnaryFunctionNodeType::SIN, "sin"},
    {UnaryFunctionNodeType::SINH, "sinh"},   {UnaryFunctionNodeType::SQRT, "sqrt"},
    {UnaryFunctionNodeType::T, "T"},         {UnaryFunctionNodeType::TAN, "tan"},
    {UnaryFunctionNodeType::TANH, "tanh"},   {UnaryFunctionNodeType::TRUNC, "trunc"}};

enum class BinaryFunctionNodeType
{
  ATAN2,
  HYPOT,
  MAX,
  MIN,
  PLOG,
  POLAR,
  POW,
  _ANY,
  _INVALID
};

const std::map<BinaryFunctionNodeType, std::string> _binary_functions = {
    {BinaryFunctionNodeType::ATAN2, "atan2"},
    {BinaryFunctionNodeType::HYPOT, "hypot"},
    {BinaryFunctionNodeType::MAX, "max"},
    {BinaryFunctionNodeType::MIN, "min"},
    {BinaryFunctionNodeType::PLOG, "plog"},
    {BinaryFunctionNodeType::POLAR, "polar"},
    {BinaryFunctionNodeType::POW, "pow"}};

enum class ConditionalNodeType
{
  IF,
  _ANY,
  _INVALID
};

enum class BracketType
{
  ROUND,
  SQUARE,
  CURLY,
  _INVALID
};

FunctionType identifyFunction(const std::string & op);

inline const OperatorProperties &
operatorProperty(UnaryOperatorNodeType op)
{
  auto it = _unary_operators.find(op);
  if (it == _unary_operators.end())
    fatalError("Unknown operator");
  return it->second;
}

inline const OperatorProperties &
operatorProperty(BinaryOperatorNodeType op)
{
  auto it = _binary_operators.find(op);
  if (it == _binary_operators.end())
    fatalError("Unknown operator");
  return it->second;
}

// end namespace SymbolicMath
}

#endif // SYMBOLICMATH_SYMBOLS_H
