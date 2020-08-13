///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include <memory>
#include <algorithm>

#include "SMNodeData.h"
#include "SMFunctionBase.h"
#include "SMTransformSimplify.h"

namespace SymbolicMath
{

Simplify::Simplify(FunctionBase & fb) : Transform(fb) { apply(); }

void
Simplify::operator()(UnaryOperatorData * n)
{
  // simplify child
  n->_args[0].apply(*this);
  if (n->_args[0].is(NumberType::_ANY))
    set(n->value());
}

void
Simplify::operator()(BinaryOperatorData * n)
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

void
Simplify::operator()(MultinaryOperatorData * n)
{
  // simplify and hoist children
  std::vector<Node> newargs;
  for (auto & arg : n->_args)
  {
    arg.apply(*this);
    if (arg.is(n->_type))
    {
      auto arg_data = std::static_pointer_cast<MultinaryOperatorData>(arg._data);
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
      std::sort(n->_args.begin(), n->_args.end(), [](Node & a, Node & b) {
        return !a.is(NumberType::_ANY) && b.is(NumberType::_ANY);
      });
      // find first number node
      auto first_num = std::find_if(
          n->_args.begin(), n->_args.end(), [](Node & a) { return a.is(NumberType::_ANY); });
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
      first_num->_data = std::make_shared<RealNumberData>(val);
      return;
    }

    default:
      return;
  }
}

void
Simplify::operator()(UnaryFunctionData * n)
{
  // simplify child
  n->_args[0].apply(*this);
  if (n->_args[0].is(NumberType::_ANY))
    set(n->value());
}

void
Simplify::operator()(BinaryFunctionData * n)
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
}

void
Simplify::operator()(ConditionalData * n)
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

void
Simplify::operator()(IntegerPowerData * n)
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

} // namespace SymbolicMath
