CC= gcc
CXX= g++
DEBUG=-O0 -g

all: clean asm-template.exe union-sample.exe

.c.o:
	$(CC) $(DEBUG) -c -o $@ $<
.cpp.o:
	$(CXX) $(DEBUG) -c -o $@ $<  -std=c++11

asm-template.exe: assem.o
	$(CXX) -o $@ $< -std=c++11

union-sample.exe: union-sample.o
	$(CXX) -o $@ $< -std=c++11

clean:
	rm -f *.o *~ \#*\#
