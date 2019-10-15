#include "preprocessor.h"

#include <string.h>

#include "util.h"

static StrHashMap* define_map;

static int enabled_read(const TokenVec* in_vec, int* index, TokenVec* out_vec) {
    Token* token = in_vec->tokens[*index];
    if (token->type != TK_HASH) {
        if (token->type == TK_IDENT && strhashmap_contains(define_map, token->str)) {
            Token* value = strhashmap_get(define_map, token->str);

            if (value != NULL) {
                tokenvec_push_back(out_vec, value);
            }

            ++(*index);
            return STATE_ENABLED;
        }
        else {
            tokenvec_push_back(out_vec, token);
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
            const Token* name = in_vec->tokens[*index];

            ++(*index);
            if (*index == in_vec->size) {
                strhashmap_put(define_map, name->str, NULL);
                return STATE_ENABLED;
            }
            Token* value = in_vec->tokens[*index];

            if (value->type != TK_NUM && value->type != TK_STR) {
                strhashmap_put(define_map, name->str, NULL);
                return STATE_ENABLED;
            }

            strhashmap_put(define_map, name->str, value);
            ++(*index);

            return STATE_ENABLED;
        }
        //
        // #include
        //
        else if (strncmp("include", token->str, 7) == 0) { 
            ++(*index);

            token = in_vec->tokens[*index];
            // FIXME: skip include <...> 
            if (token->type == TK_LANGLE) {
                while (token->type != TK_RANGLE) {
                    ++(*index);
                    token = in_vec->tokens[*index];
                }

                ++(*index);
                return STATE_ENABLED; 
            } 
            else if (token->type == TK_STR) {
                void* addr = mmap_readonly(token->str);
                if (addr == NULL) {
                    error("Failed to load file:\"%s\".\n", token->str); 
                    return STATE_INVALID;
                }

                const TokenVec* include_vec = tokenize(addr);
                if (include_vec == NULL) {
                    error("Failed to tokenize.\n");
                    return STATE_INVALID;
                }

                const TokenVec* include_processed_vec = preprocess(include_vec);
                if (include_processed_vec == NULL) {
                    error("Failed to preprocess.\n");
                    return STATE_INVALID;
                }

                for (int i = 0; i < include_processed_vec->size; ++i) {
                    tokenvec_push_back(out_vec, include_processed_vec->tokens[i]);
                }

                ++(*index);
            }
            else {
                error("Invalid token[%d]=\"%s\".\n", index, decode_token_type(in_vec->tokens[*index]->type));
                return STATE_INVALID;
            }

            return STATE_ENABLED;
        } 
        //
        // #ifndef
        //
        else if (strncmp("ifndef", token->str, 6) == 0) {
            ++(*index);

            token = in_vec->tokens[*index];
            if (token->type != TK_IDENT) {
                error("Invalid token[%d]=\"%s\".\n", index, decode_token_type(in_vec->tokens[*index]->type));
                return STATE_INVALID;
            }
            ++(*index);

            if (strhashmap_contains(define_map, token->str)) {
                return STATE_DISABLED;
            } else {
                return STATE_ENABLED;
            }
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

static int disabled_read(const TokenVec* in_vec, int* index, TokenVec* out_vec) {
    const Token* token = in_vec->tokens[*index];

    ++(*index);
    if (token->type != TK_HASH) {
        return STATE_DISABLED;
    }

    if (strncmp("endif", token->str, 5) != 0) {
        return STATE_DISABLED;
    }

    return STATE_ENABLED;
}

TokenVec* preprocess(const TokenVec* in_vec) {
    TokenVec* out_vec = tokenvec_create();
    define_map = create_strhashmap(32);

    int state = STATE_ENABLED;
    int index = 0;
    while (index < in_vec->size) {   
        switch (state) {
        case STATE_ENABLED: {
            state = enabled_read(in_vec, &index, out_vec);
            if (state == STATE_INVALID) {
                error("Invalid token[%d]=\"%s\".\n", index, decode_token_type(in_vec->tokens[index]->type));
                return NULL;
            }
            break;
        }
        case STATE_DISABLED: {
            state = disabled_read(in_vec, &index, out_vec);
            if (state == STATE_INVALID) {
                error("Invalid token[%d]=\"%s\".\n", index, decode_token_type(in_vec->tokens[index]->type));
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
