# GCC JIT
GCCDIR ?= /opt/moose/gcc-7.2.0
OBJS += SymbolicMathFunctionGCCJIT.o SymbolicMathNodeDataGCCJIT.o
CONFIG := -DSYMBOLICMATH_USE_GCCJIT
LDFLAGS := -lgccjit -L$(GCCDIR)/lib
CPPFLAGS := -I$(GCCDIR)/include/
