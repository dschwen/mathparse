///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include "SMTransform.h"

namespace SymbolicMath
{

/**
 * Hashing visitor
 */
template <typename T>
class Hash : public Transform<T>
{
  using Transform<T>::set;
  using Transform<T>::apply;

public:
  template <class U>
  Hash(U & u) : Transform<T>(u)
  {
    apply();
  }

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

protected:
  void setHash(Node<T> &, std::size_t);

  std::size_t _hash;
  std::multimap<std::size_t, Node<T> *> _hash_map;
};

} // namespace SymbolicMath
