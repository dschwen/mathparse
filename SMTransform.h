///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include <stack>

#include "SMNodeData.h"

namespace SymbolicMath
{

class Node;
class FunctionBase;

/**
 * Base class for a function transforming visitor (simplification, derivation)
 */
class Transform
{
public:
  Transform(FunctionBase &) { _current_node = nullptr; }

  virtual void operator()(SymbolData *) = 0;

  virtual void operator()(UnaryOperatorData *) = 0;
  virtual void operator()(BinaryOperatorData *) = 0;
  virtual void operator()(MultinaryOperatorData *) = 0;

  virtual void operator()(UnaryFunctionData *) = 0;
  virtual void operator()(BinaryFunctionData *) = 0;

  virtual void operator()(RealNumberData *) = 0;
  virtual void operator()(RealReferenceData *) = 0;
  virtual void operator()(RealArrayReferenceData *) = 0;
  virtual void operator()(LocalVariableData *) = 0;

  virtual void operator()(ConditionalData *) = 0;
  virtual void operator()(IntegerPowerData *) = 0;

  // set currently processed node
  void pushNode(Node * node);
  void popNode();

protected:
  void set(Real val);
  void set(UnaryOperatorType type, Node arg);
  void set(BinaryOperatorType type, Node arg0, Node arg1);
  void set(MultinaryOperatorType type, std::vector<Node> args);
  void set(UnaryFunctionType type, Node arg);
  void set(BinaryFunctionType type, Node arg0, Node arg1);
  void set(ConditionalType type, Node arg0, Node arg1, Node arg2);
  void set(IntegerPowerType, Node arg, int exponent);

  Node * _current_node;
  std::stack<Node *> _node_stack;
};

} // namespace SymbolicMath
