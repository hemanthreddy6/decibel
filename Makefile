.PHONY: build clean test

build:
	@make build -C "Lex" --quiet
	@make build -C "Parser" --quiet
	@make build -C "Semantic" --quiet
	@make build -C "CodeGen" --quiet

build_deb: build
	cp CodeGen/codegen decibel/usr/bin/decibel
	cp CodeGen/decibel_stdlib.o decibel/usr/lib/decibel/decibel_stdlib.o
	chmod -R 755 decibel/DEBIAN
	chmod -R 755 decibel/usr
	dpkg-deb --build decibel

install: 

clean:
	@make clean -C "Lex" --quiet
	@make clean -C "Lex/Tests" --quiet
	@make clean -C "Parser" --quiet
	@make clean -C "Semantic" --quiet
	@make clean -C "CodeGen" --quiet

test:
	-@make test -C "Lex" --quiet
	-@make test -C "Parser" --quiet
	-@make test -C "Semantic" --quiet
	-@make test -C "CodeGen" --quiet
