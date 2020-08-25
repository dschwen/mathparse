///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include "SMTransform.h"
#include "SMEvaluable.h"

#include <jit/jit.h>

#include <list>

// forward declaration
typedef struct jit_state jit_state_t;

namespace SymbolicMath
{

/**
 * LibJIT compiler transform
 */
template <typename T>
class CompiledLightning : public Transform<T>, public Evaluable<T>
{
public:
  CompiledLightning(Function &);
  ~CompiledLightning() override;

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
  void unaryFunctionCall(T (*func)(T));
  void binaryFunctionCall(T (*func)(T, T));

  void stackPush();
  void stackPop(int reg);

  static T wrapMin(T a, T b);
  static T wrapMax(T a, T b);

  /// current stack entry (as array index)
  int _sp;

  /// base offset of the stack from the frame pointer in bytes
  int _stack_base;

  /// lightning compiler state
  jit_state_t * _jit;

  /// store immediates in a "pointer stable" way
  std::list<T> _immediate;

  /// compiled function
  typedef T (*JITFunctionPtr)();
  JITFunctionPtr _jit_function;
};

} // namespace SymbolicMath
