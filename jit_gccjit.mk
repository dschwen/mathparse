# GCC JIT
#GCC_BIN ?= $(dir $(shell which gcc))
#GCCDIR ?= $(GCC_BIN)/..
OBJS += SMFunctionGCCJIT.o SMNodeDataGCCJIT.o
CONFIG := -DSYMBOLICMATH_USE_GCCJIT
LDFLAGS := -lgccjit
#-L$(GCCDIR)/lib
#CPPFLAGS += -I$(GCCDIR)/include/
