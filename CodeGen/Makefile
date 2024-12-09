-include ../Makefile.inc
COMPILER := $(if $(COMPILER),$(COMPILER),g++)
.PHONY: build clean

build: codegen decibel_stdlib.o

codegen: codegen.cpp ../Semantic/semantic_analyser.cpp ../Parser/y.tab.c
	clang++-18 codegen.cpp -o codegen `llvm-config-18 --cxxflags --ldflags --libs core irreader --system-libs`

../Parser/y.tab.c: ../Parser/parser.y ../Lex/lexer.l
	@make build -C "../Parser" --quiet

test: build
	@make test -C "Tests" --quiet

run: codegen decibel_stdlib.o
	./codegen < test2.dbl
	# llc-18 -filetype=obj -relocation-model=pic out.ll -o out.o
	clang++-18 out.ll decibel_stdlib.o -o out -lsfml-audio -lsfml-system
	./out

decibel_stdlib.o: decibel_stdlib.cpp
	clang++ -c decibel_stdlib.cpp -o decibel_stdlib.o

clean:
	rm -f codegen
	rm -f out.ll
	rm -f out.o
	rm -f out
	rm -f decibel_stdlib.o
