-include ../Makefile.inc
COMPILER := $(if $(COMPILER),$(COMPILER),g++)
.PHONY: build clean

build: codegen

codegen: codegen.cpp ../Semantic/semantic_analyser.cpp ../Parser/y.tab.c
	clang++ codegen.cpp -o codegen `llvm-config --cxxflags --ldflags --libs core irreader --system-libs`

../Parser/y.tab.c: ../Parser/parser.y ../Lex/lexer.l
	@make build -C "../Parser" --quiet

clean:
	rm -f codegen
