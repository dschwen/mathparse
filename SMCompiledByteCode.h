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
public:
  CompiledByteCode(Function &);

  void operator()(SymbolData *) override;

  void operator()(UnaryOperatorData *) override;
  void operator()(BinaryOperatorData *) override;
  void operator()(MultinaryOperatorData *) override;

  void operator()(UnaryFunctionData *) override;
  void operator()(BinaryFunctionData *) override;

  void operator()(RealNumberData *) override;
  void operator()(RealReferenceData *) override;
  void operator()(RealArrayReferenceData *) override;
  void operator()(LocalVariableData *) override;

  void operator()(ConditionalData *) override;
  void operator()(IntegerPowerData *) override;

  T operator()() override;

protected:
  enum class VMInstruction
  {
    LOAD_IMMEDIATE_INTEGER,
    LOAD_IMMEDIATE_REAL,
    LOAD_VARIABLE_REAL,
    UNARY_OPERATOR,
    BINARY_OPERATOR,
    MULTINARY_PLUS,
    MULTINARY_MULTIPLY,
    UNARY_FUNCTION,
    BINARY_FUNCTION,
    CONDITIONAL,
    INTEGER_POWER,
    JUMP
  };

  struct VMData
  {
    VMData(int value) : _int_value(value) {}
    VMData(T value) : _value(value) {}
    VMData(const T * variable) : _variable(variable) {}
    VMData(UnaryOperatorType unary_operator) : _unary_operator(unary_operator) {}
    VMData(BinaryOperatorType binary_operator) : _binary_operator(binary_operator) {}
    VMData(UnaryFunctionType unary_function) : _unary_function(unary_function) {}
    VMData(BinaryFunctionType binary_function) : _binary_function(binary_function) {}

    union {
      int _int_value;
      T _value;
      const T * _variable;
      UnaryOperatorType _unary_operator;
      BinaryOperatorType _binary_operator;
      UnaryFunctionType _unary_function;
      BinaryFunctionType _binary_function;
    };
  };

  using VMItem = std::pair<VMInstruction, VMData>;

  /// byte code data
  using ByteCode = std::vector<VMItem>;
  ByteCode _byte_code;

  /// execution stack (not thread safe)
  std::vector<T> _stack;
};

} // namespace SymbolicMath
