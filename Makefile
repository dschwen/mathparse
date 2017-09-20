CXX ?= clang++
CXXFLAGS ?= -std=c++11

OBJS := MathParse.o MathParseTokenizer.o

mathparse: main.C $(OBJS)
	$(CXX) $(LDFLAGS) -o mathparse main.C $(OBJS)

-include $(OBJS:.o=.d)

%.o : %.C
	echo $(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $*.C -o $@
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $*.C -o $@
	$(CXX) -MM $(CXXFLAGS) $*.C > $*.d

clean:
	rm -rf $(OBJS) *.o *.d
