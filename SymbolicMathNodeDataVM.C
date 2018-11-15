#ifdef SYMBOLICMATH_USE_VM

#include "SymbolicMathNodeData.h"
#include "SymbolicMathUtils.h"

#include <cmath>
#include <iostream> // debug

namespace SymbolicMath
{

/********************************************************
 * Real Number immediate
 ********************************************************/

JITReturnValue
RealNumberData::jit(JITStateValue & func)
{
  func.emplace_back(VMInstruction::LOAD_IMMEDIATE_REAL, _value);
}

/********************************************************
 * Real Number reference value provider
 ********************************************************/

JITReturnValue
RealReferenceData::jit(JITStateValue & func)
{
  func.emplace_back(VMInstruction::LOAD_VARIABLE_REAL, &_ref);
}

/********************************************************
 * Real Number Array reference value provider
 ********************************************************/

JITReturnValue
RealArrayReferenceData::jit(JITStateValue & func)
{
  fatalError("Not implemented");
}

/********************************************************
 * Unary Operator
 ********************************************************/

JITReturnValue
UnaryOperatorData::jit(JITStateValue & func)
{
  _args[0].jit(func);
  func.emplace_back(VMInstruction::UNARY_OPERATOR, _type);
}

/********************************************************
 * Binary Operator Node
 ********************************************************/

JITReturnValue
BinaryOperatorData::jit(JITStateValue & func)
{
  _args[0].jit(func);
  _args[1].jit(func);
  func.emplace_back(VMInstruction::BINARY_OPERATOR, _type);
}

/********************************************************
 * Multinary Operator Node
 ********************************************************/

JITReturnValue
MultinaryOperatorData::jit(JITStateValue & func)
{
  const int nargs = static_cast<int>(_args.size());

  if (nargs == 0)
    fatalError("No child nodes in multinary operator");

  for (auto arg : _args)
    arg.jit(func);

  if (nargs == 1)
    return;

  switch (_type)
  {
    case MultinaryOperatorType::ADDITION:
      func.emplace_back(VMInstruction::MULTINARY_PLUS, nargs);
      break;

    case MultinaryOperatorType::MULTIPLICATION:
      func.emplace_back(VMInstruction::MULTINARY_MULTIPLY, nargs);
      break;

    default:
      fatalError("Unknown operator");
  }
}

/********************************************************
 * Unary Function Node
 ********************************************************/

JITReturnValue
UnaryFunctionData::jit(JITStateValue & func)
{
  _args[0].jit(func);
  func.emplace_back(VMInstruction::UNARY_FUNCTION, _type);
}

/********************************************************
 * Binary Function Node
 ********************************************************/

JITReturnValue
BinaryFunctionData::jit(JITStateValue & func)
{
  _args[0].jit(func);
  _args[1].jit(func);
  func.emplace_back(VMInstruction::BINARY_FUNCTION, _type);
}

/********************************************************
 * Conditional Node
 ********************************************************/

JITReturnValue
ConditionalData::jit(JITStateValue & func)
{
  _args[0].jit(func);
  const auto conditional_ip = func.size();
  func.emplace_back(VMInstruction::CONDITIONAL, 0);
  // true branch
  _args[1].jit(func);
  // jump past false at the end of the true branch
  const auto jump_past_false_ip = func.size();
  func.emplace_back(VMInstruction::JUMP, 0);
  // set jump to false ip on conditional instruction
  func[conditional_ip].second._int_value = func.size();
  // false branch
  _args[2].jit(func);
  // set jump past false target
  func[jump_past_false_ip].second._int_value = func.size();
}

/********************************************************
 * Integer power Node
 ********************************************************/

JITReturnValue
IntegerPowerData::jit(JITStateValue & func)
{
  _arg.jit(func);
  func.emplace_back(VMInstruction::INTEGER_POWER, _exponent);
}

} // namespace SymbolicMath

#endif // SYMBOLICMATH_USE_VM
