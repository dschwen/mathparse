# Lightning
OBJS += SMFunctionLightning.o SMNodeDataLightning.o
CONFIG := -DSYMBOLICMATH_USE_LIGHTNING
LDFLAGS := -llightning
