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
  using Transform<T>::set;
  using Transform<T>::apply;

public:
  template <class U>
  Simplify(U & u) : Transform<T>(u)
  {
    apply();
  }

  void operator()(Node<T> &, SymbolData<T> &) override {}

  void operator()(Node<T> &, UnaryOperatorData<T> &) override;
  void operator()(Node<T> &, BinaryOperatorData<T> &) override;
  void operator()(Node<T> &, MultinaryOperatorData<T> &) override;

  void operator()(Node<T> &, UnaryFunctionData<T> &) override;
  void operator()(Node<T> &, BinaryFunctionData<T> &) override;

  void operator()(Node<T> &, RealNumberData<T> &) override {}
  void operator()(Node<T> &, RealReferenceData<T> &) override {}
  void operator()(Node<T> &, RealArrayReferenceData<T> &) override {}
  void operator()(Node<T> &, LocalVariableData<T> &) override {}

  void operator()(Node<T> &, ConditionalData<T> &) override;
  void operator()(Node<T> &, IntegerPowerData<T> &) override;
};

} // namespace SymbolicMath
