///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include "SMTransform.h"

namespace SymbolicMath
{

/**
 * Simplification visitor
 */
template <typename T>
class Simplify : public Transform<T>
{
  using Transform<T>::_current_node;
  using Transform<T>::set;
  using Transform<T>::apply;

public:
  Simplify(Function<T> & fb);

  void operator()(SymbolData<T> *) override {}

  void operator()(UnaryOperatorData<T> *) override;
  void operator()(BinaryOperatorData<T> *) override;
  void operator()(MultinaryOperatorData<T> *) override;

  void operator()(UnaryFunctionData<T> *) override;
  void operator()(BinaryFunctionData<T> *) override;

  void operator()(RealNumberData<T> *) override {}
  void operator()(RealReferenceData<T> *) override {}
  void operator()(RealArrayReferenceData<T> *) override {}
  void operator()(LocalVariableData<T> *) override {}

  void operator()(ConditionalData<T> *) override;
  void operator()(IntegerPowerData<T> *) override;
};

} // namespace SymbolicMath
