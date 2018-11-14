# GCC JIT
GCC_BIN ?= $(dir $(shell which gcc))
GCCDIR ?= $(GCC_BIN)/..
OBJS += SymbolicMathFunctionGCCJIT.o SymbolicMathNodeDataGCCJIT.o
CONFIG := -DSYMBOLICMATH_USE_GCCJIT
LDFLAGS := -lgccjit -L$(GCCDIR)/lib
CPPFLAGS := -I$(GCCDIR)/include/
