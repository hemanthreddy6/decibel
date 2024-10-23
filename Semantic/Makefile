.PHONY: build clean test make_parser

build: semantic_analyser

semantic_analyser: semantic_analyser.cpp make_parser
	g++ semantic_analyser.cpp -o semantic_analyser

make_parser:
	@make build -C "../Parser" --quiet

test:
	@make test -C "Tests" --quiet

clean:
	rm -f semantic_analyser
