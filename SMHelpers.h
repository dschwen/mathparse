///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include <memory>

#include "SMNode.h"

namespace SymbolicMath
{

// clang-format off
template <typename T> Node<T> abs(Node<T> a) { return Node<T>(UnaryFunctionType::ABS, a); }
template <typename T> Node<T> acos(Node<T> a) { return Node<T>(UnaryFunctionType::ACOS, a); }
template <typename T> Node<T> acosh(Node<T> a) { return Node<T>(UnaryFunctionType::ACOSH, a); }
template <typename T> Node<T> asin(Node<T> a) { return Node<T>(UnaryFunctionType::ASIN, a); }
template <typename T> Node<T> asinh(Node<T> a) { return Node<T>(UnaryFunctionType::ASINH, a); }
template <typename T> Node<T> atan(Node<T> a) { return Node<T>(UnaryFunctionType::ATAN, a); }
template <typename T> Node<T> atanh(Node<T> a) { return Node<T>(UnaryFunctionType::ATANH, a); }
template <typename T> Node<T> ceil(Node<T> a) { return Node<T>(UnaryFunctionType::CEIL, a); }
template <typename T> Node<T> cos(Node<T> a) { return Node<T>(UnaryFunctionType::COS, a); }
template <typename T> Node<T> cosh(Node<T> a) { return Node<T>(UnaryFunctionType::COSH, a); }
template <typename T> Node<T> cot(Node<T> a) { return Node<T>(UnaryFunctionType::COT, a); }
template <typename T> Node<T> csc(Node<T> a) { return Node<T>(UnaryFunctionType::CSC, a); }
template <typename T> Node<T> erf(Node<T> a) { return Node<T>(UnaryFunctionType::ERF, a); }
template <typename T> Node<T> exp(Node<T> a) { return Node<T>(UnaryFunctionType::EXP, a); }
template <typename T> Node<T> exp2(Node<T> a) { return Node<T>(UnaryFunctionType::EXP2, a); }
template <typename T> Node<T> floor(Node<T> a) { return Node<T>(UnaryFunctionType::FLOOR, a); }
template <typename T> Node<T> log(Node<T> a) { return Node<T>(UnaryFunctionType::LOG, a); }
template <typename T> Node<T> log2(Node<T> a) { return Node<T>(UnaryFunctionType::LOG2, a); }
template <typename T> Node<T> sec(Node<T> a) { return Node<T>(UnaryFunctionType::SEC, a); }
template <typename T> Node<T> sin(Node<T> a) { return Node<T>(UnaryFunctionType::SIN, a); }
template <typename T> Node<T> sinh(Node<T> a) { return Node<T>(UnaryFunctionType::SINH, a); }
template <typename T> Node<T> tan(Node<T> a) { return Node<T>(UnaryFunctionType::TAN, a); }
template <typename T> Node<T> tanh(Node<T> a) { return Node<T>(UnaryFunctionType::TANH, a); }
// clang-format on

// clang-format off
template <typename T> Node<T> atan2(Node<T> a, Node<T> b) { return Node<T>(BinaryFunctionType::ATAN2, a, b); }
template <typename T> Node<T> max(Node<T> a, Node<T> b) { return Node<T>(BinaryFunctionType::MAX, a, b); }
template <typename T> Node<T> min(Node<T> a, Node<T> b) { return Node<T>(BinaryFunctionType::MIN, a, b); }
template <typename T> Node<T> plog(Node<T> a, Node<T> b) { return Node<T>(BinaryFunctionType::PLOG, a, b); }
template <typename T> Node<T> pow(Node<T> a, Node<T> b) { return Node<T>(BinaryFunctionType::POW, a, b); }
// clang-format on

template <typename T>
Node<T>
conditional(Node<T> a, Node<T> b, Node<T> c)
{
  return Node<T>(ConditionalType::IF, a, b, c);
}

template <typename T>
std::shared_ptr<ValueProvider<T>>
symbol(const std::string & name)
{
  return std::make_shared<SymbolicMath::SymbolData<T>>(name);
}

} // namespace SymbolicMath
