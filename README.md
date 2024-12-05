# decibel
[![Lexer](https://github.com/hemanthreddy6/decibel/actions/workflows/lexer.yml/badge.svg)](https://github.com/hemanthreddy6/decibel/actions/workflows/lexer.yml)
[![Parser](https://github.com/hemanthreddy6/decibel/actions/workflows/parser.yml/badge.svg)](https://github.com/hemanthreddy6/decibel/actions/workflows/parser.yml)

Decibel is a simple, intuitive, statically typed, domain-specific audio programming language designed to simplify audio generation and processing.

## How to Run Decibel:
1. Clone the repository
2. Run the following commands:
``` make ``` command builds the project. 
``` make test ``` command runs the tests.

3. Individual components can be run. Change the directory to the respective component and run the following commands:
``` make ``` command build the individual component.
``` make test ``` command runs the tests.


## Work Done So Far:
- Lexer
- Parser
- Semantic Analysis using Abstract Syntax Tree
- Code Generation using LLVM IR (In Progress)
  
## Future Work:
- Complete Code Generation
- Allow to play the generated audio



