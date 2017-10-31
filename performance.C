#include "SymbolicMath.h"
#include "SymbolicMathFunction.h"
#include "SymbolicMathHelpers.h"
#include "SymbolicMathJITTypes.h"

#include <iostream>
#include <chrono>

int
main(int argc, char * argv[])
{
  std::cout << "SymbolicMath with backend " << SymbolicMath::jit_backend_name << '\n';
  SymbolicMath::Parser parser;

  SymbolicMath::Real c;
  auto c_var = std::make_shared<SymbolicMath::RealReferenceData>(c, "c");
  parser.registerValueProvider(c_var);

  SymbolicMath::Real T = 500.0;
  auto T_var = std::make_shared<SymbolicMath::RealReferenceData>(T, "y");
  parser.registerValueProvider(T_var);

  parser.registerConstant("kB", 8.6173324e-5);
  parser.registerConstant("T0", 410.0);

  const std::string expression =
      "(c * (1 - c) * (0.380577499303532 + -0.0885190880468175 * (1 - 2 * c) + \
                                  -0.0549325534161349 * (1 - 2 * c) ^\
                              2 + -0.203752174307515 * (1 - 2 * c) ^\
                              3 + -0.164028794268281 * (1 - 2 * c) ^\
                              4 + 0.0172550241855144 * (1 - 2 * c) ^ 5) +\
               c * -3.85953051397515 + (1 - c) * -4.16723234898182) *\
              y / T0 +\
          (c * (1 - c) * (0.385512095260579 + -0.0962430189496054 * (1 - 2 * c) +\
                              -0.041249704177587 * (1 - 2 * c) ^\
                          2 + -0.194439246552959 * (1 - 2 * c) ^\
                          3 + -0.195412847295217 * (1 - 2 * c) ^\
                          4 + -0.00967038578662529 * (1 - 2 * c) ^ 5) +\
           c * -3.83660056975999 + (1 - c) * -4.12297413272388) -\
          (0.000263056717498972 + 4.614980081531 * 10 ^ -5 * (1 - c) + -4.75235048526914 * 10 ^\
           -5 * (1 - c) ^ 2 + 9.35759929354588 * 10 ^ -6 * (1 - c) ^ 3) *\
              y * log(y) -\
          (3.04676203180853 * 10 ^ -9 + -2.07225774483557 * 10 ^\
           -8 * (1 - c) + 3.55582178830517 * 10 ^ -8 * (1 - c) ^ 2 + -2.70425743485173 * 10 ^\
           -8 * (1 - c) ^ 3) *\
              y ^\
      2 +\
          (-1.51823088659839 * 10 ^ -13 + 5.18553402098699 * 10 ^\
           -12 * (1 - c) + -4.56309143596694 * 10 ^ -12 * (1 - c) ^ 2 + 1.08597105154957 * 10 ^\
           -11 * (1 - c) ^ 3) /\
              2 * y ^\
      3 + (-(c * (1 - c) * (\
                     0.385512095260579\
                  + -0.0962430189496054 * (1 - 2*c)\
                  + -0.041249704177587  * (1 - 2*c)^2\
                  + -0.194439246552959  * (1 - 2*c)^3\
                  + -0.195412847295217  * (1 - 2*c)^4\
                  + -0.00967038578662529* (1 - 2*c)^5\
              )\
            + c * -3.83660056975999 + (1-c) * -4.12297413272388) / T0 + (0.000263056717498972\
                  +  4.614980081531*10^-5   * (1-c)\
                  + -4.75235048526914*10^-5 * (1-c)^2\
                  +  9.35759929354588*10^-6 * (1-c)^3) * log(T0) + (3.04676203180853*10^-9\
                        + -2.07225774483557*10^-8 * (1-c)\
                        +  3.55582178830517*10^-8 * (1-c)^2\
                        + -2.70425743485173*10^-8 * (1-c)^3) * T0 + (-1.51823088659839*10^-13\
                              +  5.18553402098699*10^-12 * (1-c)\
                              + -4.56309143596694*10^-12 * (1-c)^2\
                              + 1.08597105154957*10^-11  * (1-c)^3) / 2 * T0 ^ 2) * y +\
          y * kB * ((1 - c) * log(1 - c) + c * log(c))";

  const std::string expression2 = "T0 * c * 4.5565 * 10 ^ \
  - 2 ";

  auto func = parser.parse(expression);
  std::cout << func.format() << '\n';

  func.simplify();
  std::cout << " = " << func.format() << '\n'; // << func->formatTree("\t") << '\n';

  func.compile();

  unsigned int n = 0;
  for (c = 0.01; c <= 0.99; c += 0.001)
    for (T = 200.0; T <= 800.0; T += 0.01)
      n++;

  // evaluate for various values of c and T
  double sum = 0.0;
  auto start = std::chrono::high_resolution_clock::now();
  for (c = 0.01; c <= 0.99; c += 0.001)
    for (T = 200.0; T <= 800.0; T += 0.01)
      sum += func.value();
  auto finish = std::chrono::high_resolution_clock::now();

  std::cout << sum << '\n';

  std::chrono::duration<double> elapsed = finish - start;
  std::cout << "Elapsed time: " << elapsed.count() << " s\n";

  return 0;
}
