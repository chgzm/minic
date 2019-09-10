#include "tokenizer.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

static TokenVec* tokenvec_create() {
    TokenVec* vec = malloc(sizeof(TokenVec));
    vec->tokens   = malloc(sizeof(Token*) * 1024);
    vec->capacity = 1024;
    vec->size     = 0;

    return vec;
}

static void tokenvec_push_back(TokenVec* vec, Token* token) {
    if (vec->size == (vec->capacity - 1)) {
        vec->capacity *= 2;
        vec->tokens = realloc(vec->tokens, sizeof(Token*) * vec->capacity);
    }

    vec->tokens[vec->size] = token; 
    ++(vec->size);
}

static bool starts_with(char* s1, char* s2) {
    return (strncmp(s1, s2, strlen(s2)) == 0);
}

static bool is_symbol(char p) {
    switch (p) {
    case '+': case '-': 
    case '*': case '/': 
    case '%': case '=': 
    case ';': case ':':
    case '(': case ')': 
    case '{': case '}': 
    case '[': case ']': 
    case '<': case '>': 
    case '!': case '&': 
    case '^': case '|': 
    case '#': case ',': 
    case '.': {
        return true;
    }
    default: {
        return false;
    }
    }
}

TokenVec* tokenize(void* addr) {
    TokenVec* vec = tokenvec_create();

    int pos = 0;
    const char* p = (const char*)(addr);
    while (p[pos]) {
        if (isspace(p[pos])) {
            ++pos;
        }
        else if (p[pos] == '\'') {
            Token* token = read_character(p, &pos);
            if (token == NULL) {
                fprintf(stderr, "Failed to read character.\n");
                return NULL; 
            }   
            tokenvec_push_back(vec, token); 
        }
        else if (p[pos] == '"') {
            Token* token = read_string(p, &pos);
            if (token == NULL) {
                fprintf(stderr, "Failed to read string.\n");
                return NULL; 
            }   
            tokenvec_push_back(vec, token); 
        }
        else if (is_symbol(p[pos])) {
            Token* token = read_symbol(p, &pos); 
            if (token == NULL) {
                fprintf(stderr, "Failed to read symbol.\n");
                return NULL; 
            }   
            tokenvec_push_back(vec, token); 
        }
        else if (isalpha(p[pos]) || p[pos] == '_') {
            Token* token = read_identifier(p, &pos);
            if (token == NULL) {
                fprintf(stderr, "Failed to read identifier.\n");
                return NULL; 
            }   
            tokenvec_push_back(vec, token); 
        }
        else if (isdigit(p[pos])) {
            Token* token = read_number(p, &pos);
            if (token == NULL) {
                fprintf(stderr, "Failed to read number.\n");
                return NULL; 
            }   
            tokenvec_push_back(vec, token); 
        }
        else {
            fprintf(stderr, "Invalid character='%c'\n", p[pos]);
            return NULL;
        } 
    }

    return vec;
}

static Token* read_character(const char* p, int* pos) {
    ++(*pos);

    char c = p[*pos];
    ++(*pos);
    if (c == '\\') {
        c = p[*pos];
        ++(*pos);
    }

    if (p[*pos] != '\'') {
        fprintf(stderr, "char is not closed.\n");
        return NULL;
    }
    ++(*pos);

    Token* token = calloc(1, sizeof(Token));
    token->type = TK_NUM;
    token->num  = c;
    
    return token;
}

static Token* read_string(const char* p, int* pos) {
    return NULL;
}

