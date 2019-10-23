#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdio.h>
#include <stdbool.h>

enum TokenType {
    TK_NUM,       // number literal
    TK_STR,       // string literal
    TK_IDENT,     // identifier
    TK_STATIC,    // "static"
    TK_TYPEDEF,   // "typedef"
    TK_VOID,      // "void"
    TK_CHAR,      // "char"
    TK_INT,       // "int"
    TK_DOUBLE,    // "double"
    TK_STRUCT,    // "struct"
    TK_ENUM,      // "enum"
    TK_CONST,     // "const"
    TK_IF,        // "if"
    TK_ELSE,      // "else"
    TK_FOR,       // "for"
    TK_WHILE,     // "while"
    TK_SWITCH,    // "switch"
    TK_CASE,      // "case"
    TK_DEFAULT,   // "default"
    TK_BREAK,     // "break"
    TK_CONTINUE,  // "continue"
    TK_RETURN,    // "return"
    TK_SIZEOF,    // "sizeof"
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
    TK_QUESTION,  // ?
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
};

typedef struct Token Token;
struct Token {
    int type;
    int num;
    char* str;
    int strlen;
};

typedef struct TokenVec TokenVec;
struct TokenVec {
    Token** tokens;     
    int size; 
    int capacity;
};

TokenVec* tokenvec_create();
int get_token_type(const TokenVec* vec, int index);
void tokenvec_push_back(TokenVec* vec, Token* token);

TokenVec* tokenize(void* addr);

//
// debug
//

const char* decode_token_type(int type);

#endif
