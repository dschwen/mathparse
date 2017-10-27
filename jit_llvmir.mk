# SLJIT
OBJS += SymbolicMathFunctionLLVMIR.o SymbolicMathNodeDataLLVMIR.o
CONFIG := -DSYMBOLICMATH_USE_LLVMIR
LDFLAGS := $(shell llvm-config --ldflags --system-libs --libs core orcjit native)
# filter out the -fno-rtti flag (ok as we're not deriving from LLVM classes)
CXXFLAGS += $(shell llvm-config --cxxflags | sed 's/ -fno-rtti//g')
CPPFLAGS := $(CXXFLAGS)
