#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdio.h>
#include <stdbool.h>

enum TokenType {
    TK_NUM,       // number literal
    TK_STR,       // string literal
    TK_IDENT,     // identifier
    TK_AUTO,      // "auto"
    TK_REGISTER,  // "register"
    TK_STATIC,    // "static"
    TK_EXTERN,    // "extern"
    TK_TYPEDEF,   // "typedef"
    TK_VOID,      // "void"
    TK_CHAR,      // "char"
    TK_SHORT,     // "short"
    TK_INT,       // "int"
    TK_LONG,      // "long"
    TK_FLOAT,     // "float"
    TK_DOUBLE,    // "double"
    TK_SIGNED,    // "signed"
    TK_UNSIGNED,  // "unsigned"
    TK_STRUCT,    // "struct"
    TK_UNION,     // "union"
    TK_ENUM,      // "enum"
    TK_CONST,     // "const"
    TK_VOLATILE,  // "volatile"
    TK_IF,        // "if"
    TK_ELSE,      // "else"
    TK_FOR,       // "for"
    TK_WHILE,     // "while"
    TK_DO,        // "do"
    TK_SWITCH,    // "switch"
    TK_CASE,      // "case"
    TK_GOTO,      // "goto"
    TK_BREAK,     // "break"
    TK_CONTINUE,  // "continue"
    TK_RETURN,    // "return"
    TK_PLUS,      // +
    TK_MINUS,     // -
    TK_ASTER,     // *
    TK_SLASH,     // /
    TK_PER,       // %
    TK_ASSIGN,    // =
    TK_SEMICOL,   // ;
    TK_COLON,     // :
    TK_LPAREN,    // (
    TK_RPAREN,    // )
    TK_LBRCKT,    // {
    TK_RBRCKT,    // }
    TK_LSQUARE,   // [
    TK_RSQUARE,   // ]
    TK_LANGLE,    // <
    TK_RANGLE,    // >
    TK_EXCLA,     // !
    TK_AMP,       // &
    TK_HAT,       // ^
    TK_TILDE,     // ~
    TK_PIPE,      // |
    TK_HASH,      // #
    TK_COMMA,     // ,
    TK_DOT,       // .
    TK_ARROW,     // ->
    TK_EQ,        // ==
    TK_NE,        // !=
    TK_LE,        // <=
    TK_GE,        // >=
    TK_LOGOR,     // ||
    TK_LOGAND,    // &&
    TK_LSH,       // <<
    TK_RSH,       // >>
    TK_INC,       // ++
    TK_DEC,       // --
    TK_MUL_EQ,    // *=
    TK_DIV_EQ,    // /=
    TK_MOD_EQ,    // %=
    TK_ADD_EQ,    // +=
    TK_SUB_EQ,    // -=
    TK_AND_EQ,    // &=
    TK_XOR_EQ,    // ^=
    TK_OR_EQ,     // |=
    TK_EOF,       // EOF
};

struct Token {
    int type;
    int num;
    char* str;
    int strlen;
};
typedef struct Token Token;

struct TokenVec {
    Token** tokens;     
    int size; 
    int capacity;
};
typedef struct TokenVec TokenVec;

int get_token_type(const TokenVec* vec, int index);

TokenVec* tokenize(void* addr);

//
// debug
//

const char* decode_token_type(int type);

#endif
