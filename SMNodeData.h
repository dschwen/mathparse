///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include <vector>
#include <memory>
#include <array>
#include <cmath>
#include <functional>
#include <type_traits>

#include "SMNode.h"

namespace SymbolicMath
{

template <typename T>
class Parser;
template <typename T>
class Transform;

/**
 * Node data base class. This class defines a common interface for all node data
 * objects. Node data holds the semantic content of a Node object. Node data
 * utilizes polymorphism to store a variety of different node behaviors.
 */
template <typename T>
class NodeData
{
public:
  virtual ~NodeData() {}

  virtual T value() = 0;

  virtual std::string format() const = 0;
  virtual std::string formatTree(std::string indent) const { return indent + format() + '\n'; };

  virtual NodeDataPtr<T> clone() = 0;

  virtual Node<T> getArg(unsigned int i) = 0;
  virtual std::size_t size() const { return 0; }
  virtual std::size_t hash() const = 0;

  virtual bool is(Real) const { return false; };
  virtual bool is(NumberType) const { return false; };
  virtual bool is(UnaryOperatorType) const { return false; };
  virtual bool is(BinaryOperatorType) const { return false; };
  virtual bool is(MultinaryOperatorType) const { return false; };
  virtual bool is(UnaryFunctionType) const { return false; };
  virtual bool is(BinaryFunctionType) const { return false; };
  virtual bool is(ConditionalType) const { return false; };
  virtual bool is(IntegerPowerType) const { return false; };
  virtual bool is(ValueProvider<T> * a) const { return false; }

  virtual bool isValid() const { return true; };

  // virtual Shape shape() = 0;
  virtual Shape shape() { return {1}; };

  // virtual void checkIndex(const std::vector<unsigned int> & index);

  // apply a transform visitor
  virtual void apply(Transform<T> & transform) = 0;

  virtual Node<T> D(const ValueProvider<T> & vp) = 0;

  virtual unsigned short precedence() const { return 0; }

  /// amount of net stack pointer movement of this operator
  virtual void stackDepth(std::pair<int, int> & current_max) const;

  friend Node<T>;
};

/**
 * Data class for empty invalid nodes that are constructed using the Node()
 * default constructor.
 */
template <typename T>
class EmptyData : public NodeData<T>
{
public:
  std::size_t hash() const override { return 0; }
  bool isValid() const override { return false; };

  T value() override { fatalError("invalid node"); };

  std::string format() const override { fatalError("invalid node"); };
  std::string formatTree(std::string indent) const override { fatalError("invalid node"); };
  NodeDataPtr<T> clone() override { fatalError("invalid node"); };
  Node<T> getArg(unsigned int i) override { fatalError("invalid node"); }
  Node<T> D(const ValueProvider<T> & vp) override { fatalError("invalid node"); }

  void apply(Transform<T> & transform) override;
};

/**
 * Base class template for NodeData objects with exactly N child nodes
 */
template <typename T, typename Enum, std::size_t N>
class FixedArgumentData : public NodeData<T>
{
public:
  template <typename... Args, typename = typename std::enable_if<N == sizeof...(Args), void>::type>
  FixedArgumentData(Enum type, Args &&... args) : _type(type), _args({{args...}})
  {
  }

  Node<T> getArg(unsigned int i) override { return _args[i]; };
  std::size_t size() const override { return N; }
  std::size_t hash() const override
  {
    std::size_t h = 0;
    for (auto & arg : _args)
      h ^= (arg.hash() << 1);
    return h;
  }

  bool is(Enum type) const override { return _type == type || type == Enum::_ANY; }
  void stackDepth(std::pair<int, int> & current_max) const override
  {
    for (auto & arg : _args)
      arg.stackDepth(current_max);
    current_max.first -= N - 1;
  }

  Enum _type;
  std::array<Node<T>, N> _args;
};

/**
 * Base class template for NodeData objects with an arbitrary number of child nodes
 */
template <typename T, typename Enum>
class MultinaryData : public NodeData<T>
{
public:
  MultinaryData(Enum type, std::vector<Node<T>> args) : _type(type), _args(args) {}

  Node<T> getArg(unsigned int i) override { return _args[i]; };
  std::size_t size() const override { return _args.size(); }
  std::size_t hash() const override
  {
    std::size_t h = 0;
    for (auto & arg : _args)
      h ^= (arg.hash() << 1);
    return h;
  }

  bool is(Enum type) const override { return _type == type || type == Enum::_ANY; }
  void stackDepth(std::pair<int, int> & current_max) const override
  {
    for (auto & arg : _args)
      arg.stackDepth(current_max);
    current_max.first -= _args.size() - 1;
  }

  Enum _type;
  std::vector<Node<T>> _args;
};

/**
 * Base class for any childless node that can be evaluated directly
 */
template <typename T>
class ValueProvider : public NodeData<T>
{
public:
  ValueProvider(const std::string & name) : _name(name) {}
  Node<T> getArg(unsigned int i) override { fatalError("Node has no arguments"); };

