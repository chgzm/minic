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

#if 0
static bool starts_with(char* s1, char* s2) {
    return (strncmp(s1, s2, strlen(s2)) == 0);
}
#endif

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
        token->str    = calloc(1, sizeof(char) * len + 1);
        token->strlen = len + 1;
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

const char* decode_token_type(int type) {
    switch (type) {
    case TK_NUM:      { return "TK_NUM";      }
    case TK_STR:      { return "TK_STR";      }
    case TK_IDENT:    { return "TK_IDENT";    }
    case TK_INT:      { return "TK_INT";      }
    case TK_CHAR:     { return "TK_CHAR";     }
    case TK_VOID:     { return "TK_VOID";     }
    case TK_STRUCT:   { return "TK_STRUCT";   }
    case TK_IF:       { return "TK_IF";       }
    case TK_ELSE:     { return "TK_ELSE";     }
    case TK_FOR:      { return "TK_FOR";      }
    case TK_WHILE:    { return "TK_WHILE";    }
    case TK_SWITCH:   { return "TK_SWITCH";   }
    case TK_CASE:     { return "TK_CASE";     }
    case TK_BREAK:    { return "TK_BREAK";    }
    case TK_CONTINUE: { return "TK_CONTINUE"; }
    case TK_RETURN:   { return "TK_RETURN";   }
    case TK_PLUS:     { return "TK_PLUS";     }
    case TK_MINUS:    { return "TK_MINUS";    }
    case TK_ASTER:    { return "TK_ASTER";    }
    case TK_SLASH:    { return "TK_SLASH";    }
    case TK_PER:      { return "TK_PER";      }
    case TK_ASSIGN:   { return "TK_ASSIGN";   }
    case TK_SEMICOL:  { return "TK_SEMICOL";  }
    case TK_COLON:    { return "TK_COLON";    }
    case TK_LPAREN:   { return "TK_LPAREN";   }
    case TK_RPAREN:   { return "TK_RPAREN";   }
    case TK_LBRCKT:   { return "TK_LBRCKT";   }
    case TK_RBRCKT:   { return "TK_RBRCKT";   }
    case TK_LSQUARE:  { return "TK_LSQUARE";  }
    case TK_RSQUARE:  { return "TK_RSQUARE";  }
    case TK_LANGLE:   { return "TK_LANGLE";   }
    case TK_RANGLE:   { return "TK_RANGLE";   }
    case TK_EXCLA:    { return "TK_EXCLA";    }
    case TK_AMP:      { return "TK_AMP";      }
    case TK_HAT:      { return "TK_HAT";      }
    case TK_PIPE:     { return "TK_PIPE";     }
    case TK_HASH:     { return "TK_HASH";     }
    case TK_COMMA:    { return "TK_COMMA";    } 
    case TK_DOT:      { return "TK_DOT";      }
    case TK_EQ:       { return "TK_EQ";       }
    case TK_NE:       { return "TK_NE";       }
    case TK_LE:       { return "TK_LE";       }
    case TK_GE:       { return "TK_GE";       }
    case TK_LOGOR:    { return "TK_LOGOR";    }
    case TK_LOGAND:   { return "TK_LOGAND";   }
    case TK_LSH:      { return "TK_LSH";      }
    case TK_RSH:      { return "TK_RSH";      }
    case TK_INC:      { return "TK_INC";      }
    case TK_DEC:      { return "TK_DEC";      }
    case TK_MUL_EQ:   { return "TK_MUL_EQ";   }
    case TK_DIV_EQ:   { return "TK_DIV_EQ";   }
    case TK_MOD_EQ:   { return "TK_MOD_EQ";   }
    case TK_ADD_EQ:   { return "TK_ADD_EQ";   }
    case TK_SUB_EQ:   { return "TK_SUB_EQ";   }
    case TK_AND_EQ:   { return "TK_AND_EQ";   }
    case TK_XOR_EQ:   { return "TK_XOR_EQ";   }
    case TK_OR_EQ:    { return "TK_OR_EQ";    }
    case TK_EOF:      { return "TK_EOF";      }
    default:          { return "INVALID";     }
    }
}

void dump_tokens(TokenVec* vec) {
    for (int i = 0; i < vec->size; ++i) {
        fprintf(stdout, "index=%d,token=%s\n", i, decode_token_type(vec->tokens[i]->type));
    }
}