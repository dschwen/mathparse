#ifndef SYMBOLICMATHNODEDATA_H
#define SYMBOLICMATHNODEDATA_H

#include <vector>
#include <memory>
#include <array>
#include <cmath>
#include <type_traits>

#include <jit/jit.h>

#include "SymbolicMathNode.h"

namespace SymbolicMath
{

/**
 * Node data base class. This class defines a common interface for all node data
 * objects. Node data holds the semantic content of a Node object. Node data
 * utilizes polymorphism to store a variety of different node behaviors.
 */
class NodeData
{
public:
  virtual Real value() = 0;
  virtual jit_value_t jit(jit_function_t func) = 0;

  virtual std::string format() = 0;
  virtual std::string formatTree(std::string indent) = 0;

  virtual NodeDataPtr clone() = 0;

  virtual Node getArg(unsigned int i) = 0;
  virtual std::size_t size() { return 0; }

  virtual bool is(Real) { return false; };
  virtual bool is(NumberType) { return false; };
  virtual bool is(UnaryOperatorType) { return false; };
  virtual bool is(BinaryOperatorType) { return false; };
  virtual bool is(MultinaryOperatorType) { return false; };
  virtual bool is(UnaryFunctionType) { return false; };
  virtual bool is(BinaryFunctionType) { return false; };
  virtual bool is(ConditionalType) { return false; };

  virtual bool isValid() { return true; };

  // virtual Shape shape() = 0;
  virtual Shape shape() { return {1}; };

  // virtual void checkIndex(const std::vector<unsigned int> & index);

  virtual Node simplify() { return Node(); };
  virtual Node D(unsigned int id) = 0;

  virtual unsigned short precedence() { return 0; }

  friend Node;
};

/**
 * Data class for empty invalid nodes that are constructed using the Node()
 * default constructor. Invalid nodes are returned by NodeData::simplify()
 * when no simplification can be performed.
 */
class EmptyData : public NodeData
{
public:
  bool isValid() override { return false; };

  Real value() override { fatalError("invalid node"); };
  jit_value_t jit(jit_function_t func) override { fatalError("invalid node"); };

  std::string format() override { fatalError("invalid node"); };
  std::string formatTree(std::string indent) override { fatalError("invalid node"); };
  NodeDataPtr clone() override { fatalError("invalid node"); };
  Node getArg(unsigned int i) override { fatalError("invalid node"); }
  Node D(unsigned int id) override { fatalError("invalid node"); }
};

/**
 * Base class template for NodeData objects with exactly N child nodes
 */
template <typename Enum, std::size_t N>
class FixedArgumentData : public NodeData
{
public:
  template <typename... Args, typename = typename std::enable_if<N == sizeof...(Args), void>::type>
  FixedArgumentData(Enum type, Args &&... args) : _type(type), _args({args...})
  {
  }

  Node getArg(unsigned int i) override { return _args[i]; };
  std::size_t size() override { return N; }

  bool is(Enum type) override { return _type == type || type == Enum::_ANY; }

protected:
  std::array<Node, N> _args;
  Enum _type;
};

/**
 * Base class template for NodeData objects with an arbitrary number of child nodes
 */
template <typename Enum>
class MultinaryData : public NodeData
{
public:
  MultinaryData(Enum type, std::vector<Node> args) : _type(type), _args(args) {}

  Node getArg(unsigned int i) override { return _args[i]; };
  std::size_t size() override { return _args.size(); }

  bool is(Enum type) override { return _type == type || type == Enum::_ANY; }

protected:
  std::vector<Node> _args;
  Enum _type;
};

/**
 * Base class for any childless node that can be evaluated directly
 */
class ValueProviderData : public NodeData
{
public:
  ValueProviderData(unsigned int id) : _id(id) {}
  Real value() override { fatalError("Cannot evaluate node"); };
  jit_value_t jit(jit_function_t func) override { fatalError("cannot jit compile"); };
  NodeDataPtr clone() override
  {
    return std::make_shared<ValueProviderData>(_id); /* for debugging only! */
  };

  Node getArg(unsigned int i) override { fatalError("Node has no arguments"); };

  std::string format() override { return "_val" + std::to_string(_id); }
  std::string formatTree(std::string indent) override;

