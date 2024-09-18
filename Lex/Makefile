build:
	flex lexer.l
	@make build -C "../Parser" --quiet

run:
	@make run -C "Tests" --quiet

clean:
	rm -f lex.yy.c

test:
	@make test -C "Tests" --quiet