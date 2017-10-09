#ifndef SYMBOLICMATHNODE_H
#define SYMBOLICMATHNODE_H

#include <vector>
#include <memory>
#include <array>
#include <stack>
#include <type_traits>

#include "SymbolicMathSymbols.h"

namespace SymbolicMath
{

class NodeData;

class Node
{
public:
  Node(NodeData * data) : _data(data) {}

  // short cuts
  Node(Real val);

  /// copy constructor
  Node(const Node & copy);

  Node operator+(Node r);
  Node operator-(Node r);
  Node operator*(Node r);
  Node operator/(Node r);

  Node operator[](unsigned int i);

  // pass through functions
  Real value() const;

  std::string format() const;
  std::string formatTree() const;

  bool is(Real) const;
  bool is(NumberType) const;
  bool is(UnaryOperatorType) const;
  bool is(BinaryOperatorType) const;
  bool is(MultinaryOperatorType) const;
  bool is(UnaryFunctionType) const;
  bool is(BinaryFunctionType) const;
  bool is(ConditionalType) const;

  Node D(unsigned int id) const;
  void simplify();

  unsigned short precedence() const;

  // do we really need this
  std::shared_ptr<NodeData> data() { return _data; }

protected:
  std::shared_ptr<NodeData> _data;
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHNODE_H
