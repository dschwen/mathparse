#include "SymbolicMathFunctions.h"

const std::vector<SymbolicMathFunctions::FunctionProperties> SymbolicMathFunctions::_functions = {
    {1, "abs"},   {1, "acos"},  {1, "acosh"}, {1, "arg"},   {1, "asin"},  {1, "asinh"}, {1, "atan"},
    {2, "atan2"}, {1, "atanh"}, {1, "cbrt"},  {1, "ceil"},  {1, "conj"},  {1, "cos"},   {1, "cosh"},
    {1, "cot"},   {1, "csc"},   {1, "exp"},   {1, "exp2"},  {1, "floor"}, {2, "hypot"}, {3, "if"},
    {1, "imag"},  {1, "int"},   {1, "log"},   {1, "log10"}, {1, "log2"},  {2, "max"},   {2, "min"},
    {2, "plog"},  {2, "polar"}, {2, "pow"},   {1, "real"},  {1, "sec"},   {1, "sin"},   {1, "sinh"},
    {1, "sqrt"},  {1, "tan"},   {1, "tanh"},  {1, "trunc"}};

SymbolicMathFunctions::FunctionType
SymbolicMathFunctions::identifyFunction(const std::string & func)
{
  std::size_t i = 0;
  const auto last = static_cast<std::size_t>(FunctionType::INVALID);
  for (; i < last; ++i)
    if (_functions[i]._form == func)
      break;

  return static_cast<FunctionType>(i);
}
