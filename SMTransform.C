///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include "SMTransform.h"
#include "SMFunction.h"

namespace SymbolicMath
{

template <typename T>
Transform<T>::Transform(Function<T> & fb) : _root(fb._root)
{
}

template <typename T>
Transform<T>::Transform(Node<T> & root) : _root(root)
{
}

template <typename T>
void
Transform<T>::apply()
{
  // apply self to root
  _root.apply(*this);
}

template <typename T>
void
Transform<T>::set(Node<T> & node, Real val)
{
  node._data = std::make_shared<RealNumberData<T>>(val);
}

template <typename T>
void
Transform<T>::set(Node<T> & node, UnaryOperatorType type, Node<T> arg)
{
  node._data = std::make_shared<UnaryOperatorData<T>>(type, arg);
}

template <typename T>
void
Transform<T>::set(Node<T> & node, BinaryOperatorType type, Node<T> arg0, Node<T> arg1)
{
  node._data = std::make_shared<BinaryOperatorData<T>>(type, arg0, arg1);
}

template <typename T>
void
Transform<T>::set(Node<T> & node, MultinaryOperatorType type, Node<T> arg0, Node<T> arg1)
{
  set(node, type, std::vector<Node<T>>{arg0, arg1});
}

template <typename T>
void
Transform<T>::set(Node<T> & node, MultinaryOperatorType type, std::vector<Node<T>> args)
{
  node._data = std::make_shared<MultinaryOperatorData<T>>(type, args);
}

template <typename T>
void
Transform<T>::set(Node<T> & node, UnaryFunctionType type, Node<T> arg)
{
  node._data = std::make_shared<UnaryFunctionData<T>>(type, arg);
}

template <typename T>
void
Transform<T>::set(Node<T> & node, BinaryFunctionType type, Node<T> arg0, Node<T> arg1)
{
  node._data = std::make_shared<BinaryFunctionData<T>>(type, arg0, arg1);
}

template <typename T>
void
Transform<T>::set(Node<T> & node, ConditionalType type, Node<T> arg0, Node<T> arg1, Node<T> arg2)
{
  node._data = std::make_shared<ConditionalData<T>>(type, arg0, arg1, arg2);
}

template <typename T>
void
Transform<T>::set(Node<T> & node, IntegerPowerType, Node<T> arg, int exponent)
{
  node._data = std::make_shared<IntegerPowerData<T>>(arg, exponent);
}

template class Transform<Real>;

} // namespace SymbolicMath
