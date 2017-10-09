#include "SymbolicMathNode.h"
#include "SymbolicMathNodeData.h"
#include "SymbolicMathUtils.h"

#include <cmath>
#include <iostream> // debug

namespace SymbolicMath
{

Node::Node(const Node & copy) : _data(copy._data->clone()) {}

Node::Node(Real val) : _data(new RealNumberData(val)) {}

Node
Node::operator+(Node r)
{
  return Node(new MultinaryOperatorNode(MultinaryOperatorType::ADDITION, *this, r));
}

Node
Node::operator-(Node r)
{
  return Node(new BinaryOperatorNode(BinaryOperatorType::SUBTRACTION, *this, r));
}

Node Node::operator*(Node r)
{
  return Node(new MultinaryOperatorNode(MultinaryOperatorType::MULTIPLICATION, *this, r));
}

Node
Node::operator/(Node r)
{
  return Node(new BinaryOperatorNode(BinaryOperatorType::DIVISION, *this, r));
}

Node Node::operator[](unsigned int i) { return _data->getArg(i); }

Real
Node::value() const
{
  return _data->value();
}

std::string
Node::format() const
{
  return _data->format();
}

std::string
Node::formatTree() const
{
  return _data->formatTree();
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

Node
Node::D(unsigned int id) const
{
  return Node(_data->D(id));
}

void
Node::simplify()
{
  _data.reset(_data->simplify());
}

unsigned short
Node::precedence() const
{
  return _data->precedence();
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
