build:
	yacc -d parser.y
	gcc -o parser y.tab.c -lm

clean:
	rm -f parser
	rm -f y.tab.c
	rm -f y.tab.h
