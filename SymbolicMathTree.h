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

  virtual Real value();
  virtual unsigned short precedence();
  virtual std::string format();

protected:
  const TokenType _type;

  union {
    const OperatorType _operator_type;
    const FunctionType _function_type;
    const unsigned int _value_provider_id;
    const Real _real;
  };

  std::vector<std::unique_ptr<Tree>> _children;
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATH_TREE_H