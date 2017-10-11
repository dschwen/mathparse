#ifndef SYMBOLICMATHFUNCTION_H
#define SYMBOLICMATHFUNCTION_H

#include <jit/jit.h>

#include "SymbolicMathNode.h"

namespace SymbolicMath
{

/**
 * The Function class is the top level wrapper for a Node based expression tree.
 * It manages the active value providers and the just in time compilation.
 */
class Function
{
public:
  /// Construct form given node
  Function(const Node & root) : _root(root) {}

  /// Returns the derivative of the subtree at the node w.r.t. value provider id
  Node D(unsigned int id) const { return _root.D(id); }

  /// Simplify the subtree at the node in place
  void simplify() { _root.simplify(); }

  /// Compile the expression tree for faster evaluation
  void compile();

  /// Evaluate the node (using JIT if available)
  Real value();

protected:
  /// root of the
  Node _root;
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHFUNCTION_H
