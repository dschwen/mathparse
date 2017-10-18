# SLJIT
OBJS += contrib/sljit_src/sljitLir.o \
				SymbolicMathFunctionSLJIT.o SymbolicMathNodeDataSLJIT.o
CONFIG := -DSLJIT_CONFIG_AUTO=1 -DSYMBOLICMATH_USE_SLJIT
LDFLAGS += contrib/sljit_src/sljitLir.c
