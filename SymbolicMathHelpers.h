#ifndef SYMBOLICMATHHELPERS_H
#define SYMBOLICMATHHELPERS_H

#include <vector>

#include "SymbolicMathNode.h"

namespace SymbolicMath
{

// clang-format off
Node abs(Node a) { return Node(UnaryFunctionType::ABS, a); }
Node acos(Node a) { return Node(UnaryFunctionType::ACOS, a); }
Node acosh(Node a) { return Node(UnaryFunctionType::ACOSH, a); }
Node asin(Node a) { return Node(UnaryFunctionType::ASIN, a); }
Node asinh(Node a) { return Node(UnaryFunctionType::ASINH, a); }
Node atan(Node a) { return Node(UnaryFunctionType::ATAN, a); }
Node atanh(Node a) { return Node(UnaryFunctionType::ATANH, a); }
Node ceil(Node a) { return Node(UnaryFunctionType::CEIL, a); }
Node cos(Node a) { return Node(UnaryFunctionType::COS, a); }
Node cosh(Node a) { return Node(UnaryFunctionType::COSH, a); }
Node cot(Node a) { return Node(UnaryFunctionType::COT, a); }
Node csc(Node a) { return Node(UnaryFunctionType::CSC, a); }
Node erf(Node a) { return Node(UnaryFunctionType::ERF, a); }
Node exp(Node a) { return Node(UnaryFunctionType::EXP, a); }
Node exp2(Node a) { return Node(UnaryFunctionType::EXP2, a); }
Node floor(Node a) { return Node(UnaryFunctionType::FLOOR, a); }
Node log(Node a) { return Node(UnaryFunctionType::LOG, a); }
Node log2(Node a) { return Node(UnaryFunctionType::LOG2, a); }
Node sec(Node a) { return Node(UnaryFunctionType::SEC, a); }
Node sin(Node a) { return Node(UnaryFunctionType::SIN, a); }
Node sinh(Node a) { return Node(UnaryFunctionType::SINH, a); }
Node tan(Node a) { return Node(UnaryFunctionType::TAN, a); }
Node tanh(Node a) { return Node(UnaryFunctionType::TANH, a); }
// clang-format on

// clang-format off
Node atan2(Node a, Node b) { return Node(BinaryFunctionType::ATAN2, a, b); }
Node max(Node a, Node b) { return Node(BinaryFunctionType::MAX, a, b); }
Node min(Node a, Node b) { return Node(BinaryFunctionType::MIN, a, b); }
Node plog(Node a, Node b) { return Node(BinaryFunctionType::PLOG, a, b); }
Node pow(Node a, Node b) { return Node(BinaryFunctionType::POW, a, b); }
// clang-format on

Node
conditional(Node a, Node b, Node c)
{
  return Node(ConditionalType::IF, a, b, c);
}

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHHELPERS_H
