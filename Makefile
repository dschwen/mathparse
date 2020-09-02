CXX ?= clang++
CXXFLAGS ?= -O2

OBJS := SMToken.o SMTokenizer.o SMParser.o SMSymbols.o \
				SMNode.o SMNodeData.o SMUtils.o \
				SMTransform.o SMTransformSimplify.o SMCompiledByteCode.o \
				SMCompiledCCode.o SMCompiledSLJIT.o SMCompiledLibJIT.o \
				SMCSourceGenerator.o SMCompiledLightning.o

# include configuration for the selected JIT backend
ifneq ($(JIT)x, x)
include jit_$(JIT).mk
endif

# add machine specific stuff
ifneq (,$(findstring armv,$(shell uname -m)))
  LDFLAGS += -latomic
endif

# SLJIT
OBJS += contrib/sljit_src/sljitLir.o
CONFIG += -DSLJIT_CONFIG_AUTO=1

# CCode
override LDFLAGS += -ldl

# libjit
LIBJIT_DIR ?= /usr/local
override CPPFLAGS += -I$(LIBJIT_DIR)/include
override LDFLAGS += -L$(LIBJIT_DIR)/lib -ljit

# Lightning
override LDFLAGS += -llightning

# LLVM IR
LLVM_CONFIG ?= llvm-config
LLVM_MAJOR := $(shell $(LLVM_CONFIG) --version | cut -d. -f1)
ifeq "$(LLVM_MAJOR)" "9"
  override LDFLAGS += $(shell $(LLVM_CONFIG) --ldflags --system-libs --libs core orcjit native)
  override CXXFLAGS += -I$(shell $(LLVM_CONFIG) --includedir)
  override CPPFLAGS += -DLLVM_MAJOR=$(LLVM_MAJOR) -DSYMBOLICMATH_USE_LLVMIR
  OBJS += SMCompiledLLVM.o
endif

# Applications

mathparse: main.C $(OBJS)
	$(CXX) -std=c++11 $(CONFIG) $(CPPFLAGS) $(CXXFLAGS) -o mathparse main.C $(OBJS) $(LDFLAGS)

performance: Performance.C $(OBJS)
	$(CXX) -std=c++11 $(CONFIG) $(CPPFLAGS) $(CXXFLAGS) -o performance Performance.C $(OBJS) $(LDFLAGS)

unittests: UnitTests.C $(OBJS)
	$(CXX) -std=c++11 $(CONFIG) $(CPPFLAGS) $(CXXFLAGS) -o unittests UnitTests.C $(OBJS) $(LDFLAGS)

testbench: TestBench.C $(OBJS)
	$(CXX) -std=c++11 $(CONFIG) $(CPPFLAGS) $(CXXFLAGS) -o testbench TestBench.C $(OBJS) $(LDFLAGS)

-include $(OBJS:.o=.d)

%.o : %.C
	$(CXX) -std=c++11 $(CONFIG) -c $(CXXFLAGS) $(CPPFLAGS) $*.C -o $@
	$(CXX) -std=c++11 $(CONFIG) -MM $(CXXFLAGS) $(CPPFLAGS) $*.C > $*.d

%.o : %.c
	$(CC) $(CONFIG) -c $(CFLAGS) $(CPPFLAGS) $*.c -o $@
	$(CC) $(CONFIG) -MM $(CFLAGS) $(CPPFLAGS) $*.c > $*.d

.PHONY: force clean

clean:
	rm -rf $(OBJS) *.o *.d mathparse performance unittests testbench performance_fparser

# FParser (for performance comparison)

performance_fparser: PerformanceFparser.C
	$(CXX) -std=c++11 $(CPPFLAGS) $(CXXFLAGS) -I$(LIBMESH_DIR)/include -o performance_fparser PerformanceFparser.C -Wl,-rpath=$(LIBMESH_DIR)/lib -L$(LIBMESH_DIR)/lib -lmesh_opt
