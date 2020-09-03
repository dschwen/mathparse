///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include <memory>
#include <algorithm>

#include "SMNodeData.h"
#include "SMFunction.h"
#include "SMTransformSimplify.h"

namespace SymbolicMath
{

template <typename T>
Simplify<T>::Simplify(Function<T> & fb) : Transform<T>(fb)
{
  apply();
}

template <typename T>
void
Simplify<T>::operator()(UnaryOperatorData<T> * n)
{
  // simplify child
  n->_args[0].apply(*this);
  if (n->_args[0].is(NumberType::_ANY))
    set(n->value());
}

template <typename T>
void
Simplify<T>::operator()(BinaryOperatorData<T> * n)
{
  // simplify children
  n->_args[0].apply(*this);
  n->_args[1].apply(*this);

  // constant folding
  if (n->_args[0].is(NumberType::_ANY) && n->_args[1].is(NumberType::_ANY))
  {
    set(n->value());
    return;
  }

  switch (n->_type)
  {
    case BinaryOperatorType::SUBTRACTION:
      // 0 - b = -b
      if (n->_args[0].is(0.0))
        set(UnaryOperatorType::MINUS, n->_args[1]);
      // a - 0 = a
      else if (n->_args[1].is(0.0))
        _current_node->_data = n->_args[0]._data;
      return;

    case BinaryOperatorType::DIVISION:
      // a/1 = a
      if (n->_args[1].is(1.0))
        _current_node->_data = n->_args[0]._data;
      // 0/b = 0
      else if (n->_args[0].is(0.0))
        set(0.0);
      return;

    case BinaryOperatorType::POWER:
      // turn operator into function and simplify
      set(BinaryFunctionType::POW, n->_args[0], n->_args[1]);
      _current_node->apply(*this);
      return;

    case BinaryOperatorType::MODULO:
      if (n->_args[1].is(1.0))
        set(0.0);
      return;

    case BinaryOperatorType::LOGICAL_OR:
      if ((n->_args[0].is(NumberType::_ANY) && n->_args[0].value() != 0.0) ||
          (n->_args[1].is(NumberType::_ANY) && n->_args[1].value() != 0.0))
        set(1.0);
      return;

    case BinaryOperatorType::LOGICAL_AND:
      if (n->_args[0].is(0.0) || n->_args[1].is(0.0))
        set(0.0);
      return;

    default:
      return;
  }
}

template <typename T>
void
Simplify<T>::operator()(MultinaryOperatorData<T> * n)
{
  // simplify and hoist children
  std::vector<Node<T>> newargs;
  for (auto & arg : n->_args)
  {
    arg.apply(*this);
    if (arg.is(n->_type))
    {
      auto arg_data = std::static_pointer_cast<MultinaryOperatorData<T>>(arg._data);
      newargs.insert(newargs.end(), arg_data->_args.begin(), arg_data->_args.end());
    }
    else
      newargs.push_back(arg);
  }
  n->_args = newargs;

  switch (n->_type)
  {
    case MultinaryOperatorType::ADDITION:
    case MultinaryOperatorType::MULTIPLICATION:
    {
      // sort constant numbers to the end
      std::sort(n->_args.begin(), n->_args.end(), [](Node<T> & a, Node<T> & b) {
        return !a.is(NumberType::_ANY) && b.is(NumberType::_ANY);
      });
      // find first number node
      auto first_num = std::find_if(
          n->_args.begin(), n->_args.end(), [](Node<T> & a) { return a.is(NumberType::_ANY); });
      if (first_num == n->_args.end())
        return;
      Real val = first_num->value();
      while (--n->_args.end() != first_num)
      {
        if (n->_type == MultinaryOperatorType::ADDITION)
          val += n->_args.back().value();
        else
          val *= n->_args.back().value();
        n->_args.pop_back();
      }

      if ((val == 1.0 && n->_type == MultinaryOperatorType::MULTIPLICATION) ||
          (val == 0.0 && n->_type == MultinaryOperatorType::ADDITION))
        n->_args.pop_back();
      else
        first_num->_data = std::make_shared<RealNumberData<T>>(val);

      return;
    }

    default:
      return;
  }
}

template <typename T>
void
Simplify<T>::operator()(UnaryFunctionData<T> * n)
{
  // simplify child
  n->_args[0].apply(*this);
  if (n->_args[0].is(NumberType::_ANY))
    set(n->value());
}

template <typename T>
void
Simplify<T>::operator()(BinaryFunctionData<T> * n)
{
  // simplify children
  n->_args[0].apply(*this);
  n->_args[1].apply(*this);

  // constant folding
  if (n->_args[0].is(NumberType::_ANY) && n->_args[1].is(NumberType::_ANY))
  {
    set(n->value());
    return;
  }

  switch (n->_type)
  {
    case BinaryFunctionType::POW:
      if (n->_args[1].is(1.0))
      {
        _current_node->_data = n->_args[0]._data;
        return;
      }

      if (n->_args[1].is(0.0))
      {
        set(1.0);
        return;
      }

      if (n->_args[1].is(0.5))
      {
        set(UnaryFunctionType::SQRT, n->_args[0]);
        return;
      }

      if (n->_args[1].is(1.0 / 3.0))
      {
        set(UnaryFunctionType::CBRT, n->_args[0]);
        return;
      }

      if (n->_args[0].is(2.0))
      {
        set(UnaryFunctionType::EXP2, n->_args[1]);
        return;
      }

      if (n->_args[1].is(NumberType::_ANY))
      {
        auto val = n->_args[1].value();
        if (std::floor(val) == val)
        {
          set(IntegerPowerType::_ANY, n->_args[0], val);
          return;
        }
      }

      return;

    default:
      return;
  }
}

template <typename T>
void
Simplify<T>::operator()(ConditionalData<T> * n)
{
  if (n->_type != ConditionalType::IF)
    fatalError("Conditional not implemented");

  n->_args[0].apply(*this);
  n->_args[1].apply(*this);
  n->_args[2].apply(*this);

  // if the conditional is constant we can drop a branch
  if (n->_args[0].is(NumberType::_ANY))
  {
    if (n->_args[0].value() != 0.0)
      _current_node->_data = n->_args[1]._data;
    else
      _current_node->_data = n->_args[2]._data;
  }
}

template <typename T>
void
Simplify<T>::operator()(IntegerPowerData<T> * n)
{
  // (a^b)^c = a^(b*c) (c00^c01) ^ c1 = c00 ^ (c01*c1)
  if (n->_arg.is(IntegerPowerType::_ANY))
  {
    set(IntegerPowerType::_ANY, n->_arg[0], n->_arg[1].value() * n->_exponent);
    _current_node->apply(*this);
  }
  else if (n->_arg.is(NumberType::_ANY))
    set(n->value());
  else if (n->_exponent == 1)
    _current_node->_data = n->_arg._data;
  else if (n->_exponent == 0)
    set(1.0);
}

template class Simplify<Real>;

} // namespace SymbolicMath
