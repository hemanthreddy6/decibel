build:
	@if yacc -d parser.y 2>/dev/null; then \
		: ; \
	else \
		byacc -d parser.y 2>/dev/null; \
	fi
	gcc -o parser y.tab.c -lm

clean:
	rm -f parser
	rm -f y.tab.c
	rm -f y.tab.h
