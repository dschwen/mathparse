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

  void operator()(Node<T> &,SymbolData<T> &) override;

  void operator()(Node<T> &,UnaryOperatorData<T> &) override;
  void operator()(Node<T> &,BinaryOperatorData<T> &) override;
  void operator()(Node<T> &,MultinaryOperatorData<T> &) override;

  void operator()(Node<T> &,UnaryFunctionData<T> &) override;
  void operator()(Node<T> &,BinaryFunctionData<T> &) override;

  void operator()(Node<T> &,RealNumberData<T> &) override;
  void operator()(Node<T> &,RealReferenceData<T> &) override;
  void operator()(Node<T> &,RealArrayReferenceData<T> &) override;
  void operator()(Node<T> &,LocalVariableData<T> &) override;

  void operator()(Node<T> &,ConditionalData<T> &) override;
  void operator()(Node<T> &,IntegerPowerData<T> &) override;

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
