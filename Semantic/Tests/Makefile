COMPILER := $(if $(COMPILER),$(COMPILER),g++)
CORRECT_TESTS := $(wildcard *.correct)
WRONG_TESTS := $(wildcard *.wrong)
FILE_DECLARED := $(if $(FILE), true, false)

build:
	@make build -C "../" --quiet

test: build $(CORRECT_TESTS) $(WRONG_TESTS)
	@if !($(FILE_DECLARED)); then \
		for correct_test in $(CORRECT_TESTS); do \
			if (../semantic_analyser < $$correct_test | grep "error" - ); then \
				printf "%-60b%b\n" "\033[31m$$(basename $$correct_test) " "[ FAILED ]\033[0m"; \
				FAIL=1; \
			else \
				printf "%-60b%b\n" "\033[32m$$(basename $$correct_test) " "[ PASSED ]\033[0m"; \
			fi; \
		done; \
		for wrong_test in $(WRONG_TESTS); do \
			if !(../semantic_analyser < $$wrong_test | grep "error" - > /dev/null); then \
				printf "%-60b%b\n" "\033[31m$$(basename $$wrong_test) " "[ FAILED ]\033[0m"; \
				FAIL=1; \
			else \
				printf "%-60b%b\n" "\033[32m$$(basename $$wrong_test) " "[ PASSED ]\033[0m"; \
			fi; \
		done; \
		if [ -n "$$FAIL" ]; then \
			exit 1; \
		fi; \
	else \
		EXT=$$(echo $(FILE) | sed 's/.*\.\([^.]*\)/\1/'); \
		OUTPUT=$$( ../semantic_analyser < "$$FILE" ); \
		echo "$$OUTPUT"; \
		if [ "$$EXT" = "correct" ]; then \
			if (echo "$$OUTPUT" | grep -q "error" - ); then \
				printf "%-60b%b\n" "\033[31m$$(basename $(FILE)) " "[ FAILED ]\033[0m"; \
				FAIL=1; \
			else \
				printf "%-60b%b\n" "\033[32m$$(basename $(FILE)) " "[ PASSED ]\033[0m"; \
			fi; \
		else \
			if !(echo "$$OUTPUT" | grep -q "error" - ); then \
				printf "%-60b%b\n" "\033[31m$$(basename $(FILE)) " "[ FAILED ]\033[0m"; \
				FAIL=1; \
			else \
				printf "%-60b%b\n" "\033[32m$$(basename $(FILE)) " "[ PASSED ]\033[0m"; \
			fi; \
		fi; \
	fi;
