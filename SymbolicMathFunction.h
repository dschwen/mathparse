#ifndef SYMBOLICMATHFUNCTION_H
#define SYMBOLICMATHFUNCTION_H

#ifdef SYMBOLICMATH_USE_SLJIT
#include "SymbolicMathFunctionSLJIT.h"
#endif

#ifdef SYMBOLICMATH_USE_LIBJIT
#include "SymbolicMathFunctionLibJIT.h"
#endif

#endif // SYMBOLICMATHFUNCTION_H
