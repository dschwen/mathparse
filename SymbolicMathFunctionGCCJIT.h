#ifndef SYMBOLICMATHFUNCTIONGCCJIT_H
#define SYMBOLICMATHFUNCTIONGCCJIT_H

#include "SymbolicMathFunctionBase.h"
#include "SymbolicMathJITTypesGCCJIT.h"

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
  Function(const Node & root) : FunctionBase(root), _jit_closure(nullptr) { _state.ctxt = nullptr; }

  /// tear down function (release JIT context)
  ~Function();

  /// Returns the derivative of the subtree at the node w.r.t. value provider id
  Function D(ValueProviderPtr vp) const { return Function(_root.D(*vp)); }

  /// Compile the expression tree for faster evaluation
  void compile();

  /// check if the Function was successfully compiled
  bool isCompiled() { return _jit_closure; }

  /// Evaluate the node (using JIT if available)
  Real value();

protected:
  /// invalidate the JIT compiled function
  void invalidateJIT() override {}

  /// JIT compilation context data
  JITStateValue _state;

  /// JIT compilation object data
  gcc_jit_result * _jit_result;

  /// JIT compiled function object
  JITFunctionPtr _jit_closure;
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHFUNCTIONGCCJIT_H
