#ifndef SYMBOLICMATHFUNCTIONS_H
#define SYMBOLICMATHFUNCTIONS_H

#include <string>
#include <vector>
#include <type_traits>

class SymbolicMathFunctions
{
protected:
  enum FunctionType
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

  static const std::vector<FunctionProperties> _functions;

  FunctionType identifyFunction(const std::string & op);
  const FunctionProperties & functionProperty(FunctionType op)
  {
    return _functions[static_cast<int>(op)];
  }
};

#endif // SYMBOLICMATHFUNCTIONS_H
