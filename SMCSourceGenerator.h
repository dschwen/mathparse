///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include "SMTransform.h"

namespace SymbolicMath
{

template <typename T>
class CSourceGenerator : public Transform<T>
{
  using Transform<T>::apply;

public:
  CSourceGenerator(Function<T> &);

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

  std::string operator()() const { return _prologue + "return " + _source; };

  const std::string typeName();

protected:
  std::string bracket(std::string sub, short sub_precedence, short precedence);

  std::string _prologue;
  std::string _source;

  std::vector<const T *> _vars;

  unsigned int _tmp_id;
};

} // namespace SymbolicMath
