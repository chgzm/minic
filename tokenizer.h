#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdio.h>
#include <stdbool.h>

enum TokenType {
    TK_NUM,       // number literal
    TK_STR,       // string literal
    TK_IDENT,     // identifier
    TK_INT,       // "int"
    TK_CHAR,      // "char"
    TK_VOID,      // "void"
    TK_STRUCT,    // "struct"
    TK_IF,        // "if"
    TK_ELSE,      // "else"
    TK_FOR,       // "for"
    TK_WHILE,     // "while"
    TK_SWITCH,    // "switch"
    TK_CASE,      // "case"
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
    TK_PIPE,      // |
    TK_HASH,      // #
    TK_COMMA,     // ,
    TK_DOT,       // .
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

static const char* keywords[] = {
    "int", "char", "void", "struct", "if", "else", "for", "while", 
    "switch", "case", "break", "continue", "return"
};

struct Token {
    int type;
    int num;
    char* str;
};
typedef struct Token Token;

struct TokenVec {
    Token** tokens;     
    int size; 
    int capacity;
};
typedef struct TokenVec TokenVec;

TokenVec* tokenize(void* addr);

static TokenVec* tokenvec_create();
static void tokenvec_push_back(TokenVec* tokenVec, Token* token);

static Token* read_character(const char* p, int* pos);
static Token* read_string(const char* p, int* pos);
static Token* read_symbol(const char* p, int* pos);
static Token* read_identifier(const char* p, int* pos);
static Token* read_number(const char* p, int* pos);

static bool is_symbol(char p);

#endif
