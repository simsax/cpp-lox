/*
- Statements

declaration    → classDecl | funDecl | varDecl | statement ;
statement      → exprStmt | forStmt | ifStmt | printStmt | returnStmt | whileStmt | block ;

- Expressions

expression     → assignment ;
assignment     → (call ".")? IDENTIFIER "=" assignment | logic_or ;
logic_or       → logic_and ( "or" logic_and )* ;
logic_and      → equality ( "and" equality )* ;
equality       → comparison ( ( "!=" | "==" ) comparison )* ;
comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
term           → factor ( ( "-" | "+" ) factor )* ;
factor         → unary ( ( "/" | "*" ) unary )* ;
unary          → ( "!" | "-" ) unary | call ;
call           → primary ( "(" arguments? ")" | "." IDENTIFIER )* ;
arguments	   → expression ("," expression)* ;
primary        → NUMBER | STRING | "true" | "false" | "nil" | "this"
                           | "(" expression ")" | IDENTIFIER
                           | "super" "." IDENTIFIER ;
*/

#ifndef clox_compiler_h
#define clox_compiler_h

#include "chunk.h"

bool compile(const char* source, Chunk* chunk);

#endif