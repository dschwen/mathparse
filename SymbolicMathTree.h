#ifndef SYMBOLICMATH_TREE_H
#define SYMBOLICMATH_TREE_H

#include "SymbolicMathSymbols.h"
#include <vector>

typedef double Real;

namespace SymbolicMath
{

class Tree;

class Tree
{
public:
  Tree(OperatorType operator_type, std::vector<std::unique_ptr<Tree>> children);
  Tree(FunctionType function_type, std::vector<std::unique_ptr<Tree>> children);
  Tree(Real real, std::vector<std::unique_ptr<Tree>> children);

  Real value();
  std::string format();

protected:
  TokenType _type;

  union {
    OperatorType _operator_type;
    FunctionType _function_type;
    unsigned int _value_provider_id;
    Real _real;
  };

  std::vector<std::unique_ptr<Tree>> _child;
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATH_TREE_H
