///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include "SMCompiledCCode.h"
#include "SMCSourceGenerator.h"
#include "SMCompilerFactory.h"

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

registerCompiler(CompiledCCode, "CompiledCCode", Real, 10);

template <>
const std::string
CompiledCCode<Real>::typeHeader()
{
  return "#include <cmath>\n#include <algorithm>\n";
}

template <typename T>
CompiledCCode<T>::CompiledCCode(Function<T> & fb)
{
  // generate source
  CSourceGenerator<T> source(fb);
  std::string ccode = typeHeader() + "extern \"C\" " + source.typeName() + " F()\n{\n";
  ccode += source() + ";\n}";

  // save to a temporary name and rename only when the file is fully written
  char ctmpname[] = "./tmp_adc_XXXXXX.C";
  int ctmpfile = mkstemps(ctmpname, 2);
  if (ctmpfile == -1)
    fatalError("Error creating tmp file " + std::string(ctmpname));

  if (!write(ctmpfile, ccode.data(), ccode.length()))
    fatalError("Error writing source to tmp file " + std::string(ctmpname));

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

  if (system(command.c_str()) == -1)
    fatalError("Error launching compiler command  " + command);

  std::remove(ctmpname);

  // load object file in
  auto lib = dlopen(otmpname, RTLD_NOW);
  if (!lib)
  {
    // TODO: throw!
    std::remove(otmpname);
    fatalError("JIT object load failed.");
  }

  // fetch function pointer
  _jit_function = reinterpret_cast<JITFunctionPtr>(dlsym(lib, "F"));
  const char * error = dlerror();
  if (error)
  {
    // TODO: throw!
    std::remove(otmpname);
    fatalError("Error binding JIT compiled function\n" + std::string(error));
  }

  std::remove(otmpname);
}

template class CompiledCCode<Real>;

} // namespace SymbolicMath
