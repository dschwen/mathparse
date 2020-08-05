///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include <string>
#include <vector>
#include <map>

#include "SymbolicMathUtils.h"

namespace SymbolicMath
{

using Shape = std::vector<unsigned int>;

struct OperatorProperties
{
  const unsigned short _precedence;
  const bool _left_associative;
  const std::string _form;
};

enum class NumberType
{
  REAL,
  INTEGER,
  RATIONAL,
  _ANY,
  _INVALID
};

enum class UnaryOperatorType
{
  PLUS,
  MINUS,
  FACULTY,
  NOT,
  _ANY,
  _INVALID
};

const std::map<UnaryOperatorType, OperatorProperties> _unary_operators = {
    {UnaryOperatorType::PLUS, {3, false, "+"}},
    {UnaryOperatorType::MINUS, {3, false, "-"}},
    {UnaryOperatorType::FACULTY, {3, false, "!"}},
    {UnaryOperatorType::NOT, {3, false, "~"}}};

enum class BinaryOperatorType
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
  ASSIGNMENT,
  LIST,
  _ANY,
  _INVALID
};

const std::map<BinaryOperatorType, OperatorProperties> _binary_operators = {
    {BinaryOperatorType::SUBTRACTION, {6, true, "-"}},
    {BinaryOperatorType::DIVISION, {5, true, "/"}},
    {BinaryOperatorType::MODULO, {5, true, "%"}},
    {BinaryOperatorType::POWER, {4, true, "^"}},
    {BinaryOperatorType::LOGICAL_OR, {14, true, "|"}},
    {BinaryOperatorType::LOGICAL_AND, {13, true, "&"}},
    {BinaryOperatorType::LESS_THAN, {8, true, "<"}},
    {BinaryOperatorType::GREATER_THAN, {8, true, ">"}},
    {BinaryOperatorType::LESS_EQUAL, {8, true, "<="}},
    {BinaryOperatorType::GREATER_EQUAL, {8, true, ">="}},
    {BinaryOperatorType::EQUAL, {9, true, "=="}},
    {BinaryOperatorType::NOT_EQUAL, {9, true, "!="}},
    {BinaryOperatorType::ASSIGNMENT, {9, true, ":="}},
    {BinaryOperatorType::LIST, {15, true, ";"}}};

enum class MultinaryOperatorType
{
  ADDITION,
  MULTIPLICATION,
  COMPONENT,
  LIST,
  _ANY,
  _INVALID
};

const std::map<MultinaryOperatorType, OperatorProperties> _multinary_operators = {
    {MultinaryOperatorType::ADDITION, {6, true, "+"}},
    {MultinaryOperatorType::MULTIPLICATION, {5, true, "*"}},
    {MultinaryOperatorType::LIST, {15, true, ";"}}};

enum class UnaryFunctionType
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
  ERF,
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

const std::map<UnaryFunctionType, std::string> _unary_functions = {
    {UnaryFunctionType::ABS, "abs"},     {UnaryFunctionType::ACOS, "acos"},
    {UnaryFunctionType::ACOSH, "acosh"}, {UnaryFunctionType::ARG, "arg"},
    {UnaryFunctionType::ASIN, "asin"},   {UnaryFunctionType::ASINH, "asinh"},
    {UnaryFunctionType::ATAN, "atan"},   {UnaryFunctionType::ATANH, "atanh"},
    {UnaryFunctionType::CBRT, "cbrt"},   {UnaryFunctionType::CEIL, "ceil"},
    {UnaryFunctionType::CONJ, "conj"},   {UnaryFunctionType::COS, "cos"},
    {UnaryFunctionType::COSH, "cosh"},   {UnaryFunctionType::COT, "cot"},
    {UnaryFunctionType::CSC, "csc"},     {UnaryFunctionType::ERF, "erf"},
    {UnaryFunctionType::EXP, "exp"},     {UnaryFunctionType::EXP2, "exp2"},
    {UnaryFunctionType::FLOOR, "floor"}, {UnaryFunctionType::IMAG, "imag"},
    {UnaryFunctionType::INT, "int"},     {UnaryFunctionType::LOG, "log"},
    {UnaryFunctionType::LOG10, "log10"}, {UnaryFunctionType::LOG2, "log2"},
    {UnaryFunctionType::REAL, "real"},   {UnaryFunctionType::SEC, "sec"},
    {UnaryFunctionType::SIN, "sin"},     {UnaryFunctionType::SINH, "sinh"},
    {UnaryFunctionType::SQRT, "sqrt"},   {UnaryFunctionType::T, "T"},
    {UnaryFunctionType::TAN, "tan"},     {UnaryFunctionType::TANH, "tanh"},
    {UnaryFunctionType::TRUNC, "trunc"}};

enum class BinaryFunctionType
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

const std::map<BinaryFunctionType, std::string> _binary_functions = {
    {BinaryFunctionType::ATAN2, "atan2"},
    {BinaryFunctionType::HYPOT, "hypot"},
    {BinaryFunctionType::MAX, "max"},
    {BinaryFunctionType::MIN, "min"},
    {BinaryFunctionType::PLOG, "plog"},
    {BinaryFunctionType::POLAR, "polar"},
    {BinaryFunctionType::POW, "pow"}};

enum class ConditionalType
{
  IF,
  _ANY,
  _INVALID
};

enum class IntegerPowerType
{
  _ANY
};

enum class BracketType
{
  ROUND,
  SQUARE,
  CURLY,
  _INVALID
};

inline const OperatorProperties &
operatorProperty(UnaryOperatorType op)
{
  auto it = _unary_operators.find(op);
  if (it == _unary_operators.end())
    fatalError("Unknown operator");
  return it->second;
}

inline const OperatorProperties &
operatorProperty(BinaryOperatorType op)
{
  auto it = _binary_operators.find(op);
  if (it == _binary_operators.end())
    fatalError("Unknown operator");
  return it->second;
}

std::string stringify(UnaryOperatorType type);
std::string stringify(BinaryOperatorType type);
std::string stringify(MultinaryOperatorType type);
std::string stringify(UnaryFunctionType type);
std::string stringify(BinaryFunctionType type);
std::string stringify(ConditionalType type);

} // namespace SymbolicMath
