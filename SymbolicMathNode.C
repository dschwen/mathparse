#include "SymbolicMathNode.h"
#include "SymbolicMathNodeData.h"
#include "SymbolicMathUtils.h"

#include <cmath>
#include <iostream> // debug

namespace SymbolicMath
{

Node::Node() : _data(std::make_shared<EmptyData>()) {}

Node::Node(const Node & copy) : _data(copy._data) {}

Node
Node::clone(const Node & move)
{
  return Node(_data->clone());
}

Node::Node(Real val) : _data(new RealNumberData(val)) {}

Node::Node(UnaryOperatorType type, Node arg) : _data(std::make_shared<UnaryOperatorData>(type, arg))
{
}

Node::Node(BinaryOperatorType type, Node arg0, Node arg1)
  : _data(std::make_shared<BinaryOperatorData>(type, arg0, arg1))
{
}

Node::Node(MultinaryOperatorType type, std::vector<Node> args)
  : _data(std::make_shared<MultinaryOperatorData>(type, args))
{
}

Node::Node(UnaryFunctionType type, Node arg) : _data(std::make_shared<UnaryFunctionData>(type, arg))
{
}

Node::Node(BinaryFunctionType type, Node arg0, Node arg1)
  : _data(std::make_shared<BinaryFunctionData>(type, arg0, arg1))
{
}

Node::Node(ConditionalType type, Node arg0, Node arg1, Node arg2)
  : _data(std::make_shared<ConditionalData>(type, arg0, arg1, arg2))
{
}

Node::Node(IntegerPowerType, Node arg, int exponent)
  : _data(std::make_shared<IntegerPowerData>(arg, exponent))
{
}

Node
Node::operator+(Node r)
{
  return Node(MultinaryOperatorType::ADDITION, {*this, r});
}

Node
Node::operator-(Node r)
{
  return Node(BinaryOperatorType::SUBTRACTION, *this, r);
}

Node Node::operator*(Node r) { return Node(MultinaryOperatorType::MULTIPLICATION, {*this, r}); }

Node
Node::operator/(Node r)
{
  return Node(BinaryOperatorType::DIVISION, *this, r);
}

Node
Node::operator<(Node r)
{
  return Node(BinaryOperatorType::LESS_THAN, *this, r);
}

Node
Node::operator<=(Node r)
{
  return Node(BinaryOperatorType::LESS_EQUAL, *this, r);
}

Node
Node::operator>(Node r)
{
  return Node(BinaryOperatorType::GREATER_THAN, *this, r);
}

Node
Node::operator>=(Node r)
{
  return Node(BinaryOperatorType::GREATER_EQUAL, *this, r);
}

Node
Node::operator==(Node r)
{
  return Node(BinaryOperatorType::EQUAL, *this, r);
}

Node
Node::operator!=(Node r)
{
  return Node(BinaryOperatorType::NOT_EQUAL, *this, r);
}

Node
Node::operator-()
{
  return Node(UnaryOperatorType::MINUS, *this);
}

Node Node::operator[](unsigned int i) { return _data->getArg(i); }

std::size_t
Node::size()
{
  return _data->size();
}

Real
Node::value() const
{
  return _data->value();
}

JITReturnValue
Node::jit(JITStateValue & state)
{
  return _data->jit(state);
}

std::string
Node::format() const
{
  return _data->format();
}

std::string
Node::formatTree(std::string indent) const
{
  return _data->formatTree(indent);
}

bool
Node::is(Real t) const
{
  return _data->is(t);
}

bool
Node::is(NumberType t) const
{
  return _data->is(t);
}

bool
Node::is(UnaryOperatorType t) const
{
  return _data->is(t);
}

bool
Node::is(BinaryOperatorType t) const
{
  return _data->is(t);
}

bool
Node::is(MultinaryOperatorType t) const
{
  return _data->is(t);
}

bool
Node::is(UnaryFunctionType t) const
{
  return _data->is(t);
}

bool
Node::is(BinaryFunctionType t) const
{
  return _data->is(t);
}

bool
Node::is(ConditionalType t) const
{
  return _data->is(t);
}

bool
Node::isValid() const
{
  return _data->isValid();
}

Node
Node::D(const ValueProvider & vp) const
{
  return Node(_data->D(vp));
}

void
Node::simplify()
{
  auto simplified = _data->simplify();
  if (simplified.isValid())
    _data = simplified._data;
}

unsigned short
Node::precedence() const
{
  return _data->precedence();
}

void
Node::stackDepth(std::pair<int, int> & current_max)
{
  _data->stackDepth(current_max);

  // update maximum encountered stack depth
  if (current_max.second < current_max.first)
    current_max.second = current_max.first;
}

// void
// Node::checkIndex(const std::vector<unsigned int> & index)
// {
//   auto s = shape();
//
//   if (index.size() > s.size())
//     fatalError("Index exceeds object dimensions");
//
//   for (std::size_t i = 0; i < index.size(); ++i)
//     if (index[i] >= s[i])
//       fatalError("Index out of range");
// }

// end namespace SymbolicMath
}
