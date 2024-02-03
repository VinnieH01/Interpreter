# Interpreter

## About
This is a Lexer, Parser and Interpreter for a simple custom programming language.

## Filestructure
Path                                    | Comment
--------------------------------------- | -------------
`/src`                                  |  The main folder for the code.
`/spec`                                 | This folder contains language specification files such as it's grammar

## Specification
For the most up to date specifications see `/spec` 

### Grammar
```html
<program>         ::= (<stmt> ";")*

<stmt>            ::= "{" <program> "}"
                    | "print" <expr>
                    | "let" IDENTIFIER ":=" <expr>
                    | "if" "(" <expr> ")" <stmt>
                    | <expr>

<expr>            ::= <logic>

<logic>           ::= <comparison> ("&&" | "||") <logic>
                    | <comparison>

<comparison>      ::= <product> (">" | "<" | "==" | ">=" | "<=") <sum>
                    | <sum>
					
<sum>             ::= <product> ("+" | "-") <sum>
                    | <product>
					
<product>         ::= <unary> ("*" | "/") <product>
                    | <unary>
					
<unary>           ::= "-" <unary>
                    | <primary>
					
<primary>         ::= LITERAL
                    | IDENTIFIER
                    | "(" <expr> ")"
```

