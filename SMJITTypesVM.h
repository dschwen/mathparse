///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include "SMSymbols.h"
#include <utility>
#include <vector>

namespace SymbolicMath
{

const std::string jit_backend_name = "Byte code VM";

enum class VMInstruction
{
  LOAD_IMMEDIATE_INTEGER,
  LOAD_IMMEDIATE_REAL,
  LOAD_VARIABLE_REAL,
  UNARY_OPERATOR,
  BINARY_OPERATOR,
  MULTINARY_PLUS,
  MULTINARY_MULTIPLY,
  UNARY_FUNCTION,
  BINARY_FUNCTION,
  CONDITIONAL,
  INTEGER_POWER,
  JUMP
};

struct VMData
{
  VMData(int value) : _int_value(value) {}
  VMData(Real value) : _value(value) {}
  VMData(const Real * variable) : _variable(variable) {}
  VMData(UnaryOperatorType unary_operator) : _unary_operator(unary_operator) {}
  VMData(BinaryOperatorType binary_operator) : _binary_operator(binary_operator) {}
  VMData(UnaryFunctionType unary_function) : _unary_function(unary_function) {}
  VMData(BinaryFunctionType binary_function) : _binary_function(binary_function) {}

  union {
    int _int_value;
    Real _value;
    const Real * _variable;
    UnaryOperatorType _unary_operator;
    BinaryOperatorType _binary_operator;
    UnaryFunctionType _unary_function;
    BinaryFunctionType _binary_function;
  };
};

using VMItem = std::pair<VMInstruction, VMData>;

typedef Real (*JITFunctionPtr)();

using ByteCode = std::vector<VMItem>;
using JITReturnValue = void;
using JITStateValue = ByteCode;

} // namespace SymbolicMath
