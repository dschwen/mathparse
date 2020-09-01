///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include "SMUtils.h"

#include <cmath>
#include <sstream>
#include <iomanip>
#include <limits>
#include <stdexcept>

namespace SymbolicMath
{

// replace this with mooseError in the future
[[noreturn]] void
fatalError(const std::string & error)
{
  throw std::runtime_error(error);
  // std::cerr << error << '\n';
  // std::exit(1);
}

std::string
stringify(Real number)
{
  if (number == std::round(number))
    return std::to_string(static_cast<int>(number));
  else
  {
    // this or std::numeric_limits<T>::max_digits10
    const unsigned int max_digits10 =
        std::floor(std::numeric_limits<Real>::digits * std::log10(2) + 2);

    std::ostringstream os;
    os << std::setprecision(max_digits10) << number;
    return os.str();
  }
}

} // namespace SymbolicMath
