///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#pragma once

#include <string>
#include <cmath>
#include <cstdlib>
#include <iostream>

namespace SymbolicMath
{

typedef double Real;

namespace Constant
{
// circle number pi
const Real pi = 3.1415926535897932384626433832795;
// Euler's constant e
const Real e = 2.7182818284590452353602874713527;
// Golden Ratio
const Real phi = 1.6180339887498948482;
// natural logarithm of 2
const Real ln2 = std::log(2.0);
// natural logarithm of 10
const Real ln10 = std::log(10.0);
} // namespace Constant

[[noreturn]] void fatalError(const std::string & error);

std::string stringify(Real number);

} // namespace SymbolicMath
