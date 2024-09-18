# Lexical analyser
## How to build:
- In the Lex directory, run `make build`. This will run flex on the lexer.l file.
## How to run:
- In the Lex directory, run `make run`. This will create a testing lexical analyser which takes in text as input and prints the token types.
## How to run tests:
- `make test` will run all the tests in the Tests directory and output the result. All the tests should pass.
- you can also run `make test FILE=test_name`, which will run the test 'test_name.in' and give the result.
## Adding tests:
- If 'test_name' is the name of your test, create two files in the 'Tests' directory: 'test_name.in' and 'test_name.out'. The .in file must contain the input program, and the .out file must contain the expected output.
