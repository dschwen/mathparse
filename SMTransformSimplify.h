///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include "SMTransform.h"

namespace SymbolicMath
{

/**
 * Simplification visitor
 */
class Simplify : public Transform
{
public:
  Simplify(FunctionBase & fb);

  void operator()(SymbolData *) override {}

  void operator()(UnaryOperatorData *) override;
  void operator()(BinaryOperatorData *) override;
  void operator()(MultinaryOperatorData *) override;

  void operator()(UnaryFunctionData *) override;
  void operator()(BinaryFunctionData *) override;

  void operator()(RealNumberData *) override {}
  void operator()(RealReferenceData *) override {}
  void operator()(RealArrayReferenceData *) override {}
  void operator()(LocalVariableData *) override {}

  void operator()(ConditionalData *) override;
  void operator()(IntegerPowerData *) override;
};

} // namespace SymbolicMath
