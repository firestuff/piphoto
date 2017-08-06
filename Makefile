all: piphoto

piphoto: *.cc *.h Makefile
	clang-3.9 -O3 -g -Weverything -Werror -Wno-c++98-compat -Wno-c++98-c++11-compat-pedantic --std=c++1z --stdlib=libc++ -o piphoto piphoto.cc -lc++ -lunwind -lpng

run: piphoto
	./piphoto

clean:
	rm -f piphoto
