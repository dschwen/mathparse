///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include "SMTransform.h"
#include "SMFunction.h"

namespace SymbolicMath
{

template <typename T>
void
Transform<T>::pushNode(Node * node)
{
  _node_stack.push(_current_node);
  _current_node = node;
}

template <typename T>
void
Transform<T>::popNode()
{
  _current_node = _node_stack.top();
  _node_stack.pop();
}

template <typename T>
void
Transform<T>::apply()
{
  // apply self to root
  _fb._root.apply(*this);
}

template <typename T>
void
Transform<T>::set(Real val)
{
  _current_node->_data = std::make_shared<RealNumberData<T>>(val);
}

template <typename T>
void
Transform<T>::set(UnaryOperatorType type, Node arg)
{
  _current_node->_data = std::make_shared<UnaryOperatorData<T>>(type, arg);
}

template <typename T>
void
Transform<T>::set(BinaryOperatorType type, Node arg0, Node arg1)
{
  _current_node->_data = std::make_shared<BinaryOperatorData<T>>(type, arg0, arg1);
}

template <typename T>
void
Transform<T>::set(MultinaryOperatorType type, std::vector<Node> args)
{
  _current_node->_data = std::make_shared<MultinaryOperatorData<T>>(type, args);
}

template <typename T>
void
Transform<T>::set(UnaryFunctionType type, Node arg)
{
  _current_node->_data = std::make_shared<UnaryFunctionData<T>>(type, arg);
}

template <typename T>
void
Transform<T>::set(BinaryFunctionType type, Node arg0, Node arg1)
{
  _current_node->_data = std::make_shared<BinaryFunctionData<T>>(type, arg0, arg1);
}

template <typename T>
void
Transform<T>::set(ConditionalType type, Node arg0, Node arg1, Node arg2)
{
  _current_node->_data = std::make_shared<ConditionalData<T>>(type, arg0, arg1, arg2);
}

template <typename T>
void
Transform<T>::set(IntegerPowerType, Node arg, int exponent)
{
  _current_node->_data = std::make_shared<IntegerPowerData<T>>(arg, exponent);
}

template class Transform<Real>;

} // namespace SymbolicMath
