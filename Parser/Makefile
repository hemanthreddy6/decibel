-include ../Makefile.inc
COMPILER := $(if $(COMPILER),$(COMPILER),g++)

.PHONY: build test clean run

build: parser

y.tab.c: parser.y
	@if yacc --version > /dev/null 2>/dev/null; then \
		yacc -d parser.y ; \
	else \
		bison -d parser.y 2>/dev/null; \
	fi

parser: y.tab.c ../Lex/lex.yy.c
	$(COMPILER) $(CXXFLAGS) -o parser y.tab.c -lm

../Lex/lex.yy.c: ../Lex/lexer.l
	@make build -C "../Lex" --quiet

test: parser
	@make test -C "Tests" --quiet

run: parser
	@./parser

clean:
	rm -f parser
	rm -f y.tab.c
	rm -f y.tab.h
	rm -f parser.tab.c
	rm -f parser.tab.h
