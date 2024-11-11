-include ../../Makefile.inc
COMPILER := $(if $(COMPILER),$(COMPILER),g++)
.PHONY: build run clean test

TESTS := $(wildcard *.in)

INPUT_FILES := $(if $(FILE), $(FILE).in, $(TESTS))

build: lexer

../lex.yy.c: ../lexer.l
	@make lex.yy.c -C "../" --quiet

../../Parser/y.tab.c: ../../Parser/parser.y
	@make y.tab.c -C "../../Parser" --quiet

lexer: ../lex.yy.c ../../Parser/y.tab.c tester.c
	@$(COMPILER) -o lexer tester.c -lm

run: build
	./lexer

clean:
	rm -f lexer

test: build $(INPUT_FILES)
	@sorted_files=$$(echo "$(INPUT_FILES)" | tr ' ' '\n' | sort -V | tr '\n' ' '); \
	for input_file in $$sorted_files; do \
		output_file=$$(basename $$input_file .in).out; \
		if !(./lexer < $$input_file | diff --color=auto - $$output_file); then \
			echo "\033[31m$$(basename $$input_file .in) [ FAILED ]\033[0m"; \
			FAIL=1; \
		else \
			echo "\033[32m$$(basename $$input_file .in) [ PASSED ]\033[0m"; \
		fi; \
	done; \
	if [ -n "$$FAIL" ]; then \
		exit 1; \
	fi
