#include "SymbolicMathUtils.h"

#include <cmath>

namespace SymbolicMath
{

// replace this with mooseError in the future
[[noreturn]] void
fatalError(const std::string & error)
{
  std::cerr << error << '\n';
  std::exit(1);
}

std::string
stringify(Real number)
{
  if (number == std::round(number))
    return std::to_string(static_cast<int>(number));
  else
    return std::to_string(number);
}

// end namespace SymbolicMath
}
