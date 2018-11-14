# LibJIT
OBJS += SymbolicMathFunctionLibJIT.o SymbolicMathNodeDataLibJIT.o
CONFIG := -DSYMBOLICMATH_USE_LIBJIT
LDFLAGS := -ljit
