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

  Real value();
  unsigned short precedence();
  std::string format();
  bool constant();

  bool foldConstants();

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
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATH_TREE_H
