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
  FunctionBase(const Node & root) : _root(root) {}

  /// Simplify the subtree at the node in place
  void simplify()
  {
    _root.simplify();
    invalidateJIT();
  }

  ///@{ subtree output
  std::string format() const { return _root.format(); }
  std::string formatTree(std::string indent) const { return _root.formatTree(); }
  ///@}

protected:
  /// invalidate the JIT compiled function
  virtual void invalidateJIT() = 0;

  /// root node of the exprssion tree managed by this function
  Node _root;
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHFUNCTION_H
