#ifndef SYMBOLICMATHJITTYPES_H
#define SYMBOLICMATHJITTYPES_H

#ifdef SYMBOLICMATH_USE_GCCJIT
#include "SymbolicMathJITTypesGCCJIT.h"
#endif

#ifdef SYMBOLICMATH_USE_SLJIT
#include "SymbolicMathJITTypesSLJIT.h"
#endif

#ifdef SYMBOLICMATH_USE_LIBJIT
#include "SymbolicMathJITTypesLibJIT.h"
#endif

#ifdef SYMBOLICMATH_USE_LIGHTNING
#include "SymbolicMathJITTypesLightning.h"
#endif

#ifdef SYMBOLICMATH_USE_CCODE
#include "SymbolicMathJITTypesCCode.h"
#endif

#ifdef SYMBOLICMATH_USE_LLVMIR
#include "SymbolicMathJITTypesLLVMIR.h"
#endif

#endif // SYMBOLICMATHJITTYPES_H