  std::string format() const override { return _name != "" ? _name : "{V}"; }

  // used to determine if two value providers are of the same type
  bool is(ValueProvider<T> * a) const override { return getTypeID() == a->getTypeID(); }

  void stackDepth(std::pair<int, int> & current_max) const override { current_max.first++; }

protected:
  std::string _name;

  // we roll our own typeid system to avoid relying on RTTI
  virtual void * getTypeID() const = 0;

  /// The parser needs to be able to read the name of the object upon registration
  friend Parser<T>;
};

/**
 * All actual value providers need to inherit from this helper template
 * which implements the custom typeid system.
 */
template <class C, typename T>
class ValueProviderDerived : public ValueProvider<T>
{
public:
protected:
  ValueProviderDerived(const std::string & name) : ValueProvider<T>(name)
  {
    (void)_vp_typeinfo_tag;
  }
  virtual void * getTypeID() const { return reinterpret_cast<void *>(&_vp_typeinfo_tag); };

private:
  static int _vp_typeinfo_tag;
};
template <class C, typename T>
int ValueProviderDerived<C, T>::_vp_typeinfo_tag;

/**
 * Local variable that is defined using the := operator
 */
template <typename T>
class LocalVariableData : public NodeData<T>
{
public:
  LocalVariableData(std::size_t id) : _id(id) {}
  Node<T> getArg(unsigned int i) override { fatalError("Node has no arguments"); };

  T value() override;

  std::string format() const override { return "{V" + stringify(_id) + "}"; }

  std::size_t id() { return _id; }

  void stackDepth(std::pair<int, int> & current_max) const override { current_max.first++; }

  NodeDataPtr<T> clone() override { fatalError("Cannot clone local variable"); };
  std::size_t hash() const override { return std::hash<const void *>{}(this); }

  Node<T> D(const ValueProvider<T> & vp) override { fatalError("Not implemented"); };
  void apply(Transform<T> & transform) override;

  std::size_t _id;
};

/**
 * Purely symbolic node that cannot be evaluated or compiled by substituted and
 * derived w.r.t.
 */
template <typename T>
class SymbolData : public ValueProviderDerived<SymbolData<T>, T>
{
  using ValueProvider<T>::_name;

public:
  SymbolData(const std::string & name) : ValueProviderDerived<SymbolData<T>, T>(name) {}

  T value() override { fatalError("Node cannot be evaluated"); }

  NodeDataPtr<T> clone() override { return std::make_shared<SymbolData<T>>(_name); };
  std::size_t hash() const override { return std::hash<std::string>{}(_name); }

  Node<T> D(const ValueProvider<T> & vp) override;
  void apply(Transform<T> & transform) override;
};

/**
 * Simple value provider that fetches its contents from a referenced T value
 */
template <typename T>
class RealReferenceData : public ValueProviderDerived<RealReferenceData<T>, T>
{
  using ValueProvider<T>::_name;

public:
  RealReferenceData(const Real & ref, const std::string & name = "")
    : ValueProviderDerived<RealReferenceData<T>, T>(name), _ref(ref)
  {
  }

  T value() override { return _ref; };

  NodeDataPtr<T> clone() override { return std::make_shared<RealReferenceData<T>>(_ref, _name); };
  std::size_t hash() const override { return std::hash<const Real *>{}(&_ref); }

  Node<T> D(const ValueProvider<T> & vp) override;
  void apply(Transform<T> & transform) override;

  const Real & _ref;
};

/**
 * Simple value provider that fetches its contents from a referenced Real array value
 * and a referenced index variable
 */
template <typename T>
class RealArrayReferenceData : public ValueProviderDerived<RealArrayReferenceData<T>, T>
{
  using ValueProvider<T>::_name;

public:
  RealArrayReferenceData(const Real & ref, const int & index, const std::string & name = "")
    : ValueProviderDerived<RealArrayReferenceData<T>, T>(name), _ref(ref), _index(index)
  {
  }

  T value() override { return (&_ref)[_index]; };

  NodeDataPtr<T> clone() override
  {
    return std::make_shared<RealArrayReferenceData<T>>(_ref, _index, _name);
  };
  std::size_t hash() const override
  {
    return std::hash<const Real *>{}(&_ref) ^ (std::hash<const int *>{}(&_index) << 1);
  }

  Node<T> D(const ValueProvider<T> & vp) override;
  void apply(Transform<T> & transform) override;

  const Real & _ref;
  const int & _index;
};

/**
 * Base class for any childless nodes that represent a constant quantity
 */
template <typename T>
class NumberData : public NodeData<T>
{
public:
  Shape shape() override { return {1}; }

  Node<T> getArg(unsigned int i) override { fatalError("Node has no arguments"); };

  Node<T> D(const ValueProvider<T> &) override { return Node<T>(0.0); }

  void stackDepth(std::pair<int, int> & current_max) const override { current_max.first++; }

  NumberType _type;
};

/**
 * Floating point constant node
 */
template <typename T>
class RealNumberData : public NumberData<T>
{
public:
  RealNumberData(T value) : NumberData<T>(), _value(value) {}

