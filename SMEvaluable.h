///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

namespace SymbolicMath
{

/**
 * Abstract interface for an object that is evaluable using operator().
 */
template <typename T>
class Evaluable
{
public:
  /// Evaluate the node (using JIT if available)
  virtual T operator()() = 0;
};

} // namespace SymbolicMath
