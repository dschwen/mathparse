///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include <stack>

#include "SMCompiler.h"

namespace SymbolicMath
{

/**
 * C source code generation
 */
/**
 * C source code compilation, dynamic object laoding, evaluation
 */
class CompiledCCode : public Compiler
{
public:
  class Source;

  CompiledCCode(FunctionBase &);

  Real operator()() override { return _jit_function(); }

protected:
  typedef Real (*JITFunctionPtr)();

  JITFunctionPtr _jit_function;
};

class CompiledCCode::Source : public Transform
{
public:
  Source(FunctionBase &);

  void operator()(SymbolData *) override;

  void operator()(UnaryOperatorData *) override;
  void operator()(BinaryOperatorData *) override;
  void operator()(MultinaryOperatorData *) override;

  void operator()(UnaryFunctionData *) override;
  void operator()(BinaryFunctionData *) override;

  void operator()(RealNumberData *) override;
  void operator()(RealReferenceData *) override;
  void operator()(RealArrayReferenceData *) override;
  void operator()(LocalVariableData *) override;

  void operator()(ConditionalData *) override;
  void operator()(IntegerPowerData *) override;

  const std::string & operator()() const { return _source; };

protected:
  std::string _source;
};

} // namespace SymbolicMath
