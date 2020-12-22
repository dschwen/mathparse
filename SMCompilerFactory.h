///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include "SMEvaluable.h"

#include <memory>
#include <vector>
#include <string>
#include <map>

namespace SymbolicMath
{

// build function type
template <typename T>
using buildEvaluable = std::function<std::unique_ptr<Evaluable<T>>(Function<T> &)>;

template <typename T>
class CompilerFactory
{
  // private constructor - do not build this object, just use its static members
  CompilerFactory() {}

public:
  // register a compiler class C with string name C_name that operates on values of type T
  template <template <class> class C>
  static bool registerCompilerInternal(const std::string & C_name);

protected:
  // registered compilers
  static std::map<std::string, buildEvaluable<T>> _compiler_registry;
};

// static member
template <typename T>
std::map<std::string, buildEvaluable<T>> CompilerFactory<T>::_compiler_registry;

// registration macro
#define CONCAT_IMPL(x, y) x##y
#define MACRO_CONCAT(x, y) CONCAT_IMPL(x, y)
#define registerCompiler(class, name, type)                                                        \
  static bool MACRO_CONCAT(register_, __COUNTER__) =                                               \
      CompilerFactory<type>::registerCompilerInternal<class>(name)

// registration member function template implementation
template <typename T>
template <template <class> class C>
bool
CompilerFactory<T>::registerCompilerInternal(const std::string & C_name)
{
  _compiler_registry.emplace(C_name, [](Function<T> & fb) { return std::make_unique<C<T>>(fb); });
  return true;
}

} // namespace SymbolicMath
