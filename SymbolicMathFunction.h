#ifndef SYMBOLICMATHFUNCTION_H
#define SYMBOLICMATHFUNCTION_H

#include <jit/jit.h>

#include "SymbolicMathNode.h"

namespace SymbolicMath
{

using ValueProviderPtr = std::shared_ptr<ValueProvider>;
typedef Real (*JITFunctionPtr)();

/**
 * The Function class is the top level wrapper for a Node based expression tree.
 * It manages the active value providers and the just in time compilation.
 */
class Function
{
public:
  /// Construct form given node
  Function(const Node & root) : _root(root), _jit_context(nullptr), _jit_closure(nullptr) {}

  /// tear down function (release JIT context)
  ~Function();

  /// Returns the derivative of the subtree at the node w.r.t. value provider id
  Function D(ValueProviderPtr vp) const { return Function(_root.D(*vp)); }

  /// Simplify the subtree at the node in place
  void simplify() { _root.simplify(); }

  /// Compile the expression tree for faster evaluation
  void compile();

  /// Evaluate the node (using JIT if available)
  Real value();

  ///@{ subtree output
  std::string format() const { return _root.format(); }
  std::string formatTree(std::string indent) const { return _root.formatTree(); }
  ///@}

protected:
  /// root node of the exprssion tree managed by this function
  Node _root;

  /// JIT compilation context
  jit_context_t _jit_context;

  /// JIT compiled function object
  JITFunctionPtr _jit_closure;
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHFUNCTION_H
