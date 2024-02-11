# Interpreter

## About
This is a Lexer, Parser and Interpreter for a simple custom programming language.

## Filestructure
Path                                    | Comment
--------------------------------------- | -------------
`/src`                                  | The main folder for the code.
`/spec`                                 | This folder contains language specification files such as its grammar

## Specification
For the most up to date specifications see `/spec` 

### Grammar
```html
<program>         ::= (<top-level> ";")*

<top-level>       ::= "fn" IDENTIFIER "(" ")" "{" (<stmt>;*) "}"
                    | <stmt>

<stmt>            ::= "{" (<stmt>;*) "}"
                    | IDENTIFIER "(" ")"
                    | "print" <expr>
                    | "let" IDENTIFIER ":=" <expr>
                    | IDENTIFIER ":=" <expr>
                    | <if>
                    | "while" "(" <expr> ")" <stmt>
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
fn foo() 
{
	let x := 1;
	print inp + (string)(x * 1);
	print inp + (string)(x * 2);
	print inp + (string)(x * 3);
};

let inp := input;

if((int)inp < 10) let x := 2 else let x := 4;

while(x > 0) 
{
	foo();
	x := x - 1;
};
```
