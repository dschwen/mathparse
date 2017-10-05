#ifndef SYMBOLICMATHTREE_H
#define SYMBOLICMATHTREE_H

#include <vector>
#include <memory>
#include <array>
#include <stack>
#include <type_traits>

#include "SymbolicMathSymbols.h"

namespace SymbolicMath
{

class Node;
class NodePtr : public std::unique_ptr<Node>
{
  using std::unique_ptr<Node>::unique_ptr;

public:
  // ignore self-resets
  void reset(Node * ptr)
  {
    if (ptr != get())
      std::unique_ptr<Node>::reset(ptr);
  }
};

void simplify(NodePtr & node);

using Shape = std::vector<unsigned int>;

class Node
{
public:
  virtual Real value() = 0;
  // virtual Real value(const std::vector<unsigned int> & index) = 0;

  virtual std::string format() = 0;
  virtual std::string formatTree(std::string indent = "") = 0;

  virtual Node * clone() = 0;

  virtual bool is(Real) { return false; };
  virtual bool is(NumberNodeType) { return false; };
  virtual bool is(UnaryOperatorNodeType) { return false; };
  virtual bool is(BinaryOperatorNodeType) { return false; };
  virtual bool is(MultinaryOperatorNodeType) { return false; };
  virtual bool is(UnaryFunctionNodeType) { return false; };
  virtual bool is(BinaryFunctionNodeType) { return false; };
  virtual bool is(ConditionalNodeType) { return false; };

  // virtual Shape shape() = 0;
  virtual Shape shape() { return {1}; };

  virtual void checkIndex(const std::vector<unsigned int> & index);

  virtual Node * simplify() { return this; };
  virtual Node * D(unsigned int id) = 0;

  virtual unsigned short precedence() { return 0; }
};

template <typename Enum, std::size_t N>
class FixedArgumentNode : public Node
{
public:
  template <typename... Args, typename = typename std::enable_if<N == sizeof...(Args), void>::type>
  FixedArgumentNode(Enum type, Args &&... args) : _type(type)
  {
    Node * argArray[] = {args...};
    for (std::size_t i = 0; i < sizeof...(args); ++i)
      _args[i].reset(argArray[i]);
  }
  FixedArgumentNode(Enum type, std::stack<Node *> & stack) : _type(type)
  {
    for (std::size_t i = 0; i < N; ++i)
    {
      _args[N - 1 - i].reset(stack.top());
      stack.pop();
    }
  }

  bool is(Enum type) override { return _type == type || type == Enum::_ANY; }

protected:
  std::array<NodePtr, N> _args;
  Enum _type;
};

template <typename Enum>
class MultinaryNode : public Node
{
public:
  MultinaryNode(Enum type, std::vector<Node *> args) : _type(type)
  {
    for (auto arg : args)
      _args.emplace_back(arg);
  }
  MultinaryNode(Enum type, std::stack<Node *> & stack, std::size_t nargs) : _type(type)
  {
    for (std::size_t i = 0; i < nargs; ++i)
    {
      _args.emplace(_args.begin(), stack.top());
      stack.pop();
    }
  }

  bool is(Enum type) override { return _type == type || type == Enum::_ANY; }

protected:
  std::vector<NodePtr> _args;
  Enum _type;
};

class ValueProviderNode : public Node
{
public:
  ValueProviderNode(unsigned int id) : _id(id) {}
  Real value() override { fatalError("Cannot evaluate node"); };
  Node * clone() override { fatalError("Cannot clone node"); };

  std::string format() override { return "_val" + std::to_string(_id); }
  std::string formatTree(std::string indent = "") override;

  Node * D(unsigned int id) override;

protected:
  unsigned int _id;
};

class NumberNode : public Node
{
public:
  Shape shape() override { return {1}; }

  Node * D(unsigned int /*id*/) override;

protected:
  NumberNodeType _type;
};

class RealNumberNode : public NumberNode
{
public:
  RealNumberNode(Real value) : NumberNode(), _value(value) {}
  Real value() override { return _value; };
  std::string format() override { return std::to_string(_value); };
  std::string formatTree(std::string indent = "") override;

  Node * clone() override { return new RealNumberNode(_value); };

  bool is(NumberNodeType type) override;
  bool is(Real value) override { return value == _value; };

  void setValue(Real value) { _value = value; }

protected:
  Real _value;
};

/**
 * Operators o of the form 'oA'
 */
class UnaryOperatorNode : public FixedArgumentNode<UnaryOperatorNodeType, 1>
{
  using FixedArgumentNode<UnaryOperatorNodeType, 1>::FixedArgumentNode;

public:
  Real value() override;
  std::string format() override;
  std::string formatTree(std::string indent = "") override;

  Node * clone() override { return new UnaryOperatorNode(_type, _args[0]->clone()); };

  Node * simplify() override;
  Node * D(unsigned int _id) override;

  unsigned short precedence() override { return 3; }
};

/**
 * Operators o of the form 'A o B'
 */
class BinaryOperatorNode : public FixedArgumentNode<BinaryOperatorNodeType, 2>
{
  using FixedArgumentNode<BinaryOperatorNodeType, 2>::FixedArgumentNode;

public:
  Real value() override;
  std::string format() override;
  std::string formatTree(std::string indent = "") override;

  Node * clone() override
  {
    return new BinaryOperatorNode(_type, _args[0]->clone(), _args[1]->clone());
  };

  Node * simplify() override;
  Node * D(unsigned int _id) override;

  unsigned short precedence() override;
};

class MultinaryOperatorNode : public MultinaryNode<MultinaryOperatorNodeType>
{
  using MultinaryNode<MultinaryOperatorNodeType>::MultinaryNode;

public:
  Real value() override;
  std::string format() override;
  std::string formatTree(std::string indent = "") override;

  Node * clone() override;

  Node * simplify() override;
  Node * D(unsigned int id) override;

  unsigned short precedence() override;
};

/**
 * Functions o of the form 'F(A)'
 */
class UnaryFunctionNode : public FixedArgumentNode<UnaryFunctionNodeType, 1>
{
  using FixedArgumentNode<UnaryFunctionNodeType, 1>::FixedArgumentNode;

public:
  Real value() override;
  std::string format() override;
  std::string formatTree(std::string indent = "") override;

  Node * clone() override { return new UnaryFunctionNode(_type, _args[0]->clone()); }

  Node * simplify() override;
  Node * D(unsigned int _id) override;

  unsigned short precedence() override { return 3; }
};

/**
 * Functions o of the form 'F(A,B)'
 */
class BinaryFunctionNode : public FixedArgumentNode<BinaryFunctionNodeType, 2>
{
  using FixedArgumentNode<BinaryFunctionNodeType, 2>::FixedArgumentNode;

public:
  Real value() override;
  std::string format() override;
  std::string formatTree(std::string indent = "") override;

  Node * clone() override;

  Node * simplify() override;
  Node * D(unsigned int _id) override;
};

class ConditionalNode : public FixedArgumentNode<ConditionalNodeType, 3>
{
  using FixedArgumentNode<ConditionalNodeType, 3>::FixedArgumentNode;

public:
  Real value() override;
  std::string format() override;
  std::string formatTree(std::string indent = "") override;

  Node * clone() override;

  Node * simplify() override;
  Node * D(unsigned int _id) override;
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATH_TREE_H