  Node D(unsigned int id) override;

protected:
  unsigned int _id;
};

/**
 * Base class for any childless nodes that represent a constant quantity
 */
class NumberData : public NodeData
{
public:
  Shape shape() override { return {1}; }

  Node getArg(unsigned int i) override { fatalError("Node has no arguments"); };

  Node D(unsigned int /*id*/) override;

protected:
  NumberType _type;
};

/**
 * Floating point constant node
 */
class RealNumberData : public NumberData
{
public:
  RealNumberData(Real value) : NumberData(), _value(value) {}

  Real value() override { return _value; };
  jit_value_t jit(jit_function_t func) override;

  std::string format() override { return stringify(_value); };
  std::string formatTree(std::string indent) override;

  NodeDataPtr clone() override { return std::make_shared<RealNumberData>(_value); };

  bool is(NumberType type) override;
  bool is(Real value) override { return value == _value; };

  void setValue(Real value) { _value = value; }

protected:
  Real _value;
};

/**
 * Operators o of the form 'oA'
 */
class UnaryOperatorData : public FixedArgumentData<UnaryOperatorType, 1>
{
  using FixedArgumentData<UnaryOperatorType, 1>::FixedArgumentData;

public:
  Real value() override;
  jit_value_t jit(jit_function_t func) override;

  std::string format() override;
  std::string formatTree(std::string indent) override;

  NodeDataPtr clone() override;

  Node simplify() override;
  Node D(unsigned int _id) override;

  unsigned short precedence() override { return 3; }
};

/**
 * Operators o of the form 'A o B'
 */
class BinaryOperatorData : public FixedArgumentData<BinaryOperatorType, 2>
{
  using FixedArgumentData<BinaryOperatorType, 2>::FixedArgumentData;

public:
  Real value() override;
  jit_value_t jit(jit_function_t func) override;

  std::string format() override;
  std::string formatTree(std::string indent) override;

  NodeDataPtr clone() override;

  Node simplify() override;
  Node D(unsigned int _id) override;

  unsigned short precedence() override;
};

/**
 * Operators o of the form 'A o B o B o C o D ... o Z'
 */
class MultinaryOperatorData : public MultinaryData<MultinaryOperatorType>
{
  using MultinaryData<MultinaryOperatorType>::MultinaryData;

public:
  Real value() override;
  jit_value_t jit(jit_function_t func) override;

  std::string format() override;
  std::string formatTree(std::string indent) override;

  NodeDataPtr clone() override;

  Node simplify() override;
  Node D(unsigned int id) override;

  unsigned short precedence() override;

private:
  void simplifyHelper(std::vector<Node> & new_args, Node arg);
};

/**
 * Functions o of the form 'F(A)'
 */
class UnaryFunctionData : public FixedArgumentData<UnaryFunctionType, 1>
{
  using FixedArgumentData<UnaryFunctionType, 1>::FixedArgumentData;

public:
  Real value() override;
  jit_value_t jit(jit_function_t func) override;

  std::string format() override;
  std::string formatTree(std::string indent) override;

  NodeDataPtr clone() override;

  Node simplify() override;
  Node D(unsigned int _id) override;

  unsigned short precedence() override { return 3; }
};

/**
 * Functions o of the form 'F(A,B)'
 */
class BinaryFunctionData : public FixedArgumentData<BinaryFunctionType, 2>
{
  using FixedArgumentData<BinaryFunctionType, 2>::FixedArgumentData;

public:
  Real value() override;
  jit_value_t jit(jit_function_t func) override;

  std::string format() override;
  std::string formatTree(std::string indent) override;

  NodeDataPtr clone() override;

  Node simplify() override;
  Node D(unsigned int _id) override;
};

/**
 * Binary branch if(A, B, C). The condition A is evaluated first and iff A is
 * true B is evaluates otherwise C is evaluated.
 */
class ConditionalData : public FixedArgumentData<ConditionalType, 3>
{
  using FixedArgumentData<ConditionalType, 3>::FixedArgumentData;

public:
  Real value() override;
  jit_value_t jit(jit_function_t func) override;

  std::string format() override;
  std::string formatTree(std::string indent) override;

  NodeDataPtr clone() override;

  Node simplify() override;
  Node D(unsigned int _id) override;
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHNODEDATA_H
