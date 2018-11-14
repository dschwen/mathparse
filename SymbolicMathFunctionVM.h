#ifndef SYMBOLICMATHFUNCTIONVM_H
#define SYMBOLICMATHFUNCTIONVM_H

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

  /// Returns the derivative of the subtree at the node w.r.t. value provider id
  Function D(ValueProviderPtr vp) const { return Function(_root.D(*vp)); }

  /// Compile the expression tree for faster evaluation
  void compile() override;

  /// always returns true as byte code is build on demand if not manually triggered
  bool isCompiled() override { return true; }

  // evaluate bytecode
  Real value() override;

protected:
  /// invalidate the JIT compiled function
  void invalidateJIT() override { _byte_code.clear(); }

  /// byte code data
  ByteCode _byte_code;

  /// execution stack (not thread safe)
  std::vector<Real> _stack;
};

} // namespace SymbolicMath

#endif // SYMBOLICMATHFUNCTIONVM_H
