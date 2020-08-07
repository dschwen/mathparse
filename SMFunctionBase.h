///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include "SMNode.h"
#include "SMJITTypes.h"

namespace SymbolicMath
{

class Transform;

using ValueProviderPtr = std::shared_ptr<ValueProvider>;
using LocalVariables = std::vector<std::pair<Real, bool>>;

/**
 * The Function class is the top level wrapper for a Node based expression tree.
 * It manages the active value providers and the just in time compilation.
 */
class FunctionBase
{
public:
  /// Construct form given node
  FunctionBase(const Node & root) : _root(root), _jit_code(nullptr) {}
  virtual ~FunctionBase() {}

  /// Simplify the subtree at the node in place
  void simplify()
  {
    _root.simplify();
    invalidateJIT();
  }

  ///@{ subtree output
  std::string format() const { return _root.format(); }
  std::string formatTree() const { return _root.formatTree(); }
  ///@}

  /// Compile the expression tree for faster evaluation
  virtual void compile() = 0;

  /// check if the Function was successfully compiled
  virtual bool isCompiled() { return _jit_code; }

  /// Evaluate the node (using JIT if available)
  virtual Real value();

  /// Perform one time system initialization (must be called outside a threaded region!)
  static void initialize() {}

  // apply a transform visitor
  virtual void apply(Transform & transform);

protected:
  /// invalidate the JIT compiled function
  virtual void invalidateJIT() = 0;

  /// root node of the exprssion tree managed by this function
  Node _root;

  /// executable JIT code
  JITFunctionPtr _jit_code;

  /// data for storing local variables
  LocalVariables _local_variables;
};

} // namespace SymbolicMath
