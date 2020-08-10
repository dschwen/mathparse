///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

namespace SymbolicMath
{

/**
 * Base class for a function transforming visitor (simplification, derivation)
 */
class CompiledByteCode : public Transform
{
public:
  CompiledByteCode(const FunctionBase &);

  void operator()(SymbolData *) override {}

  void operator()(UnaryOperatorData *) override {}
  void operator()(BinaryOperatorData *) override {}
  void operator()(MultinaryOperatorData *) override {}

  void operator()(UnaryFunctionData *) override {}
  void operator()(BinaryFunctionData *) override {}

  void operator()(RealNumberData *) override {}
  void operator()(RealReferenceData *) {}
  void operator()(RealArrayReferenceData *) {}
  void operator()(LocalVariableData *) override {}

  void operator()(ConditionalData *) override {}
  void operator()(IntegerPowerData *) override {}
};

} // namespace SymbolicMath
