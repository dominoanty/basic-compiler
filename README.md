# basic-compiler
Project to create a basic compiler (front-end) using LLVM backend.


## Lexer.cpp
This provides the lexer and the scanner used for the parser. The tokens are defined in this file. 

## AST.cpp
This module is how the final AST should look like for code generation. Consists of different kind of statements, expressions and nodes for function definitions. 

## Parser.cpp
A handwritten recursive descent parser for the grammar defined for the language PL/0. Currently parses top level main and function definitions and recursively parses the defined statements. At the base only assignment statements are defined. 


### Pending changes
Add I/O Parsing
Add unary operators
Add Code Generation either through LLVM / C++ 
