///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include "SMTransform.h"
#include "SMEvaluable.h"

namespace SymbolicMath
{

/**
 * Compiler transform intermediate base
 */
class Compiler : public Transform, public Evaluable<Real>
{
public:
  Compiler(FunctionBase & fb) : Transform(fb) {}
};

} // namespace SymbolicMath
