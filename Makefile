CXXFLAGS = -std=c++20 -Wall -pthread -O2
LDLIBS = -lpigpio -lrt

testmcp: testmcp.cpp libmcp300x.a

libmcp300x.a: libmcp300x.a(MCP300x.o)
	ranlib $@

libmcp300x.a(MCP300x.o): MCP300x.o

MCP300x.o: MCP300x.cpp MCP300x.hpp

