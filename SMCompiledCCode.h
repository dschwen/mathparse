///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include <stack>

#include "SMTransform.h"
#include "SMEvaluable.h"

namespace SymbolicMath
{

/**
 * C source code compilation, dynamic object laoding, evaluation
 */
template <typename T>
class CompiledCCode : public Evaluable<T>
{
public:
  class Source;

  CompiledCCode(Function<T> &);

  T operator()() override { return _jit_function(); }

protected:
  typedef Real (*JITFunctionPtr)();

  JITFunctionPtr _jit_function;
};

template <typename T>
class CompiledCCode<T>::Source : public Transform<T>
{
  using Transform<T>::apply;

public:
  Source(Function<T> &);

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

  const std::string & operator()() const { return _source; };

  const std::string typeName();
  const std::string typeHeader();

protected:
  std::string _source;
};

} // namespace SymbolicMath
