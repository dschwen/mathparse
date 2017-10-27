#ifndef SYMBOLICMATHFUNCTIONLLVMIR_H
#define SYMBOLICMATHFUNCTIONLLVMIR_H

#include "SymbolicMathFunctionBase.h"
#include "SymbolicMathJITTypesLLVMIR.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/IRTransformLayer.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Support/CachePruning.h"
#include "llvm/Target/TargetMachine.h"

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
class Function : public FunctionBase
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

  using JITFunction = double (*)();

private:
  std::shared_ptr<llvm::Module> optimizeModule(std::shared_ptr<llvm::Module> M);

  /// LLVM Target machine
  std::unique_ptr<llvm::TargetMachine> _llvm_target_machine;

  /// LLVM Data layout
  const llvm::DataLayout _llvm_data_layout;

  /// Object layer
  llvm::orc::RTDyldObjectLinkingLayer _llvm_object_layer;

  // Compile layer
  llvm::orc::IRCompileLayer<decltype(_llvm_object_layer), llvm::orc::SimpleCompiler>
      _llvm_compile_layer;

  // Optimize layer
  using OptimizeFunction =
      std::function<std::shared_ptr<llvm::Module>(std::shared_ptr<llvm::Module>)>;
  llvm::orc::IRTransformLayer<decltype(_llvm_compile_layer), OptimizeFunction> _llvm_optimize_layer;

  /// LLVM module handle
  using ModuleHandle = decltype(_llvm_optimize_layer)::ModuleHandleT;
  ModuleHandle _module_handle;
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHFUNCTIONLLVMIR_H
