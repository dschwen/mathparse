#ifndef SYMBOLICMATH_TREE_H
#define SYMBOLICMATH_TREE_H

#include "SymbolicMathSymbols.h"

#include <vector>
#include <memory>

typedef double Real;

namespace SymbolicMath
{

class Node;
class NodePtr : public std::unique_ptr<Node>
{
  // ignore self-resets
  void reset(Node * ptr)
  {
    if (ptr != get())
      std::unique_ptr<Node>::reset(ptr);
  }
};

using Shape = std::vector<unsigned int>;

class Node
{
public:
  virtual Real value() = 0;
  virtual Real value(const std::vector<unsigned int> & index) { checkIndex(index); };
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

  virtual Shape shape() = 0;
  virtual void checkIndex(const std::vector<unsigned int> & index);

  virtual Node * simplify() { return this; };
  virtual Node * D(unsigned int _id) = 0;

  virtual unsigned short precedence() { return 0; }
};

class ValueProviderNode : public Node
{
};

class NumberNode : public Node
{
public:
  virtual Shape shape() { return {1}; }

  virtual Node * D(unsigned int /*_id*/);

protected:
  NumberNodeType _type;
};

class RealNumberNode : public NumberNode
{
public:
  RealNumberNode(Real value) : NumberNode(), _value(value) {}
  virtual Real value() { return _value; };
  virtual std::string format() { return std::to_string(_value); };
  virtual std::string formatTree(std::string indent = "");

  virtual bool is(NumberNodeType type)
  {
    return _type == NumberNodeType::REAL || _type == NumberNodeType::_ANY;
  };
  virtual bool is(Real value) { return value == _value; };

protected:
  Real _value;
};

/**
 * Operators o of the form 'oA'
 */
class UnaryOperatorNode : public Node
{
  UnaryOperatorNode(UnaryOperatorNodeType type, Node * argument)
    : Node(), _type(type), _argument(argument)
  {
  }
  virtual Real value();
  virtual std::string format();
  virtual std::string formatTree(std::string indent = "");

  virtual unsigned short precedence() { return 3; }

  virtual Node * simplify();
  virtual Node * D(unsigned int _id);

protected:
  UnaryOperatorNodeType _type;
  NodePtr _argument;
};

/**
 * Operators o of the form 'A o B'
 */
class BinaryOperatorNode : public Node
{
  BinaryOperatorNode(BinaryOperatorNodeType type, Node * left, Node * right)
    : Node(), _type(type), _left(left), _right(right)
  {
  }
  virtual Real value();
  virtual std::string format();
  virtual std::string formatTree(std::string indent = "");

  virtual unsigned short precedence();

protected:
  BinaryOperatorNodeType _type;
  NodePtr _left;
  NodePtr _right;
};

class MultinaryOperatorNode : public Node
{
  MultinaryOperatorNode(std::initializer_list<Node *> arguments);
  virtual Real value();
  virtual std::string format();
  virtual std::string formatTree(std::string indent = "");

  virtual unsigned short precedence();

protected:
  MultinaryOperatorNodeType _type;
  std::vector<NodePtr> _arguments;
};

class MultinaryNode : public Node
{
  MultinaryNode(std::initializer_list<Node *> arguments);
  virtual Real value();

protected:
  MultinaryNodeType _type;
  std::vector<NodePtr> _arguments;
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATH_TREE_H
