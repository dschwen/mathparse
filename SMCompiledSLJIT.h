///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include "SMTransform.h"
#include "SMEvaluable.h"

#include "contrib/sljit_src/sljitLir.h"

#include <list>

namespace SymbolicMath
{

/**
 * SLJIT compiler transform
 */
template <typename T>
class CompiledSLJIT : public Transform<T>, public Evaluable<T>
{
  using Transform<T>::apply;

public:
  CompiledSLJIT(Function<T> &);
  ~CompiledSLJIT() override;

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
  void stackPush();
  void stackPop(sljit_s32);

  void unaryFunctionCall(T (*func)(T));
  void binaryFunctionCall(T (*func)(T, T));

  void emitFcmp(sljit_s32);

  static T truncWrapper(T);
  static T plog(T, T);

  /// current stack entry (as array index)
  int _sp;

  /// SLJIT compiler context
  struct sljit_compiler * _ctx;

  /// store immediates in a "pointer stable" way
  std::list<T> _immediate;

  /// compiled function (TODO: pass result by reference)
  using JITFunctionPtr = T SLJIT_FUNC (*)();
  JITFunctionPtr _jit_function;
};

} // namespace SymbolicMath
