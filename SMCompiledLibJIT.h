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
public:
  CompiledLibJIT(Function &);
  ~CompiledLibJIT() override;

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

  T operator()() override { return _jit_function(); }

protected:
  jit_value_t unaryFunctionCall(T (*func)(T), jit_value_t);

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
