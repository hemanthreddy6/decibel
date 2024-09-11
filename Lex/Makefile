build:
	flex lexer.l
	gcc -o lexer lex.yy.c -lm

run: build
	./lexer

clean:
	rm lex.yy.c
	rm lexer
