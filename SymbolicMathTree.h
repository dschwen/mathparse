#ifndef SYMBOLICMATH_TREE_H
#define SYMBOLICMATH_TREE_H

#include "SymbolicMathSymbols.h"

#include <vector>
#include <memory>

typedef double Real;

namespace SymbolicMath
{

enum class NumberNodeType
{
  ANY,
  REAL,
  INTEGER,
  RATIONAL,
  _ANY
};

enum class UnaryOperatorNodeType
{
  PLUS,
  MINUS,
  FACULTY,
  NOT,
  _ANY
};

enum class BinaryOperatorNodeType
{
  ADDITION,
  MULTIPLICATION,
  _ANY
};

enum class MultinaryOperatorNodeType
{
  ADDITION,
  MULTIPLICATION,
  _ANY
};

enum class UnaryFunctionNodeType
{

};

enum class BinaryFunctionNodeType
{

};

enum class ConditionalNodeType
{
  IF,
  _ANY
};

enum class MultinaryNodeType
{
  COMPONENT
};

class Node;
using NodePtr = std::unique_ptr<Node>;
using Shape = std::vector<unsigned int>;

class Node
{
public:
  virtual Real value() = 0;
  virtual std::string format() = 0;
  virtual std::string formatTree(std::string indent = "") = 0;

  virtual Node * clone() = 0;

  virtual bool is(NumberNodeType) { return false; };
  virtual bool is(UnaryOperatorNodeType) { return false; };
  virtual bool is(BinaryOperatorNodeType) { return false; };
  virtual bool is(MultinaryOperatorNodeType) { return false; };
  virtual bool is(UnaryFunctionNodeType) { return false; };
  virtual bool is(BinaryFunctionNodeType) { return false; };
  virtual bool is(ConditionalNodeType) { return false; };

  virtual Shape shape() = 0;

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

  virtual Node * D(unsigned int _id) { return new NumberNode(0.0); };
};

class RealNumberNode : public NumberNode
{
public:
  NumberNode(Real value) : Node(), _value(value) {}
  virtual Real value() { return _value; };

  virtual bool is(NumberNodeType type)
  {
    return type == NumberNodeType::REAL || NumberNodeType::_ANY;
  };

protected:
  Real _value;
};

/**
 * Operators o of the form 'oA'
 */
class UnaryOperatorNode : public Node
{
  UnaryOperatorNode(UnaryOperatorNodeType type, NodePtr && argument)
    : Node(), _type(type), _argument(argument)
  {
  }
  virtual Real value();

  virtual unsigned short precedence() { return 3; }

protected:
  UnaryOperatorNodeType _type;
  NodePtr _argument;
};

/**
 * Operators o of the form 'A o B'
 */
class BinaryOperatorNode : public OperatorNode
{
  BinaryOperatorNode(BinaryOperatorNodeType type, NodePtr && left, NodePtr && right)
    : Node(), _type(type), _left(left), _right(right)
  {
  }

  virtual Real value() { return _value; };

  virtual unsigned short precedence() { return 3; }

protected:
  BinaryOperatorNodeType _type;
  NodePtr _left;
  NodePtr _right;
};

class MultinaryOperatorNode : public OperatorNode
{
  MultinaryOperatorNode(std::vector<Node *> arguments);
  virtual Real value() { return _value; };

  virtual unsigned short precedence();

protected:
  MultinaryOperatorType _type;
  std::vector<NodePtr> _arguments;
};

class FunctionNode : public Node
{
  FunctionNode(std::vector<Node *> arguments);
  virtual Real value() { return _value; };

protected:
  FunctionType _type;
  std::vector<NodePtr> _arguments;
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATH_TREE_H
