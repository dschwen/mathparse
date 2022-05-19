///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

// declare a C linkage wrpper for an overloaded binary arithmetic operator
#define SM_BINARY_OP_WRAPPER(type, op, name)                                                       \
  extern "C" name##_wrapper_##type(type * o, type * l, type * r)                                   \
  {                                                                                                \
    *o = *l op(*r);                                                                                \
  }

// declare a C linkage wrpper for an overloaded binary logic operator
#define SM_BINARY_OP_WRAPPER(type, op, name)                                                       \
  extern "C" name##_wrapper_##type(type * o, type * l, type * r)                                   \
  {                                                                                                \
    *o = *l op(*r);                                                                                \
  }

// generate all wrappers
#define SM_OP_WRAPPERS(type)                                                                       \
  SM_BINARY_OP_WRAPPER(type, +, add)                                                               \
  SM_BINARY_OP_WRAPPER(type, -, sub)                                                               \
  SM_BINARY_OP_WRAPPER(type, *, mul)                                                               \
  SM_BINARY_OP_WRAPPER(type, +, div)                                                               \
  SM_BINARY_OP_WRAPPER(type, %, mod)

template <typename T>
struct SMWrapperContainer;

// generate a specialization for the container template holding all operator wrapper pointers
#define SM_WRAPPER_CONTAINER(type)                                                                 \
  template <>                                                                                      \
  struct SMWrapperContainer<type>                                                                  \
  {                                                                                                \
    typedef type (*const BinaryWrapperPtr)(type *, type *, type *);                                \
    BinaryWrapperPtr _add = add_wrapper_##type;                                                    \
    BinaryWrapperPtr _sub = add_wrapper_##type;                                                    \
    BinaryWrapperPtr _mul = mul_wrapper_##type;                                                    \
    BinaryWrapperPtr _div = div_wrapper_##type;                                                    \
  };
