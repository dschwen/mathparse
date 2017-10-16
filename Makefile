CXX ?= clang++

OBJS := SymbolicMathToken.o SymbolicMathTokenizer.o \
			  SymbolicMathParser.o SymbolicMathSymbols.o \
				SymbolicMathNode.o SymbolicMathNodeData.o \
				SymbolicMathUtils.o contrib/sljit_src/sljitLir.o

# OBJS += SymbolicMathFunctionLibJIT.o
OBJS += SymbolicMathFunctionSLJIT.o

CONFIG := -DSLJIT_CONFIG_AUTO=1 -DSYMBOLICMATH_USE_SLJIT

mathparse: main.C $(OBJS)
	$(CXX) -std=c++11 $(LDFLAGS) -ljit -o mathparse main.C $(OBJS)

-include $(OBJS:.o=.d)

%.o : %.C
	$(CXX) -std=c++11 $(CONFIG) -c $(CXXFLAGS) $(CPPFLAGS) $*.C -o $@
	$(CXX) -std=c++11 $(CONFIG) -MM $(CXXFLAGS) $*.C > $*.d

%.o : %.c
	$(CC) $(CONFIG) -c $(CFLAGS) $(CPPFLAGS) $*.c -o $@
	$(CC) $(CONFIG) -MM $(CXXFLAGS) $*.c > $*.d

clean:
	rm -rf $(OBJS) *.o *.d

tests: test2 test3

test2: test2.C
	clang++ -std=c++11 -DSLJIT_CONFIG_AUTO=1 -o test2 test2.C contrib/sljit_src/sljitLir.c

test3: test3.C
	clang++ -std=c++11 -DSLJIT_CONFIG_AUTO=1 -o test3 test3.C contrib/sljit_src/sljitLir.c
