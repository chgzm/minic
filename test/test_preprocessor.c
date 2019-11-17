#define NULL  0
#define bool  int
#define false 0
#define true  1
#define O_RDONLY 0
#define FILE void
#define stdout 1
#define stderr 2
#define SEEK_SET 0
#define SEEK_END 2

typedef struct Vector Vector;
struct Vector {
    void** elements;
    int    size;
    int    capacity;
};

Vector* create_vector() {
    Vector* vec = malloc(sizeof(Vector));
    vec->elements  = malloc(sizeof(void*) * 16);
    vec->capacity  = 16;
    vec->size      = 0;

    return vec;
}

void vector_push_back(Vector* vec, void* e) {
    if (vec->size == vec->capacity) {
        vec->capacity *= 2;
        vec->elements = realloc(vec->elements, sizeof(void*) * vec->capacity);
    }

    vec->elements[vec->size] = e;
    ++(vec->size);
}

void error(const char* fmt, ...) {
    printf("%s", fmt);
}

typedef struct StrPtrMapEntry StrPtrMapEntry;
struct StrPtrMapEntry {
    char*            key;
    void*            val;
    StrPtrMapEntry* next;
};

typedef struct StrPtrMap StrPtrMap;
struct StrPtrMap {
    StrPtrMapEntry** entries;
    int               size;
    int               capacity;
};

StrPtrMap* create_strptrmap(int capacity) {
    StrPtrMap* map = malloc(sizeof(StrPtrMap));
    map->size      = 0;
    map->capacity  = capacity;
    map->entries   = calloc(capacity, sizeof(StrPtrMapEntry*));

    return map;
}

int calc_hash(const char* str) {
    int h = 0, pos = 0;
    while (str[pos] != '\0') {
        h += str[pos];
        ++pos;
    }

    return h;
}

void strptrmap_put(StrPtrMap* map, const char* key, void* val) {
    const int hash  = calc_hash(key);
    const int index = hash % map->capacity;
    if (map->entries[index] == NULL) {
        map->entries[index]       = calloc(1, sizeof(StrPtrMapEntry));
        map->entries[index]->key  = strdup(key);
        map->entries[index]->val  = val;
        map->entries[index]->next = NULL;
    }
    else {
        StrPtrMapEntry* current = map->entries[index];
        while (current->next != NULL) {
            current = current->next;
        }
        current->next       = calloc(1, sizeof(StrPtrMapEntry));
        current->next->key  = strdup(key);
        current->next->val  = val;
        current->next->next = NULL;
    }

    ++(map->size);
}

bool strptrmap_contains(StrPtrMap* map, const char* key) {
    const int hash  = calc_hash(key);
    const int index = hash % map->capacity;

    if (map->entries[index] == NULL) {
        return false;
    }
    else {
        StrPtrMapEntry* current = map->entries[index];
        while (current != NULL) {
            if (strcmp(current->key, key) == 0) {
                return true;
            }
            current = current->next;
        }

        return false;
    }
}

void* strptrmap_get(StrPtrMap* map, const char* key) {
    const int hash  = calc_hash(key);
    const int index = hash % map->capacity;
    if (map->entries[index] == NULL) {
        return NULL;
    }
    else {
        StrPtrMapEntry* current = map->entries[index];
        while (current != NULL) {
            if (strcmp(current->key, key) == 0) {
                return current->val;
            }

            current = current->next;
        }

        return NULL;
    }
}

char* read_file(const char* file_path) {
    FILE* fp = fopen(file_path, "r");
    if (fp == NULL) {
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    int fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* addr = calloc(fsize, sizeof(char) + 1);
    fread(addr, sizeof(char), fsize, fp);
    fclose(fp);

    return addr;
}

enum TokenType {
    TK_NUM,       // number literal
    TK_BYTE,      // char literal
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
    TK_BS,        // '\'
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
    TK_ELLIPSIS   // ...
};

typedef struct Token Token;
struct Token {
    int    type;
    int    num;
    char*  str;
    int    strlen;
    bool   has_value; // true if token is "#define" and has value (#define key value)
};

bool is_symbol(char p) {
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

Token* read_directive(const char* p, int* pos) {
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

Token* read_character(const char* p, int* pos) {
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

Token* read_string(const char* p, int* pos) {
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

Token* read_symbol(const char* p, int* pos) {
    Token* token = calloc(1, sizeof(Token));

    const char f = p[*pos];
    ++(*pos);
    const char s = p[*pos];
    switch (f) {
    case '+': {
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

Token* read_identifier(const char* p, int* pos) {
    Token* token = calloc(1, sizeof(Token));
    token->type = TK_IDENT; 

    int len = 0;
    while (isdigit(p[*pos + len]) || isalpha(p[*pos + len]) || p[*pos + len] == '_') {
        ++len;
    }

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

Token* read_number(const char* p, int* pos) {
    Token* token = calloc(1, sizeof(Token));
    token->type = TK_NUM;

    int len = 0;
    while (isdigit(p[*pos + len])) {
        ++len;
    }

    char buf[32];
    strncpy(buf, &p[*pos], 32);
    buf[len] = '\0';
    token->num = atoi(buf);
    *pos += len;

    return token;
}

bool is_line_comment(const char* p, int pos) {
    return (strncmp(&p[pos], "//", 2) == 0);
}

bool is_block_comment_begin(const char* p, int pos) {
    return (strncmp(&p[pos], "/*", 2) == 0);
}

bool is_block_comment_end(const char* p, int pos) {
    return (strncmp(&p[pos], "*/", 2) == 0);
}

Vector* tokenize(char* addr) {
    Vector* vec = create_vector();

    int pos = 0;
    const char* p = addr;
    Token* token = NULL;
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
            token = read_directive(p, &pos);
            if (token == NULL) {
                error("Failed to read directive.\n");
                return NULL;
            }
            vector_push_back(vec, token);
        }
        else if (p[pos] == '\'') {
            token = read_character(p, &pos);
            if (token == NULL) {
                error("Failed to read character.\n");
                return NULL;
            }
            vector_push_back(vec, token);
        }
        else if (p[pos] == '"') {
            token = read_string(p, &pos);
            if (token == NULL) {
                error("Failed to read string.\n");
                return NULL;
            }
            vector_push_back(vec, token);
        }
        else if (is_symbol(p[pos])) {
            token = read_symbol(p, &pos);
            if (token == NULL) {
                error("Failed to read symbol.\n");
                return NULL;
            }
            vector_push_back(vec, token);
        }
        else if (isalpha(p[pos]) || p[pos] == '_') {
            token = read_identifier(p, &pos);
            if (token == NULL) {
                error("Failed to read identifier.\n");
                return NULL;
            }
            vector_push_back(vec, token);
        }
        else if (isdigit(p[pos])) {
            token = read_number(p, &pos);
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

enum State {
    STATE_ENABLED,
    STATE_DISABLED,
    STATE_INVALID,
};

StrPtrMap* define_map;
Vector* preprocess(const Vector* in_vec);

int enabled_read(const Vector* in_vec, int* index, Vector* out_vec) {
    Token* token = in_vec->elements[*index];
    if (token->type != TK_HASH) {
        if (token->type == TK_IDENT && strptrmap_contains(define_map, token->str)) {
            Token* value = strptrmap_get(define_map, token->str);

            if (value != NULL) {
                vector_push_back(out_vec, value);
            }

            ++(*index);
            return STATE_ENABLED;
        }
        else {
            vector_push_back(out_vec, token);
            ++(*index);
            return STATE_ENABLED;
        }
    }
    else {
        //
        // #define
        //
        if (strncmp("define", token->str, 6) == 0) {
            ++(*index);
            const Token* name = in_vec->elements[*index];
            ++(*index);

            if (token->has_value) {
                Token* value = in_vec->elements[*index];
                strptrmap_put(define_map, name->str, value);
                ++(*index);

                return STATE_ENABLED;
            }
            else  {
                strptrmap_put(define_map, name->str, NULL);
                return STATE_ENABLED;
            }
        }
        //
        // #include
        //
        else if (strncmp("include", token->str, 7) == 0) { 
            ++(*index);

            token = in_vec->elements[*index];
            // FIXME: skip include <...> 
            if (token->type == TK_LANGLE) {
                while (token->type != TK_RANGLE) {
                    ++(*index);
                    token = in_vec->elements[*index];
                }

                ++(*index);
                return STATE_ENABLED; 
            } 
            else if (token->type == TK_STR) {
                char* addr = read_file(token->str);
                if (addr == NULL) {
                    error("Failed to load file:\"%s\".\n", token->str); 
                    return STATE_INVALID;
                }

                const Vector* include_vec = tokenize(addr);
                if (include_vec == NULL) {
                    error("Failed to tokenize.\n");
                    return STATE_INVALID;
                }

                const Vector* include_processed_vec = preprocess(include_vec);
                if (include_processed_vec == NULL) {
                    error("Failed to preprocess.\n");
                    return STATE_INVALID;
                }

                for (int i = 0; i < include_processed_vec->size; ++i) {
                    vector_push_back(out_vec, include_processed_vec->elements[i]);
                }

                ++(*index);
            }
            else {
                return STATE_INVALID;
            }

            return STATE_ENABLED;
        } 
        //
        // #ifdef
        //
        else if (strncmp("ifdef", token->str, 5) == 0) {
            ++(*index);

            token = in_vec->elements[*index];
            if (token->type != TK_IDENT) {
                return STATE_INVALID;
            }
            ++(*index);

            if (strptrmap_contains(define_map, token->str)) {
                return STATE_ENABLED;
            } else {
                return STATE_DISABLED;
            }
        }
        //
        // #ifndef
        //
        else if (strncmp("ifndef", token->str, 6) == 0) {
            ++(*index);

            token = in_vec->elements[*index];
            if (token->type != TK_IDENT) {
                return STATE_INVALID;
            }
            ++(*index);

            if (strptrmap_contains(define_map, token->str)) {
                return STATE_DISABLED;
            } else {
                return STATE_ENABLED;
            }
        }
        //
        // #else
        //
        else if (strncmp("else", token->str, 4) == 0) {
            ++(*index);
            return STATE_DISABLED; 
        }
        //
        // #endif
        //
        else if (strncmp("endif", token->str, 5) == 0) {
            ++(*index);
            return STATE_ENABLED;
        }
        else {
            // @todo
            ++(*index);
            return STATE_ENABLED;
        }
    }
}

int disabled_read(const Vector* in_vec, int* index, Vector* out_vec) {
    const Token* token = in_vec->elements[*index];

    ++(*index);
    if (token->type != TK_HASH) {
        return STATE_DISABLED;
    }
    else if (strncmp("else", token->str, 4) == 0) {
        return STATE_ENABLED; 
    }
    else if (strncmp("endif", token->str, 5) == 0) {
        return STATE_ENABLED;
    } else {
        return STATE_DISABLED;
    }
}

Vector* preprocess(const Vector* in_vec) {
    Vector* out_vec = create_vector();
    if (define_map == NULL) {
        define_map = create_strptrmap(32);
    }

    int state = STATE_ENABLED;
    int index = 0;
    while (index < in_vec->size) {   
        Token* token = in_vec->elements[index];
        switch (state) {
        case STATE_ENABLED: {
            state = enabled_read(in_vec, &index, out_vec);
            if (state == STATE_INVALID) {
                return NULL;
            }
            break;
        }
        case STATE_DISABLED: {
            state = disabled_read(in_vec, &index, out_vec);
            if (state == STATE_INVALID) {
                return NULL;
            }
            break;
        }
        default: {
            error("Invalid state=%d\n", state);
            return NULL;
        }
        }
    }

    return out_vec;
}

int main() {
    char* str = malloc(sizeof(char) * 256);
    strncpy(str, "int main() { return 0; }\0", 256);

    Vector* tokens    = tokenize(str);
    Vector* processed = preprocess(tokens);

    int ret = 0;
    for (int i = 0; i < processed->size; ++i) {
        Token* token = processed->elements[i];
        ret += token->type;
    }

    return ret;
}

