# LLVM IR
OBJS += SymbolicMathFunctionLLVMIR.o SymbolicMathNodeDataLLVMIR.o
CONFIG := -DSYMBOLICMATH_USE_LLVMIR
LLVM_CONFIG ?= llvm-config

# test LLVM version
LLVM_MAJOR := $(shell $(LLVM_CONFIG) --version | cut -d. -f1)
ifneq "$(LLVM_MAJOR)" "5"
ifneq "$(LLVM_MAJOR)" "6"
  $(error LLVM version 5.x or 6.x is required to build the llvmir backend)
endif
endif

LDFLAGS := $(shell $(LLVM_CONFIG) --ldflags --system-libs --libs core orcjit native)
CXXFLAGS += $(shell $(LLVM_CONFIG) --cxxflags)
CPPFLAGS := $(CXXFLAGS)
