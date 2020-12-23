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
Node<T>::Node(MultinaryOperatorType type, Node arg0, Node arg1)
  : _data(std::make_shared<MultinaryOperatorData<T>>(type, std::vector<Node>{arg0, arg1}))
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

#define SM_UNARY_OPERATOR_IMPLEMENTATION(op, type)                                                 \
  template <typename T>                                                                            \
  Node<T> Node<T>::operator op()                                                                   \
  {                                                                                                \
    return Node(type, *this);                                                                      \
  }

SM_UNARY_OPERATOR_IMPLEMENTATION(-, UnaryOperatorType::MINUS)
SM_UNARY_OPERATOR_IMPLEMENTATION(~, UnaryOperatorType::NOT)

#define SM_BINARY_OPERATOR_IMPLEMENTATION(op, type)                                                \
  template <typename T>                                                                            \
  Node<T> Node<T>::operator op(Node r)                                                             \
  {                                                                                                \
    return Node(type, *this, r);                                                                   \
  }

SM_BINARY_OPERATOR_IMPLEMENTATION(+, MultinaryOperatorType::ADDITION)
SM_BINARY_OPERATOR_IMPLEMENTATION(-, BinaryOperatorType::SUBTRACTION)
SM_BINARY_OPERATOR_IMPLEMENTATION(*, MultinaryOperatorType::MULTIPLICATION)
SM_BINARY_OPERATOR_IMPLEMENTATION(/, BinaryOperatorType::DIVISION)
SM_BINARY_OPERATOR_IMPLEMENTATION(%, BinaryOperatorType::MODULO)
SM_BINARY_OPERATOR_IMPLEMENTATION(<, BinaryOperatorType::LESS_THAN)
SM_BINARY_OPERATOR_IMPLEMENTATION(<=, BinaryOperatorType::LESS_EQUAL)
SM_BINARY_OPERATOR_IMPLEMENTATION(>, BinaryOperatorType::GREATER_THAN)
SM_BINARY_OPERATOR_IMPLEMENTATION(>=, BinaryOperatorType::GREATER_EQUAL)
SM_BINARY_OPERATOR_IMPLEMENTATION(==, BinaryOperatorType::EQUAL)
SM_BINARY_OPERATOR_IMPLEMENTATION(!=, BinaryOperatorType::NOT_EQUAL)
SM_BINARY_OPERATOR_IMPLEMENTATION(||, BinaryOperatorType::LOGICAL_OR)
SM_BINARY_OPERATOR_IMPLEMENTATION(&&, BinaryOperatorType::LOGICAL_AND)

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
