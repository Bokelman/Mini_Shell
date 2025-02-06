
FLAGS = -ansi -Wall -g -O0 -Wshadow -Wwrite-strings \
	-pedantic-errors -fstack-protector-all -Wextra

all: d8sh

d8sh: lexer.o parser.tab.o executor.o d8sh.o 
	gcc -lreadline lexer.o parser.tab.o executor.o d8sh.o -o d8sh

lexer.o: lexer.c
	gcc $(FLAGS) -c lexer.c

parser.tab.o: parser.tab.c command.h
	gcc $(FLAGS) -c parser.tab.c

executor.o: executor.c executor.h command.h
	gcc $(FLAGS) -c executor.c

d8sh.o: d8sh.c executor.h lexer.h
	gcc $(FLAGS) -c d8sh.c

clean: 
	@echo "cleaning files"
	rm -f d8sh ./*~ ./*.o


