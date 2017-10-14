#ifndef SYMBOLICMATHFUNCTIONSLJIT_H
#define SYMBOLICMATHFUNCTIONSLJIT_H

#ifdef SYMBOLICMATH_USE_SLJIT

#include "SymbolicMathFunctionBase.h"
#include "SymbolicMathSLJITTypes.h"

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
  Function(const Node & root) : FunctionBase(root), _sljit_compiler(nullptr), _jit_code(nullptr) {}

  /// tear down function (release JIT context)
  ~Function();

  /// Compile the expression tree for faster evaluation
  void compile();

  /// Evaluate the node (using JIT if available)
  Real value();

protected:
  /// JIT compiler
  struct _sljit_compiler * C; // = sljit_create_compiler(NULL);

  /// executable JIT code
  long SLJIT_CALL (*_jit_code)();

  /// floating point stack
  std::vector<double> _stack;

  /// stack entry to return (this is not necessarily _stack[0] if local variables get defined)
  std::size_t _final_stack_ptr;
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATH_USE_SLJIT

#endif // SYMBOLICMATHFUNCTION_H
