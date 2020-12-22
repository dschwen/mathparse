///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include "SMTransform.h"
#include "SMEvaluable.h"

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
  using Transform<T>::apply;

public:
  CompiledLightning(Function<T> &);
  ~CompiledLightning() override;

  void operator()(Node<T> &, SymbolData<T> &) override;

  void operator()(Node<T> &, UnaryOperatorData<T> &) override;
  void operator()(Node<T> &, BinaryOperatorData<T> &) override;
  void operator()(Node<T> &, MultinaryOperatorData<T> &) override;

  void operator()(Node<T> &, UnaryFunctionData<T> &) override;
  void operator()(Node<T> &, BinaryFunctionData<T> &) override;

  void operator()(Node<T> &, RealNumberData<T> &) override;
  void operator()(Node<T> &, RealReferenceData<T> &) override;
  void operator()(Node<T> &, RealArrayReferenceData<T> &) override;
  void operator()(Node<T> &, LocalVariableData<T> &) override;

  void operator()(Node<T> &, ConditionalData<T> &) override;
  void operator()(Node<T> &, IntegerPowerData<T> &) override;

  T operator()() override { return _jit_function(); }

protected:
  void unaryFunctionCall(T (*func)(T));
  void binaryFunctionCall(T (*func)(T, T));

  void stackPush();
  void stackPop(int reg);

  static T wrapMin(T a, T b);
  static T wrapMax(T a, T b);
  static T plog(T a, T b);

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
