///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include "SMTransform.h"
#include "SMFunctionBase.h"

namespace SymbolicMath
{

void
Transform::pushNode(Node * node)
{
  _node_stack.push(_current_node);
  _current_node = node;
}

void
Transform::popNode()
{
  _current_node = _node_stack.top();
  _node_stack.pop();
}

void
Transform::apply()
{
  // apply self to root
  _fb._root.apply(*this);
}

void
Transform::set(Real val)
{
  _current_node->_data = std::make_shared<RealNumberData>(val);
}

void
Transform::set(UnaryOperatorType type, Node arg)
{
  _current_node->_data = std::make_shared<UnaryOperatorData>(type, arg);
}

void
Transform::set(BinaryOperatorType type, Node arg0, Node arg1)
{
  _current_node->_data = std::make_shared<BinaryOperatorData>(type, arg0, arg1);
}

void
Transform::set(MultinaryOperatorType type, std::vector<Node> args)
{
  _current_node->_data = std::make_shared<MultinaryOperatorData>(type, args);
}

void
Transform::set(UnaryFunctionType type, Node arg)
{
  _current_node->_data = std::make_shared<UnaryFunctionData>(type, arg);
}

void
Transform::set(BinaryFunctionType type, Node arg0, Node arg1)
{
  _current_node->_data = std::make_shared<BinaryFunctionData>(type, arg0, arg1);
}

void
Transform::set(ConditionalType type, Node arg0, Node arg1, Node arg2)
{
  _current_node->_data = std::make_shared<ConditionalData>(type, arg0, arg1, arg2);
}

void
Transform::set(IntegerPowerType, Node arg, int exponent)
{
  _current_node->_data = std::make_shared<IntegerPowerData>(arg, exponent);
}

} // namespace SymbolicMath
