-include ../../Makefile.inc
COMPILER := $(if $(COMPILER),$(COMPILER),g++)
.PHONY: build run clean test

TESTS := $(wildcard *.dbl)

INPUT_FILES := $(if $(FILE), $(FILE).dbl, $(TESTS))

build: codegen decibel_stdlib.o

codegen: codegen.cpp ../Semantic/semantic_analyser.cpp ../Parser/y.tab.c
	clang++-18 codegen.cpp -o codegen `llvm-config-18 --cxxflags --ldflags --libs core irreader --system-libs`

../Parser/y.tab.c: ../Parser/parser.y ../Lex/lexer.l
	@make build -C "../Parser" --quiet

test: 
	@make test -C "Tests" --quiet

run: codegen decibel_stdlib.o
	./codegen < test.dbl
	llc-18 -filetype=obj -relocation-model=pic out.ll -o out.o
	clang++ out.o decibel_stdlib.o -o out -lsfml-audio -lsfml-system
	./out

decibel_stdlib.o: decibel_stdlib.cpp
	clang++ -c decibel_stdlib.cpp -o decibel_stdlib.o

clean:
	rm -f lexer

test: build $(INPUT_FILES)
	@sorted_files=$$(echo "$(INPUT_FILES)" | tr ' ' '\n' | sort -V | tr '\n' ' '); \
	for input_file in $$sorted_files; do \
		output_file=$$(basename $$input_file .dbl).out; \
		if !(./codegen < $$input_file | diff --color=auto - $$output_file); then \
			printf "%-60b%b\n" "\033[31m$$(basename $$input_file .dbl) " "[ FAILED ]\033[0m"; \
			FAIL=1; \
		else \
			printf "%-60b%b\n" "\033[32m$$(basename $$input_file .dbl) " "[ PASSED ]\033[0m"; \
		fi; \
	done; \
	if [ -n "$$FAIL" ]; then \
		exit 1; \
	fi
