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
void
Simplify<T>::operator()(Node<T> & node, UnaryOperatorData<T> & data)
{
  // simplify child
  data._args[0].apply(*this);
  if (data._args[0].is(NumberType::_ANY))
    set(node, data.value());
  else if (data._type == UnaryOperatorType::MINUS && data._args[0].is(UnaryOperatorType::MINUS))
    // two nested unary minus cancel
    node._data = data._args[0][0]._data;
}

template <typename T>
void
Simplify<T>::operator()(Node<T> & node, BinaryOperatorData<T> & data)
{
  // simplify children
  data._args[0].apply(*this);
  data._args[1].apply(*this);

  // constant folding
  if (data._args[0].is(NumberType::_ANY) && data._args[1].is(NumberType::_ANY))
  {
    set(node, data.value());
    return;
  }

  switch (data._type)
  {
    case BinaryOperatorType::SUBTRACTION:
      // 0 - b = -b
      if (data._args[0].is(0.0))
        set(node, UnaryOperatorType::MINUS, data._args[1]);
      // a - 0 = a
      else if (data._args[1].is(0.0))
        node._data = data._args[0]._data;
      return;

    case BinaryOperatorType::DIVISION:
      // a/1 = a
      if (data._args[1].is(1.0))
        node._data = data._args[0]._data;
      // 0/b = 0
      else if (data._args[0].is(0.0))
        set(node, 0.0);
      else
      {
        // turn this into a multiplication by the inverse
        set(node,
            MultinaryOperatorType::MULTIPLICATION,
            data._args[0],
            Node<T>(UnaryFunctionType::INVERSE, data._args[1]));
      }
      return;

    case BinaryOperatorType::POWER:
      // turn operator into function and simplify
      set(node, BinaryFunctionType::POW, data._args[0], data._args[1]);
      node.apply(*this);
      return;

    case BinaryOperatorType::MODULO:
      if (data._args[1].is(1.0))
        set(node, 0.0);
      return;

    case BinaryOperatorType::LOGICAL_OR:
      if ((data._args[0].is(NumberType::_ANY) && data._args[0].value() != 0.0) ||
          (data._args[1].is(NumberType::_ANY) && data._args[1].value() != 0.0))
        set(node, 1.0);
      return;

    case BinaryOperatorType::LOGICAL_AND:
      if (data._args[0].is(0.0) || data._args[1].is(0.0))
        set(node, 0.0);
      return;

    default:
      return;
  }
}

template <typename T>
void
Simplify<T>::operator()(Node<T> & node, MultinaryOperatorData<T> & data)
{
  // simplify and hoist children
  std::vector<Node<T>> newargs;
  for (auto & arg : data._args)
  {
    arg.apply(*this);
    if (arg.is(data._type))
    {
      auto arg_data = std::static_pointer_cast<MultinaryOperatorData<T>>(arg._data);
      newargs.insert(newargs.end(), arg_data->_args.begin(), arg_data->_args.end());
    }
    else
      newargs.push_back(arg);
  }
  data._args = newargs;

  switch (data._type)
  {
    case MultinaryOperatorType::ADDITION:
    case MultinaryOperatorType::MULTIPLICATION:
    {
      // sort constant numbers to the end
      std::sort(data._args.begin(), data._args.end(), [](Node<T> & a, Node<T> & b) {
        return !a.is(NumberType::_ANY) && b.is(NumberType::_ANY);
      });
      // find first number node
      auto first_num = std::find_if(
          data._args.begin(), data._args.end(), [](Node<T> & a) { return a.is(NumberType::_ANY); });
      if (first_num == data._args.end())
        return;
      Real val = first_num->value();
      while (--data._args.end() != first_num)
      {
        if (data._type == MultinaryOperatorType::ADDITION)
          val += data._args.back().value();
        else
          val *= data._args.back().value();
        data._args.pop_back();
      }

      if ((val == 1.0 && data._type == MultinaryOperatorType::MULTIPLICATION) ||
          (val == 0.0 && data._type == MultinaryOperatorType::ADDITION))
        data._args.pop_back();
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
Simplify<T>::operator()(Node<T> & node, UnaryFunctionData<T> & data)
{
  // simplify child
  data._args[0].apply(*this);
  if (data._args[0].is(NumberType::_ANY))
    set(node, data.value());
}

template <typename T>
void
Simplify<T>::operator()(Node<T> & node, BinaryFunctionData<T> & data)
{
  // simplify children
  data._args[0].apply(*this);
  data._args[1].apply(*this);

  // constant folding
  if (data._args[0].is(NumberType::_ANY) && data._args[1].is(NumberType::_ANY))
  {
    set(node, data.value());
    return;
  }

  switch (data._type)
  {
    case BinaryFunctionType::POW:
      if (data._args[1].is(1.0))
      {
        node._data = data._args[0]._data;
        return;
      }

      if (data._args[1].is(0.0))
      {
        set(node, 1.0);
        return;
      }

      if (data._args[1].is(0.5))
      {
        set(node, UnaryFunctionType::SQRT, data._args[0]);
        return;
      }

      if (data._args[1].is(1.0 / 3.0))
      {
        set(node, UnaryFunctionType::CBRT, data._args[0]);
        return;
      }

      if (data._args[0].is(2.0))
      {
        set(node, UnaryFunctionType::EXP2, data._args[1]);
        return;
      }

      if (data._args[1].is(NumberType::_ANY))
      {
        auto val = data._args[1].value();
        if (std::floor(val) == val)
        {
          set(node, IntegerPowerType::_ANY, data._args[0], val);
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
Simplify<T>::operator()(Node<T> & node, ConditionalData<T> & data)
{
  if (data._type != ConditionalType::IF)
    fatalError("Conditional not implemented");

  data._args[0].apply(*this);
  data._args[1].apply(*this);
  data._args[2].apply(*this);

  // if the conditional is constant we can drop a branch
  if (data._args[0].is(NumberType::_ANY))
  {
    if (data._args[0].value() != 0.0)
      node._data = data._args[1]._data;
    else
      node._data = data._args[2]._data;
  }
}

template <typename T>
void
Simplify<T>::operator()(Node<T> & node, IntegerPowerData<T> & data)
{
  // (a^b)^c = a^(b*c) (c00^c01) ^ c1 = c00 ^ (c01*c1)
  if (data._arg.is(IntegerPowerType::_ANY))
  {
    set(node, IntegerPowerType::_ANY, data._arg[0], data._arg[1].value() * data._exponent);
    node.apply(*this);
  }
  else if (data._arg.is(NumberType::_ANY))
    set(node, data.value());
  else if (data._exponent == 1)
    node._data = data._arg._data;
  else if (data._exponent == 0)
    set(node, 1.0);
}

template class Simplify<Real>;

} // namespace SymbolicMath
