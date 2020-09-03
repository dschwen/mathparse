///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include "SMTransform.h"
#include "SMEvaluable.h"

namespace SymbolicMath
{

/**
 * Stack based bytecode machine translation and evaluation
 */
template <typename T>
class CompiledByteCode : public Transform<T>, public Evaluable<T>
{
  using Transform<T>::apply;

public:
  CompiledByteCode(Function<T> &);

  void operator()(SymbolData<T> *) override;

  void operator()(UnaryOperatorData<T> *) override;
  void operator()(BinaryOperatorData<T> *) override;
  void operator()(MultinaryOperatorData<T> *) override;

  void operator()(UnaryFunctionData<T> *) override;
  void operator()(BinaryFunctionData<T> *) override;

  void operator()(RealNumberData<T> *) override;
  void operator()(RealReferenceData<T> *) override;
  void operator()(RealArrayReferenceData<T> *) override;
  void operator()(LocalVariableData<T> *) override;

  void operator()(ConditionalData<T> *) override;
  void operator()(IntegerPowerData<T> *) override;

  T operator()() override;

  void print();

protected:
  enum class VMInstruction : int
  {
    LOAD_IMMEDIATE_INTEGER = 0,
    LOAD_IMMEDIATE_REAL,
    LOAD_VARIABLE_REAL,

    UO_PLUS,
    UO_MINUS,
    UO_FACULTY,
    UO_NOT,

    BO_SUBTRACTION,
    BO_DIVISION,
    BO_MODULO,
    BO_POWER,
    BO_LOGICAL_OR,
    BO_LOGICAL_AND,
    BO_LESS_THAN,
    BO_GREATER_THAN,
    BO_LESS_EQUAL,
    BO_GREATER_EQUAL,
    BO_EQUAL,
    BO_NOT_EQUAL,
    BO_ASSIGNMENT,
    BO_LIST,

    MO_ADDITION,
    MO_MULTIPLICATION,
    MO_COMPONENT,
    MO_LIST,

    UF_ABS,
    UF_ACOS,
    UF_ACOSH,
    UF_ARG,
    UF_ASIN,
    UF_ASINH,
    UF_ATAN,
    UF_ATANH,
    UF_CBRT,
    UF_CEIL,
    UF_CONJ,
    UF_COS,
    UF_COSH,
    UF_COT,
    UF_CSC,
    UF_ERF,
    UF_ERFC,
    UF_EXP,
    UF_EXP2,
    UF_FLOOR,
    UF_IMAG,
    UF_INT,
    UF_LOG,
    UF_LOG10,
    UF_LOG2,
    UF_REAL,
    UF_SEC,
    UF_SIN,
    UF_SINH,
    UF_SQRT,
    UF_T,
    UF_TAN,
    UF_TANH,
    UF_TRUNC,

    BF_ATAN2,
    BF_HYPOT,
    BF_MAX,
    BF_MIN,
    BF_PLOG,
    BF_POLAR,
    BF_POW,

    CONDITIONAL,
    INTEGER_POWER,
    JUMP
  };

  /// byte code data
  std::vector<int> _byte_code;

  /// execution stack (not thread safe)
  std::vector<T> _stack;

  /// immediates
  std::vector<T> _immed;

  /// variables
  std::vector<const T *> _vars;
};

} // namespace SymbolicMath
