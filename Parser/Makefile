build:
	@if yacc --version > /dev/null 2>/dev/null; then \
		yacc -d parser.y ; \
	else \
		byacc -d parser.y 2>/dev/null; \
	fi
	gcc -o parser y.tab.c -lm

clean:
	rm -f parser
	rm -f y.tab.c
	rm -f y.tab.h
