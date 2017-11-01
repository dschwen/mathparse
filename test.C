#include "SymbolicMath.h"
#include "SymbolicMathFunction.h"
#include "SymbolicMathHelpers.h"
#include "SymbolicMathJITTypes.h"

#include <iostream>

struct Test
{
  std::string expression;
  std::function<double(double)> native;
};

// clang-format off
const std::vector<Test> tests = {
  // obvious simplifcations
  {"c/1.0", [](double c) { return c / 1.0; }},
  {"c+0", [](double c) { return c + 0.0; }},
  {"c*0", [](double c) { return c * 0.0; }},
  {"0/c", [](double c) { return 0.0 / c; }},
  // expressions
  {"1 + c + 2*c + 3*c^3", [](double c) { return 1 + c + 2*c + 3*c*c*c; }},
  {"c + sin(1.1)", [](double c) { return c + std::sin(1.1); }},
  // mathematical functions
  {"sin(c)", [](double c) { return std::sin(c); }},
  {"cos(c)", [](double c) { return std::cos(c); }},
  {"tan(c)", [](double c) { return std::tan(c); }},
  {"csc(c)", [](double c) { return 1.0 / std::sin(c); }},
  {"sec(c)", [](double c) { return 1.0 / std::cos(c); }},
  {"cot(c)", [](double c) { return 1.0 / std::tan(c); }},
  {"erf(c)", [](double c) { return std::erf(c); }},
  {"exp(c)", [](double c) { return std::exp(c); }},
  {"atan2(c,0.5)", [](double c) { return std::atan2(c, 0.5); }},
  {"atan2(0.5,c)", [](double c) { return std::atan2(0.5, c); }},
  {"atan2(2*c, 3*c)", [](double c) { return std::atan2(2*c, 3*c); }},
  {"int(c)", [](double c) { return std::round(c); }},
  {"floor(c)", [](double c) { return std::floor(c); }},
  {"ceil(c)", [](double c) { return std::ceil(c); }},
  {"trunc(c)", [](double c) { return static_cast<int>(c); }},
  // powers and their simplifications
  {"pow(c,3)", [](double c) { return std::pow(c, 3); }},
  {"pow(c,-3)", [](double c) { return std::pow(c, -3); }},
  {"pow(c,3.5)", [](double c) { return std::pow(c, 3.5); }},
  {"pow(pow(c,3.5),2)", [](double c) { return std::pow(std::pow(c, 3.5), 2.0); }},
  {"pow(pow(c,3.5),2.5)", [](double c) { return std::pow(std::pow(c, 3.5), 2.5); }},
  {"pow(c,1)", [](double c) { return std::pow(c, 1); }},
  // comparison operators
  {"c<0.2", [](double c) { return c<0.2; }},
  {"c>0.2", [](double c) { return c>0.2; }},
  {"c<=0.2", [](double c) { return c<=0.2; }},
  // {"c<=-0.7", [](double c) { return c<=-0.7; }}, // fails
  {"c>=0.2", [](double c) { return c>=0.2; }},
  {"c==0.2", [](double c) { return c==0.2; }},
  {"c!=0.2", [](double c) { return c!=0.2; }},
  // constant comparisons
  {"1<2", [](double c) { return 1<2; }},
  {"1>2", [](double c) { return 1>2; }},
  {"1<2", [](double c) { return 1<=2; }},
  {"1>2", [](double c) { return 1>=2; }},
  {"1==2", [](double c) { return 1==2; }},
  {"1!=2", [](double c) { return 1!=2; }},
  // logical operators
  {"c<0.5 & c>-0.5", [](double c) { return c<0.5 && c>-0.5; }},
  {"c>0.5 | c<-0.5", [](double c) { return c>0.5 || c<-0.5; }},
  {"c>0.5 | c<-0.5 | (c<0.3 & c > -0.1)", [](double c) { return c>0.5 || c<-0.5 || (c<0.3 && c > -0.1); }},
  // logical operators with  non 0/1 constants
  {"c<0.2 & 2.0", [](double c) { return c<0.2 && static_cast<bool>(2.0); }},
  {"c<0.2 | 2.0", [](double c) { return c<0.2 || static_cast<bool>(2.0); }},
  {"c<0.2 & 0.0", [](double c) { return c<0.2 && static_cast<bool>(0.0); }},
  {"c<0.2 | 0.0", [](double c) { return c<0.2 || static_cast<bool>(0.0); }},
  // conditional (if)
  {"if(c<0.15, 10, 20)", [](double c) { return c < 0.15 ? 10 : 20; }},
  // nested if
  {"if(c<-0.5, 10, if(c>0.2, 20, 30))", [](double c) { return c <= -0.5 ? 10 : (c > 0.2 ? 20 : 30); }},
};
// clang-format on

struct DiffTest
{
  std::string expression;
  SymbolicMath::Real cmin, cmax, dc, epsilon;
  SymbolicMath::Real tol;
};

