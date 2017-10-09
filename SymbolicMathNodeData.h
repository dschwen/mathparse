#ifndef SYMBOLICMATHNODEDATA_H
#define SYMBOLICMATHNODEDATA_H

#include <vector>
#include <memory>
#include <array>
#include <stack>
#include <type_traits>

#include "SymbolicMathNode.h"

namespace SymbolicMath
{

class NodeData
{
public:
  virtual Real value() = 0;
  // virtual Real value(const std::vector<unsigned int> & index) = 0;

  virtual std::string format() = 0;
  virtual std::string formatTree(std::string indent = "") = 0;

  virtual Node clone() = 0;

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

  // virtual Shape shape() = 0;
  virtual Shape shape() { return {1}; };

  virtual void checkIndex(const std::vector<unsigned int> & index);

  virtual Node simplify() { return this; };
  virtual Node D(unsigned int id) = 0;

  virtual unsigned short precedence() { return 0; }

  friend Node;
};

template <typename Enum, std::size_t N>
class FixedArgumentData : public NodeData
{
public:
  template <typename... Args, typename = typename std::enable_if<N == sizeof...(Args), void>::type>
  FixedArgumentData(Enum type, Args &&... args) : _type(type)
  {
    Node argArray[] = {args...};
    for (std::size_t i = 0; i < sizeof...(args); ++i)
      _args[i].reset(argArray[i]);
  }
  FixedArgumentData(Enum type, std::stack<Node> & stack) : _type(type)
  {
    for (std::size_t i = 0; i < N; ++i)
    {
      _args[N - 1 - i].reset(stack.top());
      stack.pop();
    }
  }

  Node getArg(unsigned int i) override { return _args[i]; };
  std::size_t size() override { return N; }

  bool is(Enum type) override { return _type == type || type == Enum::_ANY; }

protected:
  std::array<Node, N> _args;
  Enum _type;
};

template <typename Enum>
class MultinaryData : public NodeData
{
public:
  MultinaryData(Enum type, std::vector<Node> args) : _type(type), _args(args) {}
  MultinaryData(Enum type, std::stack<Node> & stack, std::size_t nargs) : _type(type)
  {
    for (std::size_t i = 0; i < nargs; ++i)
    {
      _args.insert(_args.begin(), stack.top());
      stack.pop();
    }
  }

  Node getArg(unsigned int i) override { return _args[i]; };
  std::size_t size() override { return _args.size(); }

  bool is(Enum type) override { return _type == type || type == Enum::_ANY; }

protected:
  std::vector<Node> _args;
  Enum _type;
};

class ValueProviderData : public NodeData
{
public:
  ValueProviderData(unsigned int id) : _id(id) {}
  Real value() override { fatalError("Cannot evaluate node"); };
  Node clone() override { return new ValueProviderData(_id); /* for debugging only! */ };

  Node getArg(unsigned int i) override { fatalError("Node has no arguments"); };

  std::string format() override { return "_val" + std::to_string(_id); }
  std::string formatTree(std::string indent = "") override;

  Node D(unsigned int id) override;

protected:
  unsigned int _id;
};

class NumberData : public NodeData
{
public:
  Shape shape() override { return {1}; }

  Node getArg(unsigned int i) override { fatalError("Node has no arguments"); };

  Node D(unsigned int /*id*/) override;

protected:
  NumberType _type;
};

class RealNumberData : public NumberData
{
public:
  RealNumberData(Real value) : NumberData(), _value(value) {}
  Real value() override { return _value; };
  std::string format() override { return stringify(_value); };
  std::string formatTree(std::string indent = "") override;

  Node clone() override { return new RealNumberData(_value); };

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
  std::string format() override;
  std::string formatTree(std::string indent = "") override;

  Node clone() override { return new UnaryOperatorData(_type, _args[0]); };

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
  std::string format() override;
  std::string formatTree(std::string indent = "") override;

  Node clone() override { return new BinaryOperatorData(_type, _args[0], _args[1]); };

  Node simplify() override;
  Node D(unsigned int _id) override;

  unsigned short precedence() override;
};

class MultinaryOperatorData : public MultinaryData<MultinaryOperatorType>
{
  using MultinaryData<MultinaryOperatorType>::MultinaryData;

public:
  Real value() override;
  std::string format() override;
  std::string formatTree(std::string indent = "") override;

  Node clone() override;

  Node simplify() override;
  Node D(unsigned int id) override;

  unsigned short precedence() override;

private:
  void simplifyHelper(RealNumberData *& constant, std::vector<Node> & new_args, Node arg);
};

/**
 * Functions o of the form 'F(A)'
 */
class UnaryFunctionData : public FixedArgumentData<UnaryFunctionType, 1>
{
  using FixedArgumentData<UnaryFunctionType, 1>::FixedArgumentData;

public:
  Real value() override;
  std::string format() override;
  std::string formatTree(std::string indent = "") override;

  Node clone() override { return new UnaryFunctionData(_type, _args[0]); }

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
  std::string format() override;
  std::string formatTree(std::string indent = "") override;

  Node clone() override;

  Node simplify() override;
  Node D(unsigned int _id) override;
};

class ConditionalData : public FixedArgumentData<ConditionalType, 3>
{
  using FixedArgumentData<ConditionalType, 3>::FixedArgumentData;

public:
  Real value() override;
  std::string format() override;
  std::string formatTree(std::string indent = "") override;

  Node clone() override;

  Node simplify() override;
  Node D(unsigned int _id) override;
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHNODEDATA_H
