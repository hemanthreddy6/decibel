-include ../Makefile.inc
COMPILER := $(if $(COMPILER),$(COMPILER),g++)
.PHONY: build clean test make_parser

build: semantic_analyser

semantic_analyser: semantic_analyser.cpp ../Parser/y.tab.c 
	$(COMPILER) semantic_analyser.cpp -o semantic_analyser

../Parser/y.tab.c: ../Parser/parser.y ../Lex/lexer.l
	@make build -C "../Parser" --quiet

test:
	@make test -C "Tests" --quiet

clean:
	rm -f semantic_analyser
