#include "LLJITHelper.h"

#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Support/Debug.h>

using namespace llvm;
using namespace llvm::orc;

LLJITHelper::LLJITHelper()
{
  LLJITBuilder Builder;
  // TT = Builder.JTMB->getTargetTriple();
  LLJIT = std::move(Builder.create().get());

  if (auto R = createHostProcessResolver(LLJIT->getDataLayout()))
    LLJIT->getMainJITDylib().setGenerator(std::move(R));
}

JITDylib::GeneratorFunction
LLJITHelper::createHostProcessResolver(DataLayout DL)
{
  char Prefix = DL.getGlobalPrefix();
  Expected<JITDylib::GeneratorFunction> R =
      DynamicLibrarySearchGenerator::GetForCurrentProcess(Prefix);

  if (!R)
  {
    LLJIT->getExecutionSession().reportError(R.takeError());
    return nullptr;
  }

  if (!*R)
  {
    LLJIT->getExecutionSession().reportError(createStringError(
        inconvertibleErrorCode(), "Generator function for host process symbols must not be null"));
    return nullptr;
  }

  return *R;
}

Error
LLJITHelper::submitModule(std::unique_ptr<Module> M, std::unique_ptr<LLVMContext> C)
{
  return LLJIT->addIRModule(ThreadSafeModule(std::move(M), std::move(C)));
}

Expected<JITTargetAddress>
LLJITHelper::getFunctionAddr(StringRef Name)
{
  Expected<JITEvaluatedSymbol> S = LLJIT->lookup(Name);
  if (!S)
    return S.takeError();

  JITTargetAddress A = S->getAddress();
  if (!A)
    return createStringError(inconvertibleErrorCode(), "'%s' evaluated to nullptr", Name.data());

  return A;
}
