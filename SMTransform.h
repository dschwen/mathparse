///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

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
  Transform(Function<T> & fb);
  Transform(Node<T> & root);
  virtual ~Transform() {}

  virtual void operator()(Node<T> &, SymbolData<T> &) = 0;

  virtual void operator()(Node<T> &, UnaryOperatorData<T> &) = 0;
  virtual void operator()(Node<T> &, BinaryOperatorData<T> &) = 0;
  virtual void operator()(Node<T> &, MultinaryOperatorData<T> &) = 0;

  virtual void operator()(Node<T> &, UnaryFunctionData<T> &) = 0;
  virtual void operator()(Node<T> &, BinaryFunctionData<T> &) = 0;

  virtual void operator()(Node<T> &, RealNumberData<T> &) = 0;
  virtual void operator()(Node<T> &, RealReferenceData<T> &) = 0;
  virtual void operator()(Node<T> &, RealArrayReferenceData<T> &) = 0;
  virtual void operator()(Node<T> &, LocalVariableData<T> &) = 0;

  virtual void operator()(Node<T> &, ConditionalData<T> &) = 0;
  virtual void operator()(Node<T> &, IntegerPowerData<T> &) = 0;

  /// Perform one time system initialization (must be called outside a threaded region!)
  static void initialize() {}

protected:
  void apply();

  void set(Node<T> & node, Real val);
  void set(Node<T> & node, UnaryOperatorType type, Node<T> arg);
  void set(Node<T> & node, BinaryOperatorType type, Node<T> arg0, Node<T> arg1);
  void set(Node<T> & node, MultinaryOperatorType type, Node<T> arg0, Node<T> arg1);
  void set(Node<T> & node, MultinaryOperatorType type, std::vector<Node<T>> args);
  void set(Node<T> & node, UnaryFunctionType type, Node<T> arg);
  void set(Node<T> & node, BinaryFunctionType type, Node<T> arg0, Node<T> arg1);
  void set(Node<T> & node, ConditionalType type, Node<T> arg0, Node<T> arg1, Node<T> arg2);
  void set(Node<T> & node, IntegerPowerType, Node<T> arg, int exponent);

private:
  Node<T> & _root;
};

} // namespace SymbolicMath
