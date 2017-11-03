#ifndef SYMBOLICMATHFUNCTIONLIGHTNING_H
#define SYMBOLICMATHFUNCTIONLIGHTNING_H

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
  ~Function();

  /// Returns the derivative of the subtree at the node w.r.t. value provider id
  Function D(ValueProviderPtr vp) const { return Function(_root.D(*vp)); }

  /// Compile the expression tree for faster evaluation
  void compile() override;

protected:
  /// invalidate the JIT compiled function
  void invalidateJIT() override;

  /// Lightning compiler state
  JITStateValue _state;
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHFUNCTIONLIGHTNING_H
