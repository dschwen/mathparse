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
  {"sin(c)", [](double c) { return std::sin(c); }},
  {"cos(c)", [](double c) { return std::cos(c); }},
  {"tan(c)", [](double c) { return std::tan(c); }},
  {"csc(c)", [](double c) { return 1.0 / std::sin(c); }},
  {"sec(c)", [](double c) { return 1.0 / std::cos(c); }},
  {"cot(c)", [](double c) { return 1.0 / std::tan(c); }},
  {"erf(c)", [](double c) { return std::erf(c); }},
  {"exp(c)", [](double c) { return std::exp(c); }},
  {"pow(c,3)", [](double c) { return std::pow(c, 3); }},
  {"pow(c,3.5)", [](double c) { return std::pow(c, 3.5); }},
  {"pow(pow(c,3.5),2)", [](double c) { return std::pow(std::pow(c, 3.5), 2.0); }},
  {"pow(pow(c,3.5),2.5)", [](double c) { return std::pow(std::pow(c, 3.5), 2.5); }},
  {"pow(c,1)", [](double c) { return std::pow(c, 1); }},
  {"c/1.0", [](double c) { return c / 1.0; }},
  {"c+0", [](double c) { return c + 0.0; }},
  {"c*0", [](double c) { return c * 0.0; }},
  {"0/c", [](double c) { return 0.0 / c; }},
  {"1 + c + 2*c + 3*c^3", [](double c) { return 1 + c + 2*c + 3*c*c*c; }},
  {"c + sin(1.1)", [](double c) { return c + std::sin(1.1); }},
  {"atan2(c,0.5)", [](double c) { return std::atan2(c, 0.5); }},
  {"atan2(0.5,c)", [](double c) { return std::atan2(0.5, c); }},
  {"atan2(2*c, 3*c)", [](double c) { return std::atan2(2*c, 3*c); }},
  {"int(c)", [](double c) { return std::round(c); }},
  {"floor(c)", [](double c) { return std::floor(c); }},
  {"ceil(c)", [](double c) { return std::ceil(c); }},
  {"trunc(c)", [](double c) { return static_cast<int>(c); }},
  {"c<0.2", [](double c) { return c<0.2; }},
  {"c>0.2", [](double c) { return c>0.2; }},
  {"c<=0.2", [](double c) { return c<=0.2; }},
  {"c>=0.2", [](double c) { return c>=0.2; }},
  {"c==0.2", [](double c) { return c==0.2; }},
  {"c!=0.2", [](double c) { return c!=0.2; }},
  {"1<2", [](double c) { return 1<2; }},
  {"1>2", [](double c) { return 1>2; }},
  {"1<2", [](double c) { return 1<=2; }},
  {"1>2", [](double c) { return 1>=2; }},
  {"1==2", [](double c) { return 1==2; }},
  {"1!=2", [](double c) { return 1!=2; }},
  {"c<0.5 & c>-0.5", [](double c) { return c<0.5 && c>-0.5; }},
  {"c>0.5 | c<-0.5", [](double c) { return c>0.5 || c<-0.5; }},
  {"c>0.5 | c<-0.5 | (c<0.3 & c > -0.1)", [](double c) { return c>0.5 || c<-0.5 || (c<0.3 && c > -0.1); }},
  {"c<0.2 & 2.0", [](double c) { return c<0.2 && static_cast<bool>(2.0); }},
  {"c<0.2 | 2.0", [](double c) { return c<0.2 || static_cast<bool>(2.0); }},
  {"c<0.2 & 0.0", [](double c) { return c<0.2 && static_cast<bool>(0.0); }},
  {"c<0.2 | 0.0", [](double c) { return c<0.2 || static_cast<bool>(0.0); }},
};
// clang-format off

int
main(int argc, char * argv[])
{
  double norm;
  int total = 0, fail = 0;

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
      std::cerr << "Error evaluating expression '" << test.expression << "' simplified to '" << func.format() << "'\n";
      fail++;
    }

    func.compile();

    // evaluate for various values of c
    norm = 0.0;
    for (c = -1.0; c <= 1.0; c += 0.3)
      norm += std::abs(func.value() - test.native(c));
    if (norm > 1e-9)
    {
      std::cerr << "Error evaluating compiled expression '" << test.expression << "' simplified to '" << func.format() << "'\n";
      fail++;
    }

    total += 3;
  }

  if (fail)
    std::cout << "\033[1;31m"; // red
  else
    std::cout << "\033[1;32m"; // green


  std::cout << (total - fail) << " tests PASSED, " << fail << " tests FAILED\n";
  std::cout << "\033[0m"; // reset color

  // deriv.simplify();
  //
  // // evaluate for various values of c
  // for (c = -1.0; c <= 1.0; c += 0.3)
  //   std::cout << deriv.value() << ' ';
  // std::cout << '\n';
  //
  // deriv.compile();
  //
  // // evaluate for various values of c
  // for (c = -1.0; c <= 1.0; c += 0.3)
  //   std::cout << deriv.value() << ' ';
  //
  // // finite differencing
  // auto dc = 1.0e-9;
  // for (c = -1.0; c <= 1.0; c += 0.3)
  // {
  //   auto a = func.value();
  //   c += dc;
  //   auto b = func.value();
  //   c -= dc;
  //   std::cout << (b - a) / dc << ' ';
  // }
  // std::cout << '\n';

  return fail ? 1 : 0;
}
