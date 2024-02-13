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

<top-level>       ::= "fn" IDENTIFIER "(" (IDENTIFIER ("," IDENTIFIER)*)? ")" "{" (<stmt>;*) "}"
                    | <stmt>

<stmt>            ::= "{" (<stmt>;*) "}"
                    | "print" <expr>
                    | "let" IDENTIFIER ":=" <expr>
                    | IDENTIFIER ":=" <expr>
                    | <if>
					| "while" "(" <expr> ")" <stmt>
                    | "ret" <expr>?
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
                    | IDENTIFIER "(" (<expr> ("," <expr>)*)? ")"
                    | "input"
                    | "(" TYPE ")" <primary>
                    | "(" <expr> ")"

<comment>         ::= // ...
                    | /* ... */
```

## Example

```rust
//Print n'th fibonacci number
fn fib(n)
{
    if(n <= 1) 
    {
        ret n;
    }
    else 
    {
        ret fib(n-1) + fib(n-2);
    };
};

//Set max equal to user input
let max := (int)(input);

//Print all the "max" fist fibonacci numbers
let x := 1;
while(x <= max) 
{
    print fib(x);
    x := x + 1;
};
```
