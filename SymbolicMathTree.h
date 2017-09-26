#ifndef SYMBOLICMATH_TREE_H
#define SYMBOLICMATH_TREE_H

#include "SymbolicMathSymbols.h"

#include <vector>
#include <memory>

typedef double Real;

namespace SymbolicMath
{

class Tree;

class Tree
{
public:
  Tree(OperatorType operator_type, std::vector<Tree *> children);
  Tree(FunctionType function_type, std::vector<Tree *> children);
  Tree(Real real);
  Tree(unsigned int real);

  Real value();
  std::string format();
  std::string formatTree(std::string indent = "");

  bool isNumber(Real number);
  bool simplify();

  std::unique_ptr<Tree> D(unsigned int _id);

protected:
  TokenType _type;

  union {
    OperatorType _operator_type;
    FunctionType _function_type;
    unsigned int _value_provider_id;
    Real _real;
  };

  std::vector<std::unique_ptr<Tree>> _children;

  unsigned short precedence();
  void become(std::unique_ptr<Tree> tree);
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATH_TREE_H
