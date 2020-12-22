///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include "SMEvaluable.h"

#include <memory>
#include <vector>
#include <utility>
#include <string>
#include <map>
#include <type_traits>
#include <stdexcept>

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
  template <template <class> class C, int priority>
  static bool registerCompilerInternal(const std::string & C_name);

  // get a list of registered compilers
  static std::vector<std::string> listCompilers();

  // get the highest priority compiler that's registered
  static std::string bestCompiler();

  // build compiler
  static std::unique_ptr<Evaluable<T>> buildCompiler(const std::string & C_name, Function<T> & fb);
  static std::unique_ptr<Evaluable<T>> buildBestCompiler(Function<T> & fb);

protected:
  // registered compilers
  static std::map<std::string, std::pair<buildEvaluable<T>, int>> _compiler_registry;
};

// static member
template <typename T>
std::map<std::string, std::pair<buildEvaluable<T>, int>> CompilerFactory<T>::_compiler_registry;

// registration macro
#define CONCAT_IMPL(x, y) x##y
#define MACRO_CONCAT(x, y) CONCAT_IMPL(x, y)
#define registerCompiler(class, name, type, priority)                                              \
  static bool MACRO_CONCAT(register_, __COUNTER__) =                                               \
      CompilerFactory<type>::registerCompilerInternal<class, priority>(name)

// registration member function template implementation
template <typename T>
template <template <class> class C, int priority>
bool
CompilerFactory<T>::registerCompilerInternal(const std::string & C_name)
{
  static_assert(priority > 0,
                "A priority greater than zero is required for a registerCompiler directive.");

  _compiler_registry.emplace(
      C_name,
      std::make_pair([](Function<T> & fb) { return std::make_unique<C<T>>(fb); }, priority));
  return true;
}

// debug methods
template <typename T>
std::vector<std::string>
CompilerFactory<T>::listCompilers()
{
  std::vector<std::string> ret;
  for (auto p : _compiler_registry)
    ret.push_back(p.first);
  return ret;
}

template <typename T>
std::string
CompilerFactory<T>::bestCompiler()
{
  std::string ret;
  int pmax = 0;

  for (auto p : _compiler_registry)
    if (p.second.second > pmax)
    {
      pmax = p.second.second;
      ret = p.first;
    }

  return ret;
}

template <typename T>
std::unique_ptr<Evaluable<T>>
CompilerFactory<T>::buildCompiler(const std::string & C_name, Function<T> & fb)
{
  auto it = _compiler_registry.find(C_name);
  if (it == _compiler_registry.end())
    throw std::out_of_range("Compiler class '" + C_name + "' not found.");
  return it->second.first(fb);
}

template <typename T>
std::unique_ptr<Evaluable<T>>
CompilerFactory<T>::buildBestCompiler(Function<T> & fb)
{
  return buildCompiler(bestCompiler(), fb);
}

} // namespace SymbolicMath
