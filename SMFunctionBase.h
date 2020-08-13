///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include "SMNode.h"
#include "SMEvaluable.h"
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
class FunctionBase : public Evaluable<Real>
{
public:
  /// Construct form given node
  FunctionBase(const Node & root) : _root(root), _jit_code(nullptr) {}
  virtual ~FunctionBase() {}

  ///@{ subtree output
  std::string format() const { return _root.format(); }
  std::string formatTree() const { return _root.formatTree(); }
  ///@}

  /// Compile the expression tree for faster evaluation
  virtual void compile() = 0;

  /// check if the Function was successfully compiled
  virtual bool isCompiled() { return _jit_code; }

  /// Evaluate the node (using JIT if available)
  virtual Real value(); // TODO: move everything to operator()
  Real operator()() { return value(); }

  /// Perform one time system initialization (must be called outside a threaded region!)
  static void initialize() {}

  /// reference to the root node
  virtual const Node & root() const { return _root; }

protected:
  /// invalidate the JIT compiled function
  virtual void invalidateJIT() = 0;

  /// root node of the expression tree managed by this function
  Node _root;

  /// executable JIT code
  JITFunctionPtr _jit_code;

  /// data for storing local variables
  LocalVariables _local_variables;

  friend class Transform;
};

} // namespace SymbolicMath
