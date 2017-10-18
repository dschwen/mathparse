# Lightning
OBJS += SymbolicMathFunctionLightning.o SymbolicMathNodeDataLightning.o
CONFIG := -DSYMBOLICMATH_USE_LIGHTNING
LDFLAGS += -llightning