static Token* read_symbol(const char* p, int* pos) {
    Token* token = calloc(1, sizeof(Token));

    const char f = p[*pos];
    ++(*pos);
    switch (f) {
    case '+': {
        const char s = p[*pos];
        switch (s) {
        case '+': {
            token->type = TK_INC;
            ++(*pos);
            return token;
        }
        case '=': {
            token->type = TK_ADD_EQ;
            ++(*pos);
            return token;
        }
        default: {
            token->type = TK_PLUS;
            return token;
        }
        }
    } 
    case '-': {
        const char s = p[*pos];
        switch (s) {
        case '-': {
            token->type = TK_DEC;
            ++(*pos);
            return token;
        }
        case '=': {
            token->type = TK_SUB_EQ;
            ++(*pos);
            return token;
        }
        default: {
            token->type = TK_MINUS;
            return token;
        }
        }
    } 
    case '*': {
        const char s = p[*pos];
        switch (s) {
        case '=': {
            token->type = TK_MUL_EQ;
            ++(*pos);
            return token;
        }
        default: {
            token->type = TK_ASTER;
            return token;
        }
        }
    } 
    case '/': {
        const char s = p[*pos];
        switch (s) {
        case '=': {
            token->type = TK_MUL_EQ;
            ++(*pos);
            return token;
        }
        default: {
            token->type = TK_ASTER;
            return token;
        }
        }
    } 
    case '%': {
        const char s = p[*pos];
        switch (s) {
        case '=': {
            token->type = TK_MOD_EQ;
            ++(*pos);
            return token;
        }
        default: {
            token->type = TK_PER;
            return token;
        }
        }
    } 
    case '=': {
        const char s = p[*pos];
        switch (s) {
        case '=': {
            token->type = TK_EQ;
            ++(*pos);
            return token;
        }
        default: {
            token->type = TK_ASSIGN;
            return token;
        }
        }
    } 
    case ';': {
        token->type = TK_SEMICOL;
        return token;
    } 
    case ':': {
        token->type = TK_COLON;
        return token;
    } 
    case '(': {
        token->type = TK_LPAREN;
        return token;
    } 
    case ')': {
        token->type = TK_RPAREN;
        return token;
    } 
    case '{': {
        token->type = TK_LBRCKT;
        return token;
    } 
    case '}': {
        token->type = TK_RBRCKT;
        return token;
    } 
    case '[': {
        token->type = TK_LSQUARE;
        return token;
    } 
    case ']': {
        token->type = TK_RSQUARE;
        return token;
    } 
    case '<': {
        const char s = p[*pos];
        switch (s) {
        case '<': {
            token->type = TK_LSH;
            ++(*pos);
            return token;
        }
        default: {
            token->type = TK_LANGLE;
            return token;
        }
        }
    } 
    case '>': {
        const char s = p[*pos];;
        switch (s) {
        case '>': {
            token->type = TK_RSH;
            ++(*pos);
            return token;
        }
        default: {
            token->type = TK_RANGLE;
            return token;
        }
        }
    } 
    case '!': {
        token->type = TK_EXCLA;
        return token;
    } 
    case '&': {
        const char s = p[*pos];
        switch (s) {
        case '&': {
            token->type = TK_LOGAND;
            ++(*pos);
            return token;
        }
        case '=': {
            token->type = TK_AND_EQ;
            ++(*pos);
            return token;
        }
        default: {
            token->type = TK_AMP;
            return token;
        }
        }
    } 
    case '^': {
        const char s = p[*pos];
        switch (s) {
        case '=': {
            token->type = TK_XOR_EQ;
            ++(*pos);
            return token;
        }
        default: {
            token->type = TK_HAT;
            return token;
        }
        }
    } 
    case '|': {
        const char s = p[*pos];
        switch (s) {
        case '=': {
            token->type = TK_OR_EQ;
            ++(*pos);
            return token;
        }
        default: {
            token->type = TK_PIPE;
            return token;
        }
        }
    } 
    case '#': {
        token->type = TK_HASH;
        return token;
    } 
    case ',': {
        token->type = TK_COMMA;
        return token;
    } 
    case '.': {
        token->type = TK_DOT;
        return token;
    } 
    default: {
        return NULL;
    }
    }
}

static Token* read_identifier(const char* p, int* pos) {
    Token* token = calloc(1, sizeof(Token));
    token->type = TK_IDENT; // Default

    int len = 0;
    while (isdigit(p[*pos + len]) || isalpha(p[*pos + len]) || p[*pos + len] == '_') {
        ++len;
    }

    //
    // Check if string equals to keywords.
    // For speed, avoid comparing with all keywords by using head character.
    // 
    switch (p[*pos]) {
    case 'i': {
        if (len != 2 && len != 3) {
            break;
        }

        if (strncmp("int", &p[*pos], 3) == 0) {
            token->type = TK_INT;
        }
        else if (strncmp("if", &p[*pos], 2) == 0) {
            token->type = TK_IF;
        }

        break;
    }
    case 'c': {
        if (len != 4 && len != 8) {
            break;
        }

        if (strncmp("char", &p[*pos], 4) == 0) {
            token->type = TK_CHAR;
        }
        else if (strncmp("case", &p[*pos], 4) == 0) {
            token->type = TK_CASE;
        }
        else if (strncmp("continue", &p[*pos], 8) == 0) {
            token->type = TK_CONTINUE;
        }

        break;
    }
    case 'v': {
        if (len != 4) {
            break;
        }

        if (strncmp("void", &p[*pos], 4) == 0) {
            token->type = TK_VOID;
        }

        break;
    }
    case 's': {
        if (len != 6) {
            break;
        }

        if (strncmp("struct", &p[*pos], 6) == 0) {
            token->type = TK_STRUCT;
        }
        else if (strncmp("switch", &p[*pos], 6) == 0) {
            token->type = TK_SWITCH;
        }
    
        break;
    }
    case 'e': {
        if (len != 4) {
            break;
        }

        if (strncmp("else", &p[*pos], 4) == 0) {
            token->type = TK_ELSE;
        }

        break;
    }
    case 'f': {
        if (len != 3) {
            break;
        }

        if (strncmp("for", &p[*pos], 3) == 0) {
            token->type = TK_FOR;
        }

        break;
    }
    case 'w': {
        if (len != 5) {
            break;
        }

        if (strncmp("while", &p[*pos], 5) == 0) {
            token->type = TK_WHILE;
        }

        break;
    }
    case 'b': {
        if (len != 5) {
            break;
        }

        if (strncmp("break", &p[*pos], 5) == 0) {
            token->type = TK_BREAK;
        }
        break;
    }
    case 'r': {
        if (len != 6) {
            break;
        }

        if (strncmp("return", &p[*pos], 6) == 0) {
            token->type = TK_RETURN;
        }
        break;
    }
    default: {
        break;
    }
    }
        
    if (token->type == TK_IDENT) {
        token->str = calloc(1, sizeof(char) * len + 1);
        strncpy(token->str, &p[*pos], len);  
    }

    *(pos) += len;

    return token;
}

static Token* read_number(const char* p, int* pos) {
    Token* token = calloc(1, sizeof(Token));
    token->type = TK_NUM;
 
    int len = 0;
    while (isdigit(p[*pos + len])) {
        ++len;
    }

    char buf[len + 1];
    strncpy(buf, &p[*pos], len);
    buf[len] = '\0';

    token->num = atoi(buf);
    *pos += len;

    return token;
}
