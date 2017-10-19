# LibJIT
OBJS += SymbolicMathFunctionLibJIT.o SymbolicMathNodeDataLibJIT.o
CONFIG := -DSLJIT_CONFIG_AUTO=1 -DSYMBOLICMATH_USE_LIBJIT
LDFLAGS := -ljit
