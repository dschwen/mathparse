#ifndef SYMBOLICMATHFUNCTIONLIBJIT_H
#define SYMBOLICMATHFUNCTIONLIBJIT_H

#include "SymbolicMathFunctionBase.h"
#include "SymbolicMathJITTypesLibJIT.h"

namespace SymbolicMath
{

using ValueProviderPtr = std::shared_ptr<ValueProvider>;
typedef Real (*JITFunctionPtr)();

/**
 * The Function class is the top level wrapper for a Node based expression tree.
 * It manages the active value providers and the just in time compilation.
 */
class Function : public FunctionBase
{
public:
  /// Construct form given node
  Function(const Node & root) : FunctionBase(root), _jit_context(nullptr), _jit_closure(nullptr) {}

  /// tear down function (release JIT context)
  ~Function();

  /// Returns the derivative of the subtree at the node w.r.t. value provider id
  Function D(ValueProviderPtr vp) const { return Function(_root.D(*vp)); }

  /// Compile the expression tree for faster evaluation
  void compile();

  /// Evaluate the node (using JIT if available)
  Real value();

protected:
  /// JIT compilation context
  jit_context_t _jit_context;

  /// JIT compiled function object
  JITFunctionPtr _jit_closure;
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHFUNCTIONLIBJIT_H
