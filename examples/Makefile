DEPENDENCIES=$(wildcard ../dfa/*.c)

prog: main.c ${DEPENDENCIES}
	gcc -o prog.out main.c ${DEPENDENCIES}

run: prog
	./prog.out