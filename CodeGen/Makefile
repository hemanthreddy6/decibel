-include ../Makefile.inc
COMPILER := $(if $(COMPILER),$(COMPILER),g++)
.PHONY: build clean

build: codegen

codegen: codegen.cpp ../Semantic/semantic_analyser.cpp ../Parser/y.tab.c
	clang++ codegen.cpp -o codegen `llvm-config --cxxflags --ldflags --libs core irreader --system-libs`

../Parser/y.tab.c: ../Parser/parser.y ../Lex/lexer.l
	@make build -C "../Parser" --quiet

run: codegen
	./codegen < test.dbl
	llc -filetype=obj -relocation-model=pic out.ll -o out.o
	clang out.o -o out
	./out

clean:
	rm -f codegen
	rm -f out.ll
	rm -f out.o
	rm -f out
