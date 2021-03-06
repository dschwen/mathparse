///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include "SMNode.h"
#include "SMTransform.h"
#include "SMNodeData.h"

#include <memory>

namespace SymbolicMath
{

template <typename T>
Node<T>::Node(const Node<T> & copy) : _data(copy._data)
{
}

template <typename T>
Node<T>
Node<T>::clone()
{
  return Node(_data->clone());
}

template <typename T>
Node<T>::Node(T val) : _data(std::make_shared<RealNumberData<T>>(val))
{
}

template <typename T>
Node<T>
Node<T>::fromReal(Real val)
{
  return Node<T>(std::make_shared<RealNumberData<T>>(val));
}

template <typename T>
Node<T>::Node(UnaryOperatorType type, Node arg)
  : _data(std::make_shared<UnaryOperatorData<T>>(type, arg))
{
}

template <typename T>
Node<T>::Node(BinaryOperatorType type, Node arg0, Node arg1)
  : _data(std::make_shared<BinaryOperatorData<T>>(type, arg0, arg1))
{
}

template <typename T>
Node<T>::Node(MultinaryOperatorType type, std::vector<Node> args)
  : _data(std::make_shared<MultinaryOperatorData<T>>(type, args))
{
}

template <typename T>
Node<T>::Node(UnaryFunctionType type, Node arg)
  : _data(std::make_shared<UnaryFunctionData<T>>(type, arg))
{
}

template <typename T>
Node<T>::Node(BinaryFunctionType type, Node arg0, Node arg1)
  : _data(std::make_shared<BinaryFunctionData<T>>(type, arg0, arg1))
{
}

template <typename T>
Node<T>::Node(ConditionalType type, Node arg0, Node arg1, Node arg2)
  : _data(std::make_shared<ConditionalData<T>>(type, arg0, arg1, arg2))
{
}

template <typename T>
Node<T>::Node(IntegerPowerType, Node arg, int exponent)
  : _data(std::make_shared<IntegerPowerData<T>>(arg, exponent))
{
}

template <typename T>
Node<T>
Node<T>::operator+(Node r)
{
  return Node(MultinaryOperatorType::ADDITION, {*this, r});
}

template <typename T>
Node<T>
Node<T>::operator-(Node r)
{
  return Node(BinaryOperatorType::SUBTRACTION, *this, r);
}

template <typename T>
Node<T> Node<T>::operator*(Node r)
{
  return Node(MultinaryOperatorType::MULTIPLICATION, {*this, r});
}

template <typename T>
Node<T>
Node<T>::operator/(Node r)
{
  return Node(BinaryOperatorType::DIVISION, *this, r);
}

template <typename T>
Node<T>
Node<T>::operator<(Node r)
{
  return Node(BinaryOperatorType::LESS_THAN, *this, r);
}

template <typename T>
Node<T>
Node<T>::operator<=(Node r)
{
  return Node(BinaryOperatorType::LESS_EQUAL, *this, r);
}

template <typename T>
Node<T>
Node<T>::operator>(Node r)
{
  return Node(BinaryOperatorType::GREATER_THAN, *this, r);
}

template <typename T>
Node<T>
Node<T>::operator>=(Node r)
{
  return Node(BinaryOperatorType::GREATER_EQUAL, *this, r);
}

template <typename T>
Node<T>
Node<T>::operator==(Node r)
{
  return Node(BinaryOperatorType::EQUAL, *this, r);
}

template <typename T>
Node<T>
Node<T>::operator!=(Node r)
{
  return Node(BinaryOperatorType::NOT_EQUAL, *this, r);
}

template <typename T>
Node<T>
Node<T>::operator-()
{
  return Node(UnaryOperatorType::MINUS, *this);
}

template <typename T>
Node<T> Node<T>::operator[](unsigned int i)
{
  return _data->getArg(i);
}

template <typename T>
std::size_t
Node<T>::size() const
{
  return _data->size();
}

template <typename T>
std::size_t
Node<T>::hash() const
{
  return _data->hash();
}

template <typename T>
T
Node<T>::value() const
{
  return _data->value();
}

template <typename T>
std::string
Node<T>::format() const
{
  return _data->format();
}

template <typename T>
std::string
Node<T>::formatTree(std::string indent) const
{
  return _data->formatTree(indent);
}

template <typename T>
bool
Node<T>::is(Real t) const
{
  return _data->is(t);
}

template <typename T>
bool
Node<T>::is(NumberType t) const
{
  return _data->is(t);
}

template <typename T>
bool
Node<T>::is(UnaryOperatorType t) const
{
  return _data->is(t);
}

template <typename T>
bool
Node<T>::is(BinaryOperatorType t) const
{
  return _data->is(t);
}

template <typename T>
bool
Node<T>::is(MultinaryOperatorType t) const
{
  return _data->is(t);
}

template <typename T>
bool
Node<T>::is(UnaryFunctionType t) const
{
  return _data->is(t);
}

template <typename T>
bool
Node<T>::is(BinaryFunctionType t) const
{
  return _data->is(t);
}

template <typename T>
bool
Node<T>::is(ConditionalType t) const
{
  return _data->is(t);
}

template <typename T>
bool
Node<T>::is(IntegerPowerType t) const
{
  return _data->is(t);
}

template <typename T>
bool
Node<T>::isValid() const
{
  return _data->isValid();
}

template <typename T>
Node<T>
Node<T>::D(const ValueProvider<T> & vp) const
{
  return Node(_data->D(vp));
}

template <typename T>
unsigned short
Node<T>::precedence() const
{
  return _data->precedence();
}

template <typename T>
void
Node<T>::stackDepth(std::pair<int, int> & current_max) const
{
  _data->stackDepth(current_max);

  // update maximum encountered stack depth
  if (current_max.second < current_max.first)
    current_max.second = current_max.first;
}

template <typename T>
void
Node<T>::apply(Transform<T> & transform)
{
  _data->apply(*this, transform);
}

} // namespace SymbolicMath
