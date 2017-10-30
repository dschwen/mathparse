# C code / dlopen
OBJS += SymbolicMathFunctionCCode.o SymbolicMathNodeDataCCode.o
CONFIG := -DSYMBOLICMATH_USE_CCODE
LDFLAGS := -ldl