  T value() override { return _value; };

  std::string format() const override { return stringify(_value); };

  NodeDataPtr<T> clone() override { return std::make_shared<RealNumberData>(_value); };
  std::size_t hash() const override { return std::hash<Real>{}(_value); }

  bool is(NumberType type) const override;
  bool is(T value) const override { return value == _value; };

  void setValue(T value) { _value = value; }
  void apply(Transform<T> & transform) override;

  Real _value;
};

/**
 * Operators o of the form 'oA'
 */
template <typename T>
class UnaryOperatorData : public FixedArgumentData<T, UnaryOperatorType, 1>
{
  using FixedArgumentData<T, UnaryOperatorType, 1>::FixedArgumentData;

public:
  T value() override;

  std::string format() const override;
  std::string formatTree(std::string indent) const override;

  NodeDataPtr<T> clone() override;

  Node<T> D(const ValueProvider<T> &) override;
  void apply(Transform<T> & transform) override;

  unsigned short precedence() const override { return 3; }
};

/**
 * Operators o of the form 'A o B'
 */
template <typename T>
class BinaryOperatorData : public FixedArgumentData<T, BinaryOperatorType, 2>
{
  using FixedArgumentData<T, BinaryOperatorType, 2>::FixedArgumentData;

public:
  T value() override;

  std::string format() const override;
  std::string formatTree(std::string indent) const override;

  NodeDataPtr<T> clone() override;

  Node<T> D(const ValueProvider<T> &) override;
  void apply(Transform<T> & transform) override;

  unsigned short precedence() const override;
};

/**
 * Operators o of the form 'A o B o B o C o D ... o Z'
 */
template <typename T>
class MultinaryOperatorData : public MultinaryData<T, MultinaryOperatorType>
{
  using MultinaryData<T, MultinaryOperatorType>::MultinaryData;

public:
  T value() override;

  std::string format() const override;
  std::string formatTree(std::string indent) const override;

  NodeDataPtr<T> clone() override;

  Node<T> D(const ValueProvider<T> & vp) override;

  unsigned short precedence() const override;
  void apply(Transform<T> & transform) override;
};

/**
 * Functions o of the form 'F(A)'
 */
template <typename T>
class UnaryFunctionData : public FixedArgumentData<T, UnaryFunctionType, 1>
{
  using FixedArgumentData<T, UnaryFunctionType, 1>::FixedArgumentData;

public:
  T value() override;

  std::string format() const override;
  std::string formatTree(std::string indent) const override;

  NodeDataPtr<T> clone() override;

  Node<T> D(const ValueProvider<T> &) override;

  unsigned short precedence() const override { return 3; }
  void apply(Transform<T> & transform) override;
};

/**
 * Functions o of the form 'F(A,B)'
 */
template <typename T>
class BinaryFunctionData : public FixedArgumentData<T, BinaryFunctionType, 2>
{
  using FixedArgumentData<T, BinaryFunctionType, 2>::FixedArgumentData;

public:
  T value() override;

  std::string format() const override;
  std::string formatTree(std::string indent) const override;

  NodeDataPtr<T> clone() override;

  Node<T> D(const ValueProvider<T> &) override;
  void apply(Transform<T> & transform) override;
};

/**
 * Binary branch if(A, B, C). The condition A is evaluated first and iff A is
 * true B is evaluates otherwise C is evaluated.
 */
template <typename T>
class ConditionalData : public FixedArgumentData<T, ConditionalType, 3>
{
  using FixedArgumentData<T, ConditionalType, 3>::FixedArgumentData;

public:
  T value() override;

  std::string format() const override;
  std::string formatTree(std::string indent) const override;

  NodeDataPtr<T> clone() override;

  Node<T> D(const ValueProvider<T> &) override;

  void stackDepth(std::pair<int, int> & current_max) const override;
  void apply(Transform<T> & transform) override;
};

/**
 * Integer power class for faster JIT code generation
 */
template <typename T>
class IntegerPowerData : public NodeData<T>
{
public:
  IntegerPowerData(Node<T> arg, int exponent) : NodeData<T>(), _arg(arg), _exponent(exponent) {}

  T value() override { return std::pow(_arg.value(), Real(_exponent)); };

  std::string format() const override;
  std::string formatTree(std::string indent) const override;

  NodeDataPtr<T> clone() override { return std::make_shared<IntegerPowerData>(_arg, _exponent); };

  Node<T> getArg(unsigned int i) override;
  std::size_t size() const override { return 1; }
  std::size_t hash() const override { return _arg.hash() ^ (std::hash<int>{}(_exponent) << 1); }

  bool is(IntegerPowerType) const override { return true; };

  Node<T> D(const ValueProvider<T> &) override;

  void stackDepth(std::pair<int, int> & current_max) const override
  {
    _arg.stackDepth(current_max);
  }
  void apply(Transform<T> & transform) override;

  Node<T> _arg;
  int _exponent;
};

} // namespace SymbolicMath
