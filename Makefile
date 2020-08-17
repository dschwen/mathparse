CXX ?= clang++
CPPFLAGS ?= -O2

# (lightning,sljit,libjit)
JIT ?= vm

OBJS := SMToken.o SMTokenizer.o SMParser.o SMSymbols.o \
				SMNode.o SMNodeData.o SMUtils.o SMFunctionBase.o \
				SMTransform.o SMTransformSimplify.o SMCompiledByteCode.o \
				SMCompiledCCode.o SMCompiledSLJIT.o

# include configuration for the selected JIT backend
include jit_$(JIT).mk

# add machine specific stuff
ifneq (,$(findstring armv,$(shell uname -m)))
  LDFLAGS += -latomic
endif

# SLJIT
OBJS += contrib/sljit_src/sljitLir.o
CONFIG += -DSLJIT_CONFIG_AUTO=1

# CCode
override LDFLAGS += -ldl

# Lightning
#override LDFLAGS += -llightning


mathparse: main.C $(OBJS)
	$(CXX) -std=c++11 $(CONFIG) $(CPPFLAGS) $(CXXFLAGS) -o mathparse main.C $(OBJS) $(LDFLAGS)

performance: Performance.C $(OBJS)
	$(CXX) -std=c++11 $(CONFIG) $(CPPFLAGS) $(CXXFLAGS) -o performance performance.C $(OBJS) $(LDFLAGS)

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
.jit_backend: force
	echo '$(JIT)' | cmp -s - $@ || echo '$(JIT)' > $@

# force rebuild when compiling with a new JIT backend
$(OBJS): .jit_backend

clean:
	rm -rf $(OBJS) *.o *.d mathparse performance test2 test3 performance_fparser

# FParser (for performance comparison)

fparser4.5.2.zip:
	wget http://warp.povusers.org/FunctionParser/fparser4.5.2.zip

fparser/fparser.hh: fparser4.5.2.zip
	mkdir -p fparser && cd fparser && unzip -DD ../fparser4.5.2.zip

%.o : %.cc
	$(CXX) -std=c++11 $(CONFIG) -c $(CXXFLAGS) $(CPPFLAGS) $*.cc -o $@
	$(CXX) -std=c++11 $(CONFIG) -MM $(CXXFLAGS) $(CPPFLAGS) $*.cc > $*.d

FPARSER_SRC := $(wildcard fparser/*.cc)
FPARSER_OBJS := $(patsubst %.cc, %.o, $(FPARSER_SRC))
fparser: $(FPARSER_OBJS)

performance_fparser: performance_fparser.C fparser/fparser.hh $(FPARSER_OBJS)
	$(CXX) -std=c++11 $(CPPFLAGS) $(CXXFLAGS) -Ifparser -o performance_fparser performance_fparser.C $(FPARSER_OBJS)

# Tinkering around

tests: test2 test3

test2: test2.C
	clang++ -std=c++11 -DSLJIT_CONFIG_AUTO=1 -o test2 test2.C contrib/sljit_src/sljitLir.c

test3: test3.C
	clang++ -std=c++11 -DSLJIT_CONFIG_AUTO=1 -o test3 test3.C contrib/sljit_src/sljitLir.c

test5: test5.C
	clang++ test5.C `llvm-config --cxxflags --ldflags --system-libs --libs core orcjit native` -g -o test5

test6: test6.C
	clang++ test6.C `llvm-config --cxxflags --ldflags --system-libs --libs core orcjit native` -g -o test6