// clang-format off
const std::vector<DiffTest> difftests = {
  {"c", -10, 10, 0.7, 0.1, 1e-9},
  {"abs(c)", -10, 10, 0.7, 0.1, 1e-9},
  {"sin(c)", -4, 4, 0.1, 1e-8, 1e-8},
  {"cos(c)", -4, 4, 0.1, 1e-8, 1e-8},
  {"tan(c)", -4, 4, 0.11, 1e-9, 2e-7},
  {"csc(c)", -0.99, 0.99, 0.1, 1e-9, 1e-7},
  {"sec(c)", -0.99, 0.99, 0.1, 1e-9, 1e-7},
  {"cot(c)", -0.99, 0.99, 0.1, 1e-9, 1e-7},
  {"sinh(c)", -5, 5, 0.1, 1e-9, 1e-7},
  {"cosh(c)", -5, 5, 0.1, 1e-9, 1e-7},
  {"tanh(c)", -5, 5, 0.2, 1e-9, 1e-6},
  {"asin(c)", -0.99, 0.99, 0.1, 1e-9, 1e-7},
  {"acos(c)", -0.99, 0.99, 0.1, 1e-9, 1e-7},
  {"atan(c)", -5, 5, 0.1, 1e-9, 1e-6},
  {"asinh(c)", -5, 5, 0.1, 1e-9, 1e-6},
  {"acosh(c)", 1.01, 5, 0.1, 1e-9, 2e-7},
  {"atanh(c)", -0.99, 0.99, 0.1, 1e-9, 1e-7},
  {"cbrt(c)", 0.01, 5, 0.1, 1e-9, 2e-7},
  {"erf(c)", -5, 5, 0.1, 1e-9, 3e-7},
  {"log(c)", 0.01, 5, 0.1, 1e-9, 2e-7},
  // {"log2(c)", 0.01, 5, 0.1, 1e-9, 2e-7},
  // {"log10(c)", 0.01, 5, 0.1, 1e-9, 2e-7},
};
// clang-format on

int
main(int argc, char * argv[])
{
  double norm;
  int total = 0, fail = 0;

  // test direct evaluation, simplification, and compilation

  for (auto & test : tests)
  {
    SymbolicMath::Parser parser;

    SymbolicMath::Real c;
    auto c_var = std::make_shared<SymbolicMath::RealReferenceData>(c, "c");
    parser.registerValueProvider(c_var);

    auto func = parser.parse(test.expression);

    // evaluate for various values of c
    norm = 0.0;
    for (c = -1.0; c <= 1.0; c += 0.3)
      norm += std::abs(func.value() - test.native(c));
    if (norm > 1e-9)
    {
      std::cerr << "Error evaluating unsimplified expression '" << test.expression << "'\n";
      fail++;
    }

    func.simplify();

    // evaluate for various values of c
    norm = 0.0;
    for (c = -1.0; c <= 1.0; c += 0.3)
      norm += std::abs(func.value() - test.native(c));
    if (norm > 1e-9)
    {
      std::cerr << "Error evaluating expression '" << test.expression << "' simplified to '"
                << func.format() << "'\n";
      fail++;
    }

    func.compile();

    if (!func.isCompiled())
    {
      std::cerr << "Error compiling expression '" << test.expression << "' simplified to '"
                << func.format() << "'\n";
      fail++;
    }

    // evaluate for various values of c
    norm = 0.0;
    for (c = -1.0; c <= 1.0; c += 0.3)
      norm += std::abs(func.value() - test.native(c));
    if (norm > 1e-9)
    {
      std::cerr << "Error " << norm << "evaluating compiled expression '" << test.expression
                << "' simplified to '" << func.format() << "'\n";
      fail++;
    }

    total += 3;
  }

  // test derivatives
  for (auto & test : difftests)
  {
    SymbolicMath::Parser parser;

    SymbolicMath::Real c;
    auto c_var = std::make_shared<SymbolicMath::RealReferenceData>(c, "c");
    parser.registerValueProvider(c_var);

    auto func = parser.parse(test.expression);
    func.simplify();
    func.compile();

    auto diff = func.D(c_var);
    diff.simplify();
    diff.compile();
    if (!diff.isCompiled())
    {
      std::cerr << "Error compiling derivative of '" << test.expression << "' simplified to '"
                << diff.format() << "'\n";
      fail++;
    }

    // finite differencing
    norm = 0.0;
    SymbolicMath::Real abssum = 0.0;
    for (c = test.cmin; c <= test.cmax; c += test.dc)
    {
      auto a = func.value();
      c += test.epsilon;
      auto b = func.value();
      c -= test.epsilon;

      auto d = diff.value();
      abssum += std::abs(d);

      norm += std::abs(d - (b - a) / test.epsilon);
    }
    // for debug purposes (to set tolerances for new tests)
    std::cerr << norm / abssum << '\t' << test.expression << '\n';
    if (norm / abssum > test.tol)
    {
      std::cerr << "Derivative does not match finite differencing for '" << test.expression
                << "' and the derivative '" << diff.format() << "' " << norm << "\n";
      fail++;
    }

    total += 2;
  }

  // Final output

  if (fail)
  {
    std::cout << "\033[1;31m"; // red
    std::cout << (total - fail) << " tests PASSED, " << fail << " tests FAILED!\n";
  }
  else
  {
    std::cout << "\033[1;32m"; // green
    std::cout << "ALL " << total << " tests PASSED.\n";
  }

  std::cout << "\033[0m"; // reset color

  return fail ? 1 : 0;
}
