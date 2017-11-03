#ifndef SYMBOLICMATHFUNCTIONCCODE_H
#define SYMBOLICMATHFUNCTIONCCODE_H

#include "SymbolicMathFunctionBase.h"

namespace SymbolicMath
{

/**
 * The Function class is the top level wrapper for a Node based expression tree.
 * It manages the active value providers and the just in time compilation.
 */
class Function : public FunctionBase
{
public:
  /// Construct form given node
  Function(const Node & root) : FunctionBase(root) {}

  /// tear down function (release JIT context)
  ~Function() override;

  /// Returns the derivative of the subtree at the node w.r.t. value provider id
  Function D(ValueProviderPtr vp) const { return Function(_root.D(*vp)); }

  /// Compile the expression tree for faster evaluation
  void compile();

  /// Evaluate the node (using JIT if available)
  Real value();

protected:
  /// invalidate the JIT compiled function
  void invalidateJIT() override {}

  /// JIT compiled function object
  JITFunctionPtr _jit_code;
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHFUNCTIONCCODE_H
