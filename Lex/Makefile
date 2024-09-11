build:
	flex lexer.l
	gcc -o lexer lex.yy.c

run: build
	./lexer

clean:
	rm lex.yy.c
	rm lexer
