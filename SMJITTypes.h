///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#ifdef SYMBOLICMATH_USE_GCCJIT
#include "SMJITTypesGCCJIT.h"
#endif

#ifdef SYMBOLICMATH_USE_SLJIT
#include "SMJITTypesSLJIT.h"
#endif

#ifdef SYMBOLICMATH_USE_LIBJIT
#include "SMJITTypesLibJIT.h"
#endif

#ifdef SYMBOLICMATH_USE_LIGHTNING
#include "SMJITTypesLightning.h"
#endif

#ifdef SYMBOLICMATH_USE_CCODE
#include "SMJITTypesCCode.h"
#endif

#ifdef SYMBOLICMATH_USE_LLVMIR
#include "SMJITTypesLLVMIR.h"
#endif

#ifdef SYMBOLICMATH_USE_VM
#include "SMJITTypesVM.h"
#endif
