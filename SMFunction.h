///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include "SMNode.h"
#include "SMEvaluable.h"

namespace SymbolicMath
{

template <typename T>
class Transform;

template <typename T>
using ValueProviderPtr = std::shared_ptr<ValueProvider<T>>;

/**
 * The Function class is the top level wrapper for a Node based expression tree.
 * It manages the active value providers and the just in time compilation.
 */
template <typename T>
class Function : public Evaluable<T>
{
public:
  /// Construct form given node
  Function(const Node<T> & root) : _root(root) {}
  virtual ~Function() {}

  ///@{ subtree output
  std::string format() const { return _root.format(); }
  std::string formatTree() const { return _root.formatTree(); }
  ///@}

  /// Evaluate the node (using JIT if available)
  T operator()() { return _root.value(); }

  /// reference to the root node
  virtual const Node<T> & root() const { return _root; }

  using LocalVariables = std::vector<std::pair<T, bool>>;

protected:
  /// root node of the expression tree managed by this function
  Node<T> _root;

  /// data for storing local variables
  LocalVariables _local_variables;

  friend class Transform<T>;
};

} // namespace SymbolicMath
