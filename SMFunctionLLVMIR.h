///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/Format.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>

#include "LLJITHelper.h"
#include "SMFunction.h"

#include <functional>
#include <memory>

namespace SymbolicMath
{

using ValueProviderPtr = std::shared_ptr<ValueProvider>;

/**
 * Perform one time system initialization for the JIT module
 */
class FunctionJITInitialization
{
public:
  FunctionJITInitialization();

  FunctionJITInitialization(FunctionJITInitialization const &) = delete;
  void operator=(FunctionJITInitialization const &) = delete;
};

/**
 * The Function class is the top level wrapper for a Node based expression tree.
 * It manages the active value providers and the just in time compilation.
 */
class Function : public Function
{
public:
  /// Construct form given node
  Function(const Node & root);

  /// Copy constructor (do not copy JIT stuff!)
  Function(const Function & F);

  /// tear down function (release JIT context)
  virtual ~Function();

  /// Returns the derivative of the subtree at the node w.r.t. value provider id
  Function D(ValueProviderPtr vp) const { return Function(_root.D(*vp)); }

  /// Compile the expression tree for faster evaluation
  void compile() override;

  /// Perform one time system initialization (must be called outside a threaded region!)
  static void initialize() { static FunctionJITInitialization init; }

protected:
  /// invalidate the JIT compiled function
  void invalidateJIT() override;

private:
  LLJITHelper _lljit;
};

} // namespace SymbolicMath
