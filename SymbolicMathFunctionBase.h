#ifndef SYMBOLICMATHFUNCTIONBASE_H
#define SYMBOLICMATHFUNCTIONBASE_H

#include "SymbolicMathNode.h"

namespace SymbolicMath
{

using ValueProviderPtr = std::shared_ptr<ValueProvider>;

/**
 * The Function class is the top level wrapper for a Node based expression tree.
 * It manages the active value providers and the just in time compilation.
 */
class FunctionBase
{
public:
  /// Construct form given node
  Function(const Node & root) : _root(root) {}

  /// Returns the derivative of the subtree at the node w.r.t. value provider id
  Function D(ValueProviderPtr vp) const { return Function(_root.D(*vp)); }

  /// Simplify the subtree at the node in place
  void simplify() { _root.simplify(); }

  ///@{ subtree output
  std::string format() const { return _root.format(); }
  std::string formatTree(std::string indent) const { return _root.formatTree(); }
  ///@}

protected:
  /// root node of the exprssion tree managed by this function
  Node _root;
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHFUNCTION_H