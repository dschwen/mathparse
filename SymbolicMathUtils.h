#ifndef SYMBOLICMATHUTILS_H
#define SYMBOLICMATHUTILS_H

#include "SymbolicMathTree.h"

#include <string>
#include <cstdlib>
#include <iostream>

namespace SymbolicMath
{

void simplify(NodePtr & node);

// replace this with mooseError in the future
_Noreturn void
fatalError(const std::string & error)
{
  std::cerr << error << '\n';
  std::exit(1);
}

std::string stringify(NumberNodeType type);
std::string stringify(UnaryOperatorNodeType type);
std::string stringify(BinaryOperatorNodeType type);
std::string stringify(MultinaryOperatorNodeType type);
std::string stringify(UnaryFunctionNodeType type);
std::string stringify(BinaryFunctionNodeType type);
std::string stringify(ConditionalNodeType type);
std::string stringify(MultinaryNodeType type);

template <typename T>
std::string
stringifyHelper(T type, const std::vector<std::string> list)
{
  const auto index = static_cast<int>(type);

  if (index >= list.size())
    fatalError("Unknown type");

  return list[index];
}

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHUTILS_H
