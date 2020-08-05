///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

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

#ifdef SYMBOLICMATH_USE_VM
#include "SymbolicMathJITTypesVM.h"
#endif
