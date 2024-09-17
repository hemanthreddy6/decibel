build:
	flex lexer.l
	@make build -C "../Parser" --quiet

clean:
	rm -f lex.yy.c

test:
	@make test -C "Tests" --quiet