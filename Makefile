
gbemu: src/*
	cc -Wall -Wno-deprecated-declarations -Werror -o gbemu src/*.c -lcsfml-window -lcsfml-graphics

.PHONY: clean
clean:
	rm -fr gbemu
