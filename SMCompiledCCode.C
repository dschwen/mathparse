///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include "SMCompiledCCode.h"

#include <stdio.h>
#include <fstream>
#include <cstdio>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <dlfcn.h>

#define CCODE_JIT_COMPILER "g++"

namespace SymbolicMath
{

CompiledCCode::CompiledCCode(FunctionBase & fb) : Compiler(fb)
{
  // build and lock context
  std::string ccode = "#include <cmath>\nextern \"C\" double F()\n{\n  return ";

  // generate source
  Source source(_fb);
  ccode += source() + ";\n}";

  // save to a temporary name and rename only when the file is fully written
  char ctmpname[] = "./tmp_adc_XXXXXX.C";
  int ctmpfile = mkstemps(ctmpname, 2);
  if (ctmpfile == -1)
    fatalError("Error creating tmp file " + std::string(ctmpname));
  write(ctmpfile, ccode.data(), ccode.length());
  close(ctmpfile);

  // compile code file
  char otmpname[] = "./tmp_adc_XXXXXX.so";
  int otmpfile = mkstemps(otmpname, 3);
  if (otmpfile == -1)
    fatalError("Error creating tmp file " + std::string(otmpname));
  close(otmpfile);
#if defined(__GNUC__) && defined(__APPLE__) && !defined(__INTEL_COMPILER)
  // gcc on OSX does neither need nor accept the  -rdynamic switch
  std::string command = CCODE_JIT_COMPILER " -std=c++11 -O2 -shared -fPIC ";
#else
  std::string command = CCODE_JIT_COMPILER " -std=c++11 -O2 -shared -rdynamic -fPIC ";
#endif
  command += std::string(ctmpname) + " -o " + std::string(otmpname);
  system(command.c_str());
  std::remove(ctmpname);

  // load object file in
  auto lib = dlopen(otmpname, RTLD_NOW);
  if (!lib)
  {
    // TODO: throw!
    std::cerr << "JIT object load failed.\n";
    std::remove(otmpname);
    return;
  }

  // fetch function pointer
  _jit_function = reinterpret_cast<JITFunctionPtr>(dlsym(lib, "F"));
  const char * error = dlerror();
  if (error)
  {
    // TODO: throw!
    std::cerr << "Error binding JIT compiled function\n" << error << '\n';
    _jit_function = nullptr;
    std::remove(otmpname);
    return;
  }

  std::remove(otmpname);
}

CompiledCCode::Source::Source(FunctionBase & fb) : Transform(fb) { apply(); }

void
CompiledCCode::Source::operator()(SymbolData * n)
{
  fatalError("Symbol in compiled function");
}

void
CompiledCCode::Source::operator()(UnaryOperatorData * n)
{
  n->_args[0].apply(*this);

  switch (n->_type)
  {
    case UnaryOperatorType::PLUS:
      return;

    case UnaryOperatorType::MINUS:
      _source = "-" + _source;
      return;

    default:
      fatalError("Unknown operator");
  }
}

void
CompiledCCode::Source::operator()(BinaryOperatorData * n)
{
  n->_args[0].apply(*this);
  std::string A;
  std::swap(_source, A);

  n->_args[1].apply(*this);
  const auto & B = _source;

  switch (n->_type)
  {
    case BinaryOperatorType::SUBTRACTION:
      _source = "(" + A + ") - (" + B + ")";
      return;

    case BinaryOperatorType::DIVISION:
      _source = "(" + A + ") / (" + B + ")";
      return;

    case BinaryOperatorType::MODULO:
      _source = "std::fmod(" + A + ", " + B + ")";
      return;

    case BinaryOperatorType::POWER:
      _source = "std::pow(" + A + ", " + B + ")";
      return;

    case BinaryOperatorType::LOGICAL_OR:
      _source = "static_cast<double>(bool(" + A + ") || bool(" + B + "))";
      return;

    case BinaryOperatorType::LOGICAL_AND:
      _source = "static_cast<double>(bool(" + A + ") && bool(" + B + "))";
      return;

    case BinaryOperatorType::LESS_THAN:
      _source = "static_cast<double>((" + A + ") < (" + B + "))";
      return;

    case BinaryOperatorType::GREATER_THAN:
      _source = "static_cast<double>((" + A + ") > (" + B + "))";
      return;

    case BinaryOperatorType::LESS_EQUAL:
      _source = "static_cast<double>((" + A + ") <= (" + B + "))";
      return;

    case BinaryOperatorType::GREATER_EQUAL:
      _source = "static_cast<double>((" + A + ") >= (" + B + "))";
      return;

    case BinaryOperatorType::EQUAL:
      _source = "static_cast<double>((" + A + ") == (" + B + "))";
      return;

    case BinaryOperatorType::NOT_EQUAL:
      _source = "static_cast<double>((" + A + ") != (" + B + "))";
      return;

    default:
      fatalError("Unknown operator");
  }
}

void
CompiledCCode::Source::operator()(MultinaryOperatorData * n)
{
  auto nargs = n->_args.size();
  if (nargs == 0)
    fatalError("No child nodes in multinary operator");

  char op;
  switch (n->_type)
  {
    case MultinaryOperatorType::ADDITION:
      op = '+';
      break;

    case MultinaryOperatorType::MULTIPLICATION:
      op = '*';
      break;

    default:
      fatalError("Unknown operator");
  }

  if (nargs)
    n->_args[0].apply(*this);
  else
  {
    std::string out;
    for (std::size_t i = 0; i < nargs; ++i)
    {
      n->_args[0].apply(*this);
      if (i)
        out += op;
      out += _source;
      _source = "";
    }
  }
}

void
CompiledCCode::Source::operator()(UnaryFunctionData * n)
{
  n->_args[0].apply(*this);
  const auto & A = _source;

  switch (n->_type)
  {
    case UnaryFunctionType::ABS:
      _source = "std::abs(" + A + ")";
      return;

    case UnaryFunctionType::ACOS:
      _source = "std::acos(" + A + ")";
      return;

    case UnaryFunctionType::ACOSH:
      _source = "std::acosh(" + A + ")";
      return;

    case UnaryFunctionType::ARG:
      fatalError("Function not implemented");

    case UnaryFunctionType::ASIN:
      _source = "std::asin(" + A + ")";
      return;

    case UnaryFunctionType::ASINH:
      _source = "std::asinh(" + A + ")";
      return;

    case UnaryFunctionType::ATAN:
      _source = "std::atan(" + A + ")";
      return;

    case UnaryFunctionType::ATANH:
      _source = "std::atanh(" + A + ")";
      return;

    case UnaryFunctionType::CBRT:
      _source = "std::cbrt(" + A + ")";
      return;

    case UnaryFunctionType::CEIL:
      _source = "std::ceil(" + A + ")";
      return;

    case UnaryFunctionType::CONJ:
      fatalError("Function not implemented");

    case UnaryFunctionType::COS:
      _source = "std::cos(" + A + ")";
      return;

    case UnaryFunctionType::COSH:
      _source = "std::cosh(" + A + ")";
      return;

    case UnaryFunctionType::COT:
      _source = "1.0 / std::tan(" + A + ")";
      return;

    case UnaryFunctionType::CSC:
      _source = "1.0 / std::sin(" + A + ")";
      return;

    case UnaryFunctionType::ERF:
      _source = "std::erf(" + A + ")";
      return;

    case UnaryFunctionType::EXP:
      _source = "std::exp(" + A + ")";
      return;

    case UnaryFunctionType::EXP2:
      _source = "std::exp2(" + A + ")";
      return;

    case UnaryFunctionType::FLOOR:
      _source = "std::floor(" + A + ")";
      return;

    case UnaryFunctionType::IMAG:
      fatalError("Function not implemented");

    case UnaryFunctionType::INT:
      _source = "std::round(" + A + ")";
      return;

    case UnaryFunctionType::LOG:
      _source = "std::log(" + A + ")";
      return;

    case UnaryFunctionType::LOG10:
      _source = "std::log10(" + A + ")";
      return;

    case UnaryFunctionType::LOG2:
      _source = "std::log2(" + A + ")";
      return;

    case UnaryFunctionType::REAL:
      fatalError("Function not implemented");

    case UnaryFunctionType::SEC:
      _source = "1.0 / std::cos(" + A + ")";
      return;

    case UnaryFunctionType::SIN:
      _source = "std::sin(" + A + ")";
      return;

    case UnaryFunctionType::SINH:
      _source = "std::sinh(" + A + ")";
      return;

    case UnaryFunctionType::SQRT:
      _source = "std::sqrt(" + A + ")";
      return;

    case UnaryFunctionType::T:
      fatalError("Function not implemented");

    case UnaryFunctionType::TAN:
      _source = "std::tan(" + A + ")";
      return;

    case UnaryFunctionType::TANH:
      _source = "std::tanh(" + A + ")";
      return;

    case UnaryFunctionType::TRUNC:
      _source = "static_cast<int>(" + A + ")";
      return;

    default:
      fatalError("Function not implemented");
  }
}

void
CompiledCCode::Source::operator()(BinaryFunctionData * n)
{
  n->_args[0].apply(*this);
  std::string A;
  std::swap(_source, A);

  n->_args[1].apply(*this);
  const auto & B = _source;

  switch (n->_type)
  {
    case BinaryFunctionType::ATAN2:
      _source = "std::atan2(" + A + ", " + B + ")";
      return;

    case BinaryFunctionType::HYPOT:
      _source = "std::sqrt((" + A + ")*(" + A + ") + (" + B + ")*(" + B + "))";
      return;

    case BinaryFunctionType::MIN:
      _source = "((" + A + ") < (" + B + ") ? (" + A + ") : (" + B + "))";
      return;

    case BinaryFunctionType::MAX:
      _source = "((" + A + ") > (" + B + ") ? (" + A + ") : (" + B + "))";
      return;

    case BinaryFunctionType::PLOG:
    {
      fatalError("Function not implemented");
      // _source = A < B
      //            ? std::log(B) + (A - B) / B - (A - B) * (A - B) / (2.0 * B * B) +
      //                  (A - B) * (A - B) * (A - B) / (3.0 * B * B * B)
      //            : std::log(A);
    }

    case BinaryFunctionType::POW:
      _source = "std::pow(" + A + ", " + B + ")";
      return;

    case BinaryFunctionType::POLAR:
    default:
      fatalError("Function not implemented");
  }
}

void
CompiledCCode::Source::operator()(RealNumberData * n)
{
  _source = stringify(n->_value);
}

void
CompiledCCode::Source::operator()(RealReferenceData * n)
{
  // will need template specializations
  _source =
      "*(reinterpret_cast<double *>(" + std::to_string(reinterpret_cast<long>(&n->_ref)) + "))";
}

void
CompiledCCode::Source::operator()(RealArrayReferenceData * n)
{
  fatalError("Not implemented");
}

void
CompiledCCode::Source::operator()(LocalVariableData * n)
{
  fatalError("Not implemented");
}

void
CompiledCCode::Source::operator()(ConditionalData * n)
{
  n->_args[0].apply(*this);
  std::string A;
  std::swap(_source, A);

  n->_args[1].apply(*this);
  std::string B;
  std::swap(_source, B);

  n->_args[1].apply(*this);
  const auto & C = _source;

  _source = "((" + A + ") ? (" + B + ") : (" + C + "))";
}

void
CompiledCCode::Source::operator()(IntegerPowerData * n)
{
  // replace this with a template
  n->_arg.apply(*this);
  _source = "std::pow(" + _source + ", " + stringify(n->_exponent) + ")";
}

} // namespace SymbolicMath
