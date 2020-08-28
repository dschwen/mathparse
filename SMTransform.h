///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include <stack>

#include "SMNodeData.h"

namespace SymbolicMath
{

template <typename T>
class Node;

template <typename T>
class Function;

/**
 * Base class for a function transforming visitor (simplification, derivation)
 */
template <typename T>
class Transform
{
public:
  Transform(Function<T> & fb) : _fb(fb) { _current_node = nullptr; }
  virtual ~Transform() {}

  virtual void operator()(SymbolData<T> *) = 0;

  virtual void operator()(UnaryOperatorData<T> *) = 0;
  virtual void operator()(BinaryOperatorData<T> *) = 0;
  virtual void operator()(MultinaryOperatorData<T> *) = 0;

  virtual void operator()(UnaryFunctionData<T> *) = 0;
  virtual void operator()(BinaryFunctionData<T> *) = 0;

  virtual void operator()(RealNumberData<T> *) = 0;
  virtual void operator()(RealReferenceData<T> *) = 0;
  virtual void operator()(RealArrayReferenceData<T> *) = 0;
  virtual void operator()(LocalVariableData<T> *) = 0;

  virtual void operator()(ConditionalData<T> *) = 0;
  virtual void operator()(IntegerPowerData<T> *) = 0;

  /// Perform one time system initialization (must be called outside a threaded region!)
  static void initialize() {}

  // set currently processed node
  void pushNode(Node<T> * node);
  void popNode();

protected:
  Function<T> & _fb;

  void apply();

  void set(Real val);
  void set(UnaryOperatorType type, Node<T> arg);
  void set(BinaryOperatorType type, Node<T> arg0, Node<T> arg1);
  void set(MultinaryOperatorType type, std::vector<Node<T>> args);
  void set(UnaryFunctionType type, Node<T> arg);
  void set(BinaryFunctionType type, Node<T> arg0, Node<T> arg1);
  void set(ConditionalType type, Node<T> arg0, Node<T> arg1, Node<T> arg2);
  void set(IntegerPowerType, Node<T> arg, int exponent);

  Node<T> * _current_node;
  std::stack<Node<T> *> _node_stack;
};

} // namespace SymbolicMath
