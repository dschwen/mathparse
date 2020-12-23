///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include <vector>
#include <memory>
#include <array>
#include <stack>
#include <type_traits>

#include "SMSymbols.h"
#include "SMNodeForward.h"

namespace SymbolicMath
{

template <typename T>
class Transform;
template <typename T>
class NodeData;
template <typename T>
class ValueProvider;

template <typename T>
using NodeDataPtr = std::shared_ptr<NodeData<T>>;

/**
 * The Node class is the building block for the SymbolicMath tree structures that
 * represent mathematical expressions. It is the non-polymorphous wrapper for the
 * polymorphous NodeData, which is stored as a shared pointer. This design allows
 * the operator overloading to construct expression trees in C++ (in addition to
 * trees constructed by the Parser object).
 */
template <typename T>
class Node
{
public:
  /// Default constructor builds an empty node
  Node() = delete;

  /// Construct form given data object
  Node(NodeDataPtr<T> data) : _data(data) {}

  /// Copy constructor (shallow copy!)
  Node(const Node<T> & copy);

  /// Deep copy
  Node<T> clone();

  ///@{ Short cut constructors
  Node(T val);
  static Node<T> fromReal(Real val);
  Node(UnaryOperatorType type, Node<T> arg);
  Node(BinaryOperatorType type, Node<T> arg0, Node<T> arg1);
  Node(MultinaryOperatorType type, Node<T> arg0, Node<T> arg1);
  Node(MultinaryOperatorType type, std::vector<Node> args);
  Node(UnaryFunctionType type, Node<T> arg);
  Node(BinaryFunctionType type, Node<T> arg0, Node<T> arg1);
  Node(ConditionalType type, Node<T> arg0, Node<T> arg1, Node<T> arg2);
  Node(IntegerPowerType, Node<T> arg, int exponent);
  ///@}

  ///@{ Operators to construct expression trees
  Node<T> operator-();
  Node<T> operator~();

  Node<T> operator+(const Node<T> & r);
  Node<T> operator-(const Node<T> & r);
  Node<T> operator*(const Node<T> & r);
  Node<T> operator/(const Node<T> & r);
  Node<T> operator%(const Node<T> & r);
  Node<T> operator<(const Node<T> & r);
  Node<T> operator<=(const Node<T> & r);
  Node<T> operator>(const Node<T> & r);
  Node<T> operator>=(const Node<T> & r);
  Node<T> operator==(const Node<T> & r);
  Node<T> operator!=(const Node<T> & r);
  Node<T> operator||(const Node<T> & r);
  Node<T> operator&&(const Node<T> & r);
  ///@}

  ///@{ Friend functions to construct expression trees
  friend Node<T> std::abs(const Node<T> &);
  friend Node<T> std::acos(const Node<T> &);
  friend Node<T> std::acosh(const Node<T> &);
  friend Node<T> std::asin(const Node<T> &);
  friend Node<T> std::asinh(const Node<T> &);
  friend Node<T> std::atan(const Node<T> &);
  friend Node<T> std::atanh(const Node<T> &);
  friend Node<T> std::cbrt(const Node<T> &);
  friend Node<T> std::ceil(const Node<T> &);
  friend Node<T> std::cos(const Node<T> &);
  friend Node<T> std::cosh(const Node<T> &);
  friend Node<T> std::erf(const Node<T> &);
  friend Node<T> std::erfc(const Node<T> &);
  friend Node<T> std::exp(const Node<T> &);
  friend Node<T> std::exp2(const Node<T> &);
  friend Node<T> std::floor(const Node<T> &);
  friend Node<T> std::log(const Node<T> &);
  friend Node<T> std::log10(const Node<T> &);
  friend Node<T> std::log2(const Node<T> &);
  friend Node<T> std::round(const Node<T> &);
  friend Node<T> std::sin(const Node<T> &);
  friend Node<T> std::sinh(const Node<T> &);
  friend Node<T> std::sqrt(const Node<T> &);
  friend Node<T> std::tan(const Node<T> &);
  friend Node<T> std::tanh(const Node<T> &);
  ///@}

  /// Bracket operator for child node access
  Node<T> operator[](unsigned int i);
  /// Number of child nodes
  std::size_t size() const;
  /// Simple hash for quick comparison
  std::size_t hash() const;

  /// pass through functions
  T value() const;

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
  bool is(IntegerPowerType) const;
  ///@}

  /// Test if the node is valid (i.e. does not have an EmptyData data content)
  bool isValid() const;

  /// Returns the derivative of the subtree at the node w.r.t. value provider id
  Node<T> D(const ValueProvider<T> &) const;

  /// Returns the precedence of the node for output purposes (bracket placement)
  unsigned short precedence() const;

  // returns the maximum stack depth of the current subtree
  void stackDepth(std::pair<int, int> & current_max) const;

  // apply a transform to the current subtree
  void apply(Transform<T> &);

  /// shared pointer to the actual guts of the node
  NodeDataPtr<T> _data;
};

} // namespace SymbolicMath
