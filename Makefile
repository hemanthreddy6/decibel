.PHONY: build clean test

build:
	@make build -C "Lex" --quiet
	@make build -C "Parser" --quiet
	@make build -C "Semantic" --quiet

clean:
	@make clean -C "Lex" --quiet
	@make clean -C "Lex/Tests" --quiet
	@make clean -C "Parser" --quiet
	@make clean -C "Semantic" --quiet

test:
	@make test -C "Lex" --quiet
	@make test -C "Parser" --quiet
	@make test -C "Semantic" --quiet
