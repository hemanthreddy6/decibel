.PHONY: build test clean

build: parser

y.tab.c: parser.y
	@if yacc --version > /dev/null 2>/dev/null; then \
		yacc -d parser.y ; \
	else \
		byacc -d parser.y 2>/dev/null; \
	fi

parser: y.tab.c ../Lex/lex.yy.c
	gcc -o parser y.tab.c -lm

../Lex/lex.yy.c:
	@make build -C "../Lex"

test: parser
	@make test -C "Tests" --quiet

clean:
	rm -f parser
	rm -f y.tab.c
	rm -f y.tab.h

