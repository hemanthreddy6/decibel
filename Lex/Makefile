TEST_INPUT_FILES := $(wildcard Tests/*.in)

build:
	flex lexer.l
	gcc -o lexer lex.yy.c -lm

run: build
	./lexer

clean:
	rm lex.yy.c
	rm lexer

test: $(TEST_INPUT_FILES)
	@make build --quiet
	@for input_file in $(TEST_INPUT_FILES); do \
		output_file=Tests/$$(basename $$input_file .in).out; \
		(./lexer < $$input_file) | diff -q $$output_file - > /dev/null && \
			echo "\033[32m$$(basename $$input_file .in) [ PASSED ]\033[0m" || \
			(echo "\033[31m$$(basename $$input_file .in) [ FAILED ]\033[0m" && exit 1); \
	done
	@make clean --quiet