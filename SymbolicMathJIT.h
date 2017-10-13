#ifndef SYMBOLICMATHJIT_H
#define SYMBOLICMATHJIT_H

#include "SymbolicMathUtils.h"
#include "contrib/sljit_src/sljitLir.h"
#include <cmath>

namespace SymbolicMath
{

#define GLUE_HELPER(x, y) x##y
#define GLUE(x, y) GLUE_HELPER(x, y)

#define SLJIT_MATH_WRAPPER1(FUNC)                                                                  \
  static long SLJIT_CALL GLUE(sljit_wrap_, FUNC)(long a)                                           \
  {                                                                                                \
    auto & A = *reinterpret_cast<double *>(a);                                                     \
    A = std::FUNC(A);                                                                              \
    return 0;                                                                                      \
  }

#define SLJIT_MATH_WRAPPER1X(FUNC, EXPR)                                                           \
  static long SLJIT_CALL GLUE(sljit_wrap_, FUNC)(long a)                                           \
  {                                                                                                \
    auto & A = *reinterpret_cast<double *>(a);                                                     \
    A = EXPR;                                                                                      \
    return 0;                                                                                      \
  }

#define SLJIT_MATH_WRAPPER2(FUNC)                                                                  \
  static long SLJIT_CALL GLUE(sljit_wrap_, FUNC)(long a, long b)                                   \
  {                                                                                                \
    auto & B = *reinterpret_cast<double *>(b);                                                     \
    auto & A = *reinterpret_cast<double *>(a);                                                     \
    A = std::FUNC(A, B);                                                                           \
    return 0;                                                                                      \
  }

SLJIT_MATH_WRAPPER1(abs)
SLJIT_MATH_WRAPPER1(acos)
SLJIT_MATH_WRAPPER1(acosh)
SLJIT_MATH_WRAPPER1(asin)
SLJIT_MATH_WRAPPER1(asinh)
SLJIT_MATH_WRAPPER1(atan)
SLJIT_MATH_WRAPPER1(atanh)
SLJIT_MATH_WRAPPER1(cbrt)
SLJIT_MATH_WRAPPER1(ceil)
SLJIT_MATH_WRAPPER1(cos)
SLJIT_MATH_WRAPPER1(cosh)
SLJIT_MATH_WRAPPER1(erf)
SLJIT_MATH_WRAPPER1(exp)
SLJIT_MATH_WRAPPER1(exp2)
SLJIT_MATH_WRAPPER1(floor)
SLJIT_MATH_WRAPPER1(log)
SLJIT_MATH_WRAPPER1(log10)
SLJIT_MATH_WRAPPER1(log2)
SLJIT_MATH_WRAPPER1(sin)
SLJIT_MATH_WRAPPER1(sinh)
SLJIT_MATH_WRAPPER1(sqrt)
SLJIT_MATH_WRAPPER1(tan)
SLJIT_MATH_WRAPPER1(tanh)
SLJIT_MATH_WRAPPER1X(sec, 1.0 / std::cos(A))
SLJIT_MATH_WRAPPER1X(csc, 1.0 / std::sin(A))
SLJIT_MATH_WRAPPER1X(cot, 1.0 / std::tan(A))
SLJIT_MATH_WRAPPER1X(int, A < 0 ? std::ceil(A - 0.5) : std::floor(A + 0.5))
SLJIT_MATH_WRAPPER1X(trunc, static_cast<int>(A))

SLJIT_MATH_WRAPPER2(atan2)
SLJIT_MATH_WRAPPER2(max)
SLJIT_MATH_WRAPPER2(min)
SLJIT_MATH_WRAPPER2(pow)

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHJIT_H
