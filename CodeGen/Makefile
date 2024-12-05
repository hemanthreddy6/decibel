-include ../Makefile.inc
COMPILER := $(if $(COMPILER),$(COMPILER),g++)
.PHONY: build clean

build: codegen

codegen: codegen.cpp ../Semantic/semantic_analyser.cpp ../Parser/y.tab.c
	clang++-18 codegen.cpp -o codegen `llvm-config-18 --cxxflags --ldflags --libs core irreader --system-libs`

../Parser/y.tab.c: ../Parser/parser.y ../Lex/lexer.l
	@make build -C "../Parser" --quiet

test: run

run: codegen
	./codegen < test.dbl
	llc-18 -filetype=obj -relocation-model=pic out.ll -o out.o
	clang-18 out.o -o out
	./out

clean:
	rm -f codegen
	rm -f out.ll
	rm -f out.o
	rm -f out
