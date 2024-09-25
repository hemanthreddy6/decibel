CORRECT_TESTS := $(wildcard *.correct)
WRONG_TESTS := $(wildcard *.wrong)

CORRECT_INPUT_FILES := $(if $(FILE), $(FILE), $(CORRECT_TESTS))
WRONG_INPUT_FILES := $(if $(FILE), $(FILE), $(WRONG_TESTS))

test: $(CORRECT_INPUT_FILES)
	for correct_test in $(CORRECT_INPUT_FILES); do \
		if (../parser < $$correct_test | grep "syntax error" - ); then \
			echo "\033[31m$$(basename $$correct_test) [ FAILED ]\033[0m"; \
			FAIL=1; \
		else \
			echo "\033[32m$$(basename $$correct_test) [ PASSED ]\033[0m"; \
		fi; \
	done;
	for wrong_test in $(WRONG_INPUT_FILES); do \
		if !(../parser < $$wrong_test | grep "syntax error" - > /dev/null); then \
			echo "\033[31m$$(basename $$wrong_test) [ FAILED ]\033[0m"; \
			FAIL=1; \
		else \
			echo "\033[32m$$(basename $$wrong_test) [ PASSED ]\033[0m"; \
		fi; \
	done;
