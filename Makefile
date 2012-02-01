LDFLAGS=-pthread

all: main

clean:
	rm -f *.o *~ main

dist:
	-rm -rf thread-interfere
	mkdir thread-interfere
	cp README Makefile main.cpp atomic_counter.hpp thread-interfere
	tar cjf thread-interfere.tar.bz2 thread-interfere
	
		