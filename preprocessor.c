#include "preprocessor.h"

#include <string.h>

#include "util.h"

static StrPtrMap* define_map;

static int enabled_read(const Vector* in_vec, int* index, Vector* out_vec) {
    Token* token = in_vec->elements[*index];
    if (token->type != TK_HASH) {
        if (token->type == TK_IDENT && strptrmap_contains(define_map, token->str)) {
            Token* value1 = strptrmap_get(define_map, token->str);
            if (value1 != NULL) {
                vector_push_back(out_vec, value1);
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
                Token* value2 = in_vec->elements[*index];
                strptrmap_put(define_map, name->str, value2);
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
                error("Invalid token[%d]=\"%s\".\n", index, decode_token_type(token->type));
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
                error("Invalid token[%d]=\"%s\".\n", index, decode_token_type(token->type));
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
                error("Invalid token[%d]=\"%s\".\n", index, decode_token_type(token->type));
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

static int disabled_read(const Vector* in_vec, int* index, Vector* out_vec) {
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
                error("Invalid token[%d]=\"%s\".\n", index, decode_token_type(token->type));
                return NULL;
            }
            break;
        }
        case STATE_DISABLED: {
            state = disabled_read(in_vec, &index, out_vec);
            if (state == STATE_INVALID) {
                error("Invalid token[%d]=\"%s\".\n", index, decode_token_type(token->type));
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
