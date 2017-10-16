#ifndef SYMBOLICMATHJITTYPES_H
#define SYMBOLICMATHJITTYPES_H

#ifdef SYMBOLICMATH_USE_SLJIT
#include "SymbolicMathJITTypesSLJIT.h"
#endif

#ifdef SYMBOLICMATH_USE_LIBJIT
#include "SymbolicMathJITTypesLibJIT.h"
#endif

#ifdef SYMBOLICMATH_USE_LIGHTNING
#include "SymbolicMathJITTypesLightning.h"
#endif

#endif // SYMBOLICMATHJITTYPES_H
