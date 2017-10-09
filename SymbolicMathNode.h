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

using NodeDataPtr = std::shared_ptr<NodeData>;

class Node
{
public:
  // default constructor builds an empty node
  Node();

  // construct form given data object
  Node(NodeDataPtr data) : _data(data) {}

  // short cuts
  Node(Real val);
  Node(UnaryOperatorType type, Node arg);
  Node(BinaryOperatorType type, Node arg0, Node arg1);
  Node(MultinaryOperatorType type, std::vector<Node> args);
  Node(UnaryFunctionType type, Node arg);
  Node(BinaryFunctionType type, Node arg0, Node arg1);
  Node(ConditionalType type, Node arg0, Node arg1, Node arg2);

  /// copy constructor
  Node(const Node & copy);

  Node operator+(Node r);
  Node operator-(Node r);
  Node operator*(Node r);
  Node operator/(Node r);

  Node operator[](unsigned int i);
  std::size_t size();

  // pass through functions
  Real value() const;

  std::string format() const;
  std::string formatTree(std::string indent = "") const;

  bool is(Real) const;
  bool is(NumberType) const;
  bool is(UnaryOperatorType) const;
  bool is(BinaryOperatorType) const;
  bool is(MultinaryOperatorType) const;
  bool is(UnaryFunctionType) const;
  bool is(BinaryFunctionType) const;
  bool is(ConditionalType) const;

  bool isValid() const;

  Node D(unsigned int id) const;
  void simplify();

  unsigned short precedence() const;

protected:
  NodeDataPtr _data;
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHNODE_H
