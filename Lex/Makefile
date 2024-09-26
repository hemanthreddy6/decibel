.PHONY: build run clean test

build: lex.yy.c

lex.yy.c: lexer.l
	flex lexer.l

run:
	@make run -C "Tests" --quiet

clean:
	rm -f lex.yy.c

test:
	@make test -C "Tests" --quiet