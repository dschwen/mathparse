///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

// forward declaration
namespace SymbolicMath
{

template <typename T>
class Node;

}

// forward declarations
namespace std
{

template <typename T>
SymbolicMath::Node<T> abs(const SymbolicMath::Node<T> &);
template <typename T>
SymbolicMath::Node<T> acos(const SymbolicMath::Node<T> &);
template <typename T>
SymbolicMath::Node<T> acosh(const SymbolicMath::Node<T> &);
template <typename T>
SymbolicMath::Node<T> asin(const SymbolicMath::Node<T> &);
template <typename T>
SymbolicMath::Node<T> asinh(const SymbolicMath::Node<T> &);
template <typename T>
SymbolicMath::Node<T> atan(const SymbolicMath::Node<T> &);
template <typename T>
SymbolicMath::Node<T> atanh(const SymbolicMath::Node<T> &);
template <typename T>
SymbolicMath::Node<T> cbrt(const SymbolicMath::Node<T> &);
template <typename T>
SymbolicMath::Node<T> ceil(const SymbolicMath::Node<T> &);
template <typename T>
SymbolicMath::Node<T> cos(const SymbolicMath::Node<T> &);
template <typename T>
SymbolicMath::Node<T> cosh(const SymbolicMath::Node<T> &);
template <typename T>
SymbolicMath::Node<T> erf(const SymbolicMath::Node<T> &);
template <typename T>
SymbolicMath::Node<T> erfc(const SymbolicMath::Node<T> &);
template <typename T>
SymbolicMath::Node<T> exp(const SymbolicMath::Node<T> &);
template <typename T>
SymbolicMath::Node<T> exp2(const SymbolicMath::Node<T> &);
template <typename T>
SymbolicMath::Node<T> floor(const SymbolicMath::Node<T> &);
template <typename T>
SymbolicMath::Node<T> log(const SymbolicMath::Node<T> &);
template <typename T>
SymbolicMath::Node<T> log10(const SymbolicMath::Node<T> &);
template <typename T>
SymbolicMath::Node<T> log2(const SymbolicMath::Node<T> &);
template <typename T>
SymbolicMath::Node<T> round(const SymbolicMath::Node<T> &);
template <typename T>
SymbolicMath::Node<T> sin(const SymbolicMath::Node<T> &);
template <typename T>
SymbolicMath::Node<T> sinh(const SymbolicMath::Node<T> &);
template <typename T>
SymbolicMath::Node<T> sqrt(const SymbolicMath::Node<T> &);
template <typename T>
SymbolicMath::Node<T> tan(const SymbolicMath::Node<T> &);
template <typename T>
SymbolicMath::Node<T> tanh(const SymbolicMath::Node<T> &);

} // namespace std
