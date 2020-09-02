///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include "SMTransform.h"
#include "SMEvaluable.h"

#include <jit/jit.h>

#include <list>

namespace SymbolicMath
{

/**
 * LibJIT compiler transform
 */
template <typename T>
class CompiledLibJIT : public Transform<T>, public Evaluable<T>
{
  using Transform<T>::apply;

public:
  CompiledLibJIT(Function<T> &);
  ~CompiledLibJIT() override;

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

  T operator()() override { return _jit_function(); }

protected:
  jit_value_t unaryFunctionCall(T (*func)(T), jit_value_t);
  jit_value_t binaryFunctionCall(T (*func)(T, T), jit_value_t, jit_value_t);

  static T plog(T, T);

  /// JIT compilation context
  jit_context_t _jit_context;

  /// JIT return value
  jit_value_t _value;

  /// JIT state
  jit_function_t _state;

  /// store immediates in a "pointer stable" way
  std::list<T> _immediate;

  /// compiled function
  typedef Real (*JITFunctionPtr)();
  JITFunctionPtr _jit_function;
};

} // namespace SymbolicMath
