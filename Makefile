all: piphoto

objects = piphoto.o lut.o util.o

piphoto: $(objects) Makefile
	clang-3.9 -O3 -g -Weverything -Werror --std=c++1z --stdlib=libc++ -o piphoto $(objects) -lc++ -lunwind -lpng

%.o: %.cc *.h Makefile
	clang-3.9 -O3 -g -Weverything -Werror -Wno-padded -Wno-c++98-compat -Wno-c++98-c++11-compat-pedantic --std=c++1z --stdlib=libc++ -c -o $@ $<

run: piphoto
	./piphoto

clean:
	rm -f piphoto *.o
