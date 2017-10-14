#ifndef SYMBOLICMATHFUNCTIONLIBJIT_H
#define SYMBOLICMATHFUNCTIONLIBJIT_H

#ifdef SYMBOLICMATH_USE_LIBJIT

#include "SymbolicMathFunctionBase.h"
#include "SymbolicMathLibJITTypes.h"

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

#endif // SYMBOLICMATH_USE_LIBJIT

#endif // SYMBOLICMATHFUNCTION_H
