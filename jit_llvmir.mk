# SLJIT
OBJS += SymbolicMathFunctionLLVMIR.o SymbolicMathNodeDataLLVMIR.o
CONFIG := -DSYMBOLICMATH_USE_LLVMIR
LLVM_CONFIG ?= llvm-config
LDFLAGS := $(shell $(LLVM_CONFIG) --ldflags --system-libs --libs core orcjit native)
CXXFLAGS += $(shell $(LLVM_CONFIG) --cxxflags)
CPPFLAGS := $(CXXFLAGS)
