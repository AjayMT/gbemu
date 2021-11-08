
gbemu: src/*
	cc -Wall -Wno-deprecated-declarations -Werror -o gbemu src/*.c -lcsfml-window -lcsfml-graphics

gbemu_tests: src/* test/*
	cc -g -Wall -Werror -o gbemu_tests test/*.c $(filter-out src/gbemu.c, $(wildcard src/*.c))

.PHONY: clean
clean:
	rm -fr gbemu gbemu_tests *.dSYM
