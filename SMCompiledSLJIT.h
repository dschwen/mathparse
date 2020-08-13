///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include "SMTransform.h"
#include "SMEvaluable.h"

#include <list>

namespace SymbolicMath
{

/**
 * SLJIT compiler transform
 */
template <typename T>
class CompiledSLJITTempl : public Transform, public Evaluable<T>
{
public:
  CompiledSLJITTempl(FunctionBase &);
  ~CompiledSLJITTempl() override;

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
  void stackPush();
  void stackPop(sljit_s32);

  void unaryFunctionCall(T (*func)(T));
  void binaryFunctionCall(T (*func)(T, T));

  void emitFcmp(sljit_s32);

  static T truncWrapper(T);

  /// current stack entry (as array index)
  int _sp;

  /// SLJIT compiler context
  struct sljit_compiler * _ctx;

  /// store immediates in a "pointer stable" way
  std::list<T> _immediate;

  /// compiled function
  using JITFunctionPtr = T SLJIT_FUNC (*)();
  JITFunctionPtr _jit_function;
};

using CompiledSLJIT = CompiledSLJITTempl<Real>;

} // namespace SymbolicMath
