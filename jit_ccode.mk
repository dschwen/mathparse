# C code / dlopen
OBJS += SMFunctionCCode.o SMNodeDataCCode.o
CONFIG := -DSYMBOLICMATH_USE_CCODE -DCCODE_JIT_COMPILER='"$(CXX)"'
LDFLAGS := -ldl
