CXX ?= clang++

OBJS := MathParse.o MathParseTokenizer.o MathParseAST.o MathParseOperators.o MathParseFunctions.o

mathparse: main.C $(OBJS)
	$(CXX) -std=c++11 $(LDFLAGS) -o mathparse main.C $(OBJS)

-include $(OBJS:.o=.d)

%.o : %.C
	$(CXX) -std=c++11 -c $(CXXFLAGS) $(CPPFLAGS) $*.C -o $@
	$(CXX) -std=c++11 -MM $(CXXFLAGS) $*.C > $*.d

clean:
	rm -rf $(OBJS) *.o *.d
