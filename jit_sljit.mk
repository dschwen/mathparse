# SLJIT
OBJS += SymbolicMathFunctionSLJIT.o SymbolicMathNodeDataSLJIT.o
CONFIG := -DSLJIT_CONFIG_AUTO=1 -DSYMBOLICMATH_USE_SLJIT
CXXFLAGS += -mshstk
LDFLAGS := contrib/sljit_src/sljitLir.c -mshstk
