#ifndef SYMBOLICMATHFUNCTIONLIGHTNING_H
#define SYMBOLICMATHFUNCTIONLIGHTNING_H

#include "SymbolicMathFunctionBase.h"
#include "SymbolicMathJITTypesLightning.h"

namespace SymbolicMath
{

using ValueProviderPtr = std::shared_ptr<ValueProvider>;

/**
 * The Function class is the top level wrapper for a Node based expression tree.
 * It manages the active value providers and the just in time compilation.
 */
class Function : public FunctionBase
{
public:
  /// Construct form given node
  Function(const Node & root) : FunctionBase(root), _jit_code(nullptr) {}

  /// tear down function (release JIT context)
  ~Function();

  /// Returns the derivative of the subtree at the node w.r.t. value provider id
  Function D(ValueProviderPtr vp) const { return Function(_root.D(*vp)); }

  /// Compile the expression tree for faster evaluation
  void compile();

  /// check if the Function was successfully compiled
  bool isCompiled() { return _jit_code; }

  /// Evaluate the node (using JIT if available)
  Real value();

protected:
  /// invalidate the JIT compiled function
  void invalidateJIT() override;

  using JITFunction = double (*)(void);

  /// Lightning compiler state
  JITStateValue _state;

  /// executable JIT code
  JITFunction _jit_code;
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHFUNCTIONLIGHTNING_H
