#include "tokenizer.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "util.h"

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
    case '!': case '?': 
    case '&': case '^': 
    case '|': case '#': 
    case ',': case '.': 
    case '\\': {
        return true;
    }
    default: {
        return false;
    }
    }
}

static Token* read_directive(const char* p, int* pos) {
    ++(*pos);

    int len = 0;
    while (p[*pos + len] != ' ' && p[*pos + len] != '\n') {
        ++len;
    }

    Token* token  = calloc(1, sizeof(Token));
    token->type   = TK_HASH;
    token->strlen = len;
    token->str    = malloc(sizeof(char) * token->strlen + 1);
    strncpy(token->str, &p[*pos], token->strlen);
    token->str[token->strlen] = '\0';

    *pos += len; 

    int a = 0;
    while (p[*pos + a] == ' ') {
        ++a;
    }

    while (isalpha(p[*pos + a]) || p[*pos + a] == '_' || isdigit(p[*pos +a])) {
        ++a;
    }

    while (p[*pos + a] == ' ') {
        ++a;
    }

    if (p[*pos + a] != '\n') {
        token->has_value = true;
    }

    return token;
}

static Token* read_character(const char* p, int* pos) {
    ++(*pos);

    char c = p[*pos];
    ++(*pos);
    if (c == '\\') {
        const char s = p[*pos];
        ++(*pos);

        switch (s) {
        case '0': {
            c = '\0';
            break; 
        }
        default: {
            break;
        }
        }        
    }

    if (p[*pos] != '\'') {
        error("char is not closed.\n");
        return NULL;
    }
    ++(*pos);

    Token* token = calloc(1, sizeof(Token));
    token->type = TK_BYTE;
    token->num  = c;

    return token;
}

static Token* read_string(const char* p, int* pos) {
    ++(*pos);
    int len = 0;
    bool escape = false;
    while (true) {
        if (!escape && p[*pos + len] == '"') {
            break;
        }

        if (p[*pos + len] == '\\') { 
            escape = true;
            ++len;
        }
        else {
            escape = false;
            ++len;
        }
    }

    Token* token  = calloc(1, sizeof(Token));
    token->type   = TK_STR;
    token->strlen = len;
    token->str    = malloc(sizeof(char) * token->strlen + 1);
    strncpy(token->str, &p[*pos], token->strlen);
    token->str[token->strlen] = '\0';

    *pos += (len + 1);


    return token;
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
        case '>': {
            token->type = TK_ARROW;
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
            token->type = TK_DIV_EQ;
            ++(*pos);
            return token;
        }
        default: {
            token->type = TK_SLASH;
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
        case '=': {
            token->type = TK_LE;
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
        case '=': {
            token->type = TK_GE;
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
        const char s = p[*pos];
        switch (s) {
        case '=': {
            token->type = TK_NE;
            ++(*pos);
            return token;
        }
        default: {
            token->type = TK_EXCLA;
            return token;
        }
        }
    }
    case '?': {
        token->type = TK_QUESTION;
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
        case '|': {
            token->type = TK_LOGOR;
            ++(*pos);
            return token;
        }
        default: {
            token->type = TK_PIPE;
            return token;
        }
        }
    }
    case ',': {
        token->type = TK_COMMA;
        return token;
    }
    case '.': {
        const char s = p[*pos];
        const char t = p[*pos + 1];
        if (s == '.' && t == '.') {
            token->type = TK_ELLIPSIS;
            *pos += 2;
        } else {
            token->type = TK_DOT;
        }
        return token;
    }
    case '\\': {
        token->type = TK_BS;
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
    case 'b': {
        if (len != 5) {
            break;
        }

        if (strncmp("break", &p[*pos], 5) == 0) {
            token->type = TK_BREAK;
        }

        break;
    }
    case 'c': {
        if (len != 4 && len != 5 && len != 8) {
            break;
        }

        if      (strncmp("char",     &p[*pos], 4) == 0) { token->type = TK_CHAR;     }
        else if (strncmp("case",     &p[*pos], 4) == 0) { token->type = TK_CASE;     }
        else if (strncmp("const",    &p[*pos], 5) == 0) { token->type = TK_CONST;    }
        else if (strncmp("continue", &p[*pos], 8) == 0) { token->type = TK_CONTINUE; }

        break;
    }
    case 'd': {
        if (len != 6 && len != 7) {
            break;
        }

        if      (strncmp("double",  &p[*pos], 6) == 0) { token->type = TK_DOUBLE;  }
        else if (strncmp("default", &p[*pos], 7) == 0) { token->type = TK_DEFAULT; }

        break;
    }
    case 'e': {
        if (len != 4) {
            break;
        }

        if      (strncmp("else",   &p[*pos], 4) == 0) { token->type = TK_ELSE; } 
        else if (strncmp("enum",   &p[*pos], 4) == 0) { token->type = TK_ENUM; }

        break;
    }
    case 'f': {
        if (len != 3) {
            break;
        }

        if (strncmp("for",&p[*pos], 3) == 0) { 
            token->type = TK_FOR;   
        }

        break;
    }
    case 'i': {
        if (len != 2 && len != 3) {
            break;
        }

        if      (strncmp("if",  &p[*pos], 2) == 0) { token->type = TK_IF;  }
        else if (strncmp("int", &p[*pos], 3) == 0) { token->type = TK_INT; }

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
    case 's': {
        if (len != 6) {
            break;
        }

        if      (strncmp("struct", &p[*pos], 6) == 0) { token->type = TK_STRUCT; }
        else if (strncmp("switch", &p[*pos], 6) == 0) { token->type = TK_SWITCH; }
        else if (strncmp("static", &p[*pos], 6) == 0) { token->type = TK_STATIC; }
        else if (strncmp("sizeof", &p[*pos], 6) == 0) { token->type = TK_SIZEOF; }

        break;
    }
    case 't': {
        if (len != 7) {
            break;
        }

        if (strncmp("typedef", &p[*pos], 7) == 0) { 
            token->type = TK_TYPEDEF;
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
    case 'w': {
        if (len != 5) {
            break;
        }

        if (strncmp("while", &p[*pos], 5) == 0) {
            token->type = TK_WHILE;
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
        token->str[len] = '\0';
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

static bool is_line_comment(const char* p, int pos) {
    return (strncmp(&p[pos], "//", 2) == 0);
}

static bool is_block_comment_begin(const char* p, int pos) {
    return (strncmp(&p[pos], "/*", 2) == 0);
}

static bool is_block_comment_end(const char* p, int pos) {
    return (strncmp(&p[pos], "*/", 2) == 0);
}

Vector* tokenize(char* addr) {
    Vector* vec = create_vector();

    int pos = 0;
    const char* p = addr;
    while (p[pos]) {
        if (isspace(p[pos])) {
            ++pos;
        }
        else if (is_line_comment(p, pos)) {
            while (p[pos] != '\n') {
                ++pos;
            } 
        }
        else if (is_block_comment_begin(p, pos)) {
            while (!is_block_comment_end(p, pos)) {
                ++pos;
            }
            pos += 2;
        }
        else if (p[pos] == '#') {
            Token* token = read_directive(p, &pos);
            if (token == NULL) {
                error("Failed to read directive.\n");
                return NULL;
            }
            vector_push_back(vec, token);
        }
        else if (p[pos] == '\'') {
            Token* token = read_character(p, &pos);
            if (token == NULL) {
                error("Failed to read character.\n");
                return NULL;
            }
            vector_push_back(vec, token);
        }
        else if (p[pos] == '"') {
            Token* token = read_string(p, &pos);
            if (token == NULL) {
                error("Failed to read string.\n");
                return NULL;
            }
            vector_push_back(vec, token);
        }
        else if (is_symbol(p[pos])) {
            Token* token = read_symbol(p, &pos);
            if (token == NULL) {
                error("Failed to read symbol.\n");
                return NULL;
            }
            vector_push_back(vec, token);
        }
        else if (isalpha(p[pos]) || p[pos] == '_') {
            Token* token = read_identifier(p, &pos);
            if (token == NULL) {
                error("Failed to read identifier.\n");
                return NULL;
            }
            vector_push_back(vec, token);
        }
        else if (isdigit(p[pos])) {
            Token* token = read_number(p, &pos);
            if (token == NULL) {
                error("Failed to read number.\n");
                return NULL;
            }
            vector_push_back(vec, token);
        }
        else {
            error("Invalid character[%d]='%c'\n", pos, p[pos]);
            return NULL;
        }
    }

    return vec;
}

const char* decode_token_type(int type) {
    switch (type) {
    case TK_NUM:      { return "TK_NUM";      }
    case TK_BYTE:     { return "TK_BYTE";     }
    case TK_STR:      { return "TK_STR";      }
    case TK_IDENT:    { return "TK_IDENT";    }
    case TK_STATIC:   { return "TK_STATIC";   }
    case TK_TYPEDEF:  { return "TK_TYPEDEF";  }
    case TK_VOID:     { return "TK_VOID";     }
    case TK_CHAR:     { return "TK_CHAR";     }
    case TK_INT:      { return "TK_INT";      }
    case TK_DOUBLE:   { return "TK_DOUBLE";   }
    case TK_STRUCT:   { return "TK_STRUCT";   }
    case TK_ENUM:     { return "TK_ENUM";     }
    case TK_CONST:    { return "TK_CONST";    }
    case TK_IF:       { return "TK_IF";       }
    case TK_ELSE:     { return "TK_ELSE";     }
    case TK_FOR:      { return "TK_FOR";      }
    case TK_WHILE:    { return "TK_WHILE";    }
    case TK_SWITCH:   { return "TK_SWITCH";   }
    case TK_CASE:     { return "TK_CASE";     }
    case TK_BREAK:    { return "TK_BREAK";    }
    case TK_CONTINUE: { return "TK_CONTINUE"; }
    case TK_RETURN:   { return "TK_RETURN";   }
    case TK_SIZEOF:   { return "TK_SIZEOF";   }
    case TK_PLUS:     { return "TK_PLUS";     }
    case TK_MINUS:    { return "TK_MINUS";    }
    case TK_ASTER:    { return "TK_ASTER";    }
    case TK_SLASH:    { return "TK_SLASH";    }
    case TK_BS:       { return "TK_BS";       }
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
    case TK_ARROW:    { return "TK_ARROW";    }
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
    case TK_ELLIPSIS: { return "TK_ELLIPSIS"; }
    default:          { return "INVALID";     }
    }
}

int get_token_type(const Vector* vec, int index) {
    Token* token = vec->elements[index];
    return token->type;
}
