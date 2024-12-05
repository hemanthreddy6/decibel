# Decibel - A Domain-Specific Language for Audio Programming

[![Lexer](https://github.com/hemanthreddy6/decibel/actions/workflows/lexer.yml/badge.svg)](https://github.com/hemanthreddy6/decibel/actions/workflows/lexer.yml)
[![Parser](https://github.com/hemanthreddy6/decibel/actions/workflows/parser.yml/badge.svg)](https://github.com/hemanthreddy6/decibel/actions/workflows/parser.yml)

## Introduction
Decibel is a statically typed, domain-specific programming language designed to simplify audio generation and manipulation. It offers a powerful yet intuitive approach to handle complex audio tasks, including sound generation, mixing, and real-time processing. With its user-friendly syntax and a rich set of operators, Decibel allows both professionals and beginners to create high-quality audio applications effortlessly.

## Unique Selling Points (USP)
- **Intuitive Syntax**: Makes complex audio operations easy to understand with a straightforward syntax.
- **Comprehensive Audio Operations**: Provides powerful operators for audio manipulation, including concatenation, superposition, panning, and more.
- **LLVM Integration**: Leverages LLVM for for efficient code generation to enhance performance.
- **Currying Support**: Allows users to create more flexible and reusable functions by breaking down functions that take multiple parameters into simpler ones with fewer arguments.
- **Advanced Language Design**: Decibel’s unique approach of treating functions as variables and supporting currying gives it a level of versatility and power not seen in most other audio programming languages.
- **Functions as Variables**: Lets you treat functions as first-class variables, offering dynamic behavior and more flexible programming options.
- **Seamless Integration (Future Feature)**: Designed to easily integrate with external programs and files through import statements, with the framework already in place for future implementation.

## How to Run Decibel:
1. Clone the repository
2. Run the following commands:
``` make ``` command builds the project. 
``` make test ``` command runs the tests.

3. Individual components can be run. Change the directory to the respective component and run the following commands:
``` make ``` command builds the individual component.
``` make test ``` command runs the tests.


## Work Done So Far:
- Lexer
- Parser
- Semantic Analysis using Abstract Syntax Tree
- Code Generation using LLVM IR (In Progress)
  
## Future Work:
- Complete Code Generation
- Allow to play the generated audio


## Comparison with Existing Software

While there are several audio programming languages and frameworks currently available, such as **Pure Data**, **Max/MSP**, **Supercollider**, and **Csound**, Decibel differentiates itself by focusing on ease of use and high-level abstraction:

- **Pure Data/Max/MSP**: These platforms offer visual programming, which can be intuitive but require extensive learning, whereas Decibel uses a readable textual approach, easing the learning curve for traditional programmers.
  
- **Supercollider**: While Supercollider is powerful and flexible for sound synthesis, Decibel offers similar capabilities in a more approachable format for beginners.

- **Csound**: Csound's complex syntax can be challenging, but Decibel's simpler and more intuitive syntax enables faster audio project prototyping.

In comparison, Decibel stands out by offering both **simplicity** and **power**, allowing users to focus on audio creation rather than learning complex programming paradigms. It brings together the flexibility of professional audio tools and the user-friendly approach needed for rapid prototyping and development.

