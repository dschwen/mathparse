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

  CompiledCCode(Function &);

  Real operator()() override { return _jit_function(); }

protected:
  typedef Real (*JITFunctionPtr)();

  const std::string typeName();
  const std::string typeHeader();

  JITFunctionPtr _jit_function;
};

template <typename T>
class CompiledCCode<T>::Source : public Transform<T>
{
public:
  Source(Function &);

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

  const std::string & operator()() const { return _source; };

protected:
  std::string _source;
};

} // namespace SymbolicMath
