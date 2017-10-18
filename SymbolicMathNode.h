#ifndef SYMBOLICMATHNODE_H
#define SYMBOLICMATHNODE_H

#include <vector>
#include <memory>
#include <array>
#include <stack>
#include <type_traits>

#include "SymbolicMathSymbols.h"
#include "SymbolicMathJITTypes.h"

namespace SymbolicMath
{

class NodeData;
class ValueProvider;

using NodeDataPtr = std::shared_ptr<NodeData>;

/**
 * The Node class is the building block for the SymbolicMath tree structures that
 * represent mathematical expressions. It is the non-polymorphous wrapper for the
 * polymorphous NodeData, which is stored as a shared pointer. This design allows
 * the operator overloading to construct expression trees in C++ (in addition to
 * trees constructed by the Parser object).
 */
class Node
{
public:
  /// Default constructor builds an empty node
  Node();

  /// Construct form given data object
  Node(NodeDataPtr data) : _data(data) {}

  /// Copy constructor (shallow copy!)
  Node(const Node & copy);

  /// Deep copy
  Node clone(const Node & move);

  ///@{ Short cut constructors
  Node(Real val);
  Node(UnaryOperatorType type, Node arg);
  Node(BinaryOperatorType type, Node arg0, Node arg1);
  Node(MultinaryOperatorType type, std::vector<Node> args);
  Node(UnaryFunctionType type, Node arg);
  Node(BinaryFunctionType type, Node arg0, Node arg1);
  Node(ConditionalType type, Node arg0, Node arg1, Node arg2);
  Node(IntegerPowerType, Node arg, int exponent);
  ///@}

  ///@{ Operators to construct expression trees
  Node operator+(Node r);
  Node operator-(Node r);
  Node operator*(Node r);
  Node operator/(Node r);
  Node operator<(Node r);
  Node operator<=(Node r);
  Node operator>(Node r);
  Node operator>=(Node r);
  Node operator==(Node r);
  Node operator!=(Node r);
  Node operator-();
  ///@}

  /// Bracket operator for child node access
  Node operator[](unsigned int i);
  /// Number of child nodes
  std::size_t size();

  /// pass through functions
  Real value() const;
  JITReturnValue jit(JITStateValue & state);

  ///@{ subtree output
  std::string format() const;
  std::string formatTree(std::string indent = "") const;
  ///@}

  ///@{ Query the nature of the node data
  bool is(Real) const;
  bool is(NumberType) const;
  bool is(UnaryOperatorType) const;
  bool is(BinaryOperatorType) const;
  bool is(MultinaryOperatorType) const;
  bool is(UnaryFunctionType) const;
  bool is(BinaryFunctionType) const;
  bool is(ConditionalType) const;
  ///@}

  /// Test if the node is valid (i.e. does not have an EmptyData data content)
  bool isValid() const;

  /// Returns the derivative of the subtree at the node w.r.t. value provider id
  Node D(const ValueProvider &) const;

  /// Simplify the subtree at the node in place
  void simplify();

  /// Returns the precedence of the node for output purposes (bracket placement)
  unsigned short precedence() const;

  // returns the maximum stack depth of the current subtree
  void stackDepth(std::pair<int, int> & current_max);

protected:
  /// shared pointer to the actual guts of the node
  NodeDataPtr _data;
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHNODE_H
