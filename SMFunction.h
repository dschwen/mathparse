///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#ifdef SYMBOLICMATH_USE_GCCJIT
#include "SMFunctionGCCJIT.h"
#endif

#ifdef SYMBOLICMATH_USE_SLJIT
#include "SMFunctionSLJIT.h"
#endif

#ifdef SYMBOLICMATH_USE_LIBJIT
#include "SMFunctionLibJIT.h"
#endif

#ifdef SYMBOLICMATH_USE_LIGHTNING
#include "SMFunctionLightning.h"
#endif

#ifdef SYMBOLICMATH_USE_CCODE
#include "SMFunctionCCode.h"
#endif

#ifdef SYMBOLICMATH_USE_LLVMIR
#include "SMFunctionLLVMIR.h"
#endif

#ifdef SYMBOLICMATH_USE_VM
#include "SMFunctionVM.h"
#endif
