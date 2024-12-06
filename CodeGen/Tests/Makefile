-include ../../Makefile.inc
COMPILER := $(if $(COMPILER),$(COMPILER),g++)
.PHONY: build run clean test

TESTS := $(wildcard *.dbl)

INPUT_FILES := $(if $(FILE), $(FILE).dbl, $(TESTS))

build: ../codegen ../decibel_stdlib.o

../codegen: ../codegen.cpp ../../Semantic/semantic_analyser.cpp ../../Parser/y.tab.c
	@make build -C "../" --quiet

../Parser/y.tab.c: ../Parser/parser.y ../Lex/lexer.l
	@make build -C "../../Parser" --quiet

run: codegen decibel_stdlib.o
	./codegen < test.dbl
	llc-18 -filetype=obj -relocation-model=pic out.ll -o out.o
	clang++ out.o decibel_stdlib.o -o out -lsfml-audio -lsfml-system
	./out

decibel_stdlib.o: decibel_stdlib.cpp
	@make build -C "../" --quiet

test: build $(INPUT_FILES)
	@sorted_files=$$(echo "$(INPUT_FILES)" | tr ' ' '\n' | sort -V | tr '\n' ' '); \
	for input_file in $$sorted_files; do \
		output_file=$$(basename $$input_file .dbl).out; \
		output_file2=$$(basename $$input_file .dbl).actual_out; \
		../codegen < $$input_file; \
		llc-18 -filetype=obj -relocation-model=pic out.ll -o out.o; \
		clang++ out.o ../decibel_stdlib.o -o out -lsfml-audio -lsfml-system; \
		./out > $$output_file2; \
		if !(diff --color=auto $$output_file2 $$output_file); then \
			printf "%-60b%b\n" "\033[31m$$(basename $$input_file .dbl) " "[ FAILED ]\033[0m"; \
			FAIL=1; \
		else \
			printf "%-60b%b\n" "\033[32m$$(basename $$input_file .dbl) " "[ PASSED ]\033[0m"; \
		fi; \
	done; \
	if [ -n "$$FAIL" ]; then \
		exit 1; \
	fi
