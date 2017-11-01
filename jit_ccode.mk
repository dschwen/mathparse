# C code / dlopen
OBJS += SymbolicMathFunctionCCode.o SymbolicMathNodeDataCCode.o
CONFIG := -DSYMBOLICMATH_USE_CCODE -DCCODE_JIT_COMPILER='"$(CXX)"'
LDFLAGS := -ldl
