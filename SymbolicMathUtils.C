#include "SymbolicMathUtils.h"

namespace SymbolicMath
{

// replace this with mooseError in the future
_Noreturn void
fatalError(const std::string & error)
{
  std::cerr << error << '\n';
  std::exit(1);
}

// end namespace SymbolicMath
}
