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
                    | <if>
                    | <expr>

<if>              ::= "if" "(" <expr> ")" <stmt>
                    | "if" "(" <expr> ")" <stmt> "else" <stmt>

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
                    | "input"
                    | "(" TYPE ")" <primary>
                    | "(" <expr> ")"

<comment>         ::= // ...
                    | /* ... */
```

## Example

```rust
let inp := (int)input;

if(inp < 10) {
	print "Your input was less than 10";
} else if(inp < 20) {
	print "Your input was between 10-19";
} else {
	print "Your input was greater than 19";
};

print "Your input multiplied by 64.3 is:";
print (float)inp * 64.3;
```
