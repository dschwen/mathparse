///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include "SMTransformHash.h"
#include "SMFunction.h"

#include <functional>

namespace SymbolicMath
{

template <typename T>
Hash<T>::Hash(Function<T> & fb) : Transform<T>(fb)
{
  apply();
}

template <typename T>
void
Hash<T>::operator()(Node<T> & node, SymbolData<T> & data)
{
  static const std::size_t salt = std::hash<const void *>{}(reinterpret_cast<const void *>(&salt));

  setHash(node, salt ^ std::hash<std::string>{}(data._name));
}

template <typename T>
void
Hash<T>::operator()(Node<T> & node, UnaryOperatorData<T> & data)
{
  static const std::size_t salt = std::hash<const void *>{}(reinterpret_cast<const void *>(&salt));

  data._args[0].apply(*this);
  setHash(node, salt ^ std::hash<UnaryOperatorType>{}(data._type) ^ (_hash << 1));
}

template <typename T>
void
Hash<T>::operator()(Node<T> & node, BinaryOperatorData<T> & data)
{
  static const std::size_t salt = std::hash<const void *>{}(reinterpret_cast<const void *>(&salt));

  data._args[0].apply(*this);
  const auto hashA = _hash << 1;
  data._args[1].apply(*this);
  const auto hashB = _hash << 2;
  setHash(node, salt ^ std::hash<BinaryOperatorType>{}(data._type) ^ hashA ^ hashB);
}

template <typename T>
void
Hash<T>::operator()(Node<T> & node, MultinaryOperatorData<T> & data)
{
  static const std::size_t salt = std::hash<const void *>{}(reinterpret_cast<const void *>(&salt));

  auto h = salt ^ std::hash<MultinaryOperatorType>{}(data._type);
  for (auto & arg : data._args)
  {
    arg.apply(*this);
    h = (h << 1) ^ _hash;
  }
  setHash(node, h);
}

template <>
void
Hash<Real>::operator()(Node<Real> & node, UnaryFunctionData<Real> & data)
{
  static const std::size_t salt = std::hash<const void *>{}(reinterpret_cast<const void *>(&salt));

  data._args[0].apply(*this);
  setHash(node, salt ^ std::hash<UnaryFunctionType>{}(data._type) ^ (_hash << 1));
}

template <typename T>
void
Hash<T>::operator()(Node<T> & node, BinaryFunctionData<T> & data)
{
  static const std::size_t salt = std::hash<const void *>{}(reinterpret_cast<const void *>(&salt));

  data._args[0].apply(*this);
  const auto hashA = _hash << 1;
  data._args[1].apply(*this);
  const auto hashB = _hash << 2;
  setHash(node, salt ^ std::hash<BinaryFunctionType>{}(data._type) ^ hashA ^ hashB);
}

template <>
void
Hash<Real>::operator()(Node<Real> & node, RealNumberData<Real> & data)
{
  static const std::size_t salt = std::hash<const void *>{}(reinterpret_cast<const void *>(&salt));

  setHash(node, salt ^ std::hash<Real>{}(data._value));
}

template <typename T>
void
Hash<T>::operator()(Node<T> & node, RealReferenceData<T> & data)
{
  setHash(node, std::hash<const void *>{}(&data._ref));
}

template <typename T>
void
Hash<T>::operator()(Node<T> & node, RealArrayReferenceData<T> & data)
{
  fatalError("Not implemented yet");
}

template <typename T>
void
Hash<T>::operator()(Node<T> & node, LocalVariableData<T> & data)
{
  fatalError("Not implemented");
}

template <typename T>
void
Hash<T>::operator()(Node<T> & node, ConditionalData<T> & data)
{
  static const std::size_t salt = std::hash<const void *>{}(reinterpret_cast<const void *>(&salt));

  data._args[0].apply(*this);
  const auto hashA = _hash << 1;
  data._args[1].apply(*this);
  const auto hashB = _hash << 2;
  data._args[2].apply(*this);
  const auto hashC = _hash << 3;
  setHash(node, salt ^ hashA ^ hashB ^ hashC);
}

template <typename T>
void
Hash<T>::operator()(Node<T> & node, IntegerPowerData<T> & data)
{
  static const std::size_t salt = std::hash<const void *>{}(reinterpret_cast<const void *>(&salt));

  data._arg.apply(*this);
  setHash(node, salt ^ _hash ^ (std::hash<int>{}(data._exponent) << 1));
}

template <typename T>
void
Hash<T>::setHash(Node<T> & node, std::size_t h)
{
  _hash = h;
  _hash_map.emplace(h, &node);
}

template class Hash<Real>;

} // namespace SymbolicMath
