#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static NodeVec* create_nodevec() {
    NodeVec* vec  = malloc(sizeof(NodeVec));
    vec->nodes    = malloc(sizeof(void*) * 16);
    vec->capacity = 16;
    vec->size     = 0;

    return vec;
}

static void nodevec_push_back(NodeVec* vec, void* node) {
    if (vec->size == (vec->capacity - 1)) {
        vec->capacity *= 2;
        vec->nodes = realloc(vec->nodes, sizeof(void*) * vec->capacity);
    }

    vec->nodes[vec->size] = node;
    ++(vec->size);
}

static ExprNode* create_expr_node(TokenVec* vec, int* index) {
    ExprNode* expr_node = malloc(sizeof(ExprNode));
    expr_node->node_type = ND_EXPR;
   
    Token* token = vec->tokens[*index];
    ++(*index);

    switch (token->type) {
    case TK_NUM: {
        expr_node->integer_constant = token->num;
        break;
    }
    default: {
        fprintf(stderr, "Invalid token type=%d\n", token->type);
        return NULL;
    }
    }

    return expr_node;
}

static ReturnNode* create_return_node(TokenVec* vec, int* index) {
    ReturnNode* return_node = malloc(sizeof(ReturnNode));
    return_node->node_type = ND_RETURN;

    if (vec->tokens[*index]->type != TK_SEMICOL) {
        return_node->expr = create_expr_node(vec, index);  
        if (return_node->expr == NULL) {
            fprintf(stderr, "Failed to create expression node.\n");
            return NULL;
        }
    }

    Token* token = vec->tokens[*index];
    ++(*index);

    if (token->type != TK_SEMICOL) {
        fprintf(stderr, "Invalid token type=%d.\n", token->type);
        return NULL;
    }

    return return_node;
}

static JumpStmtNode* create_jump_stmt_node(TokenVec* vec, int* index) {
    JumpStmtNode* jump_stmt_node = malloc(sizeof(JumpStmtNode));
    jump_stmt_node->node_type = ND_JUMP_STMT;

    Token* token = vec->tokens[*index];
    ++(*index);

    switch (token->type) {
    case TK_RETURN: {
        jump_stmt_node->ret = create_return_node(vec, index);
        if (jump_stmt_node->ret == NULL) {
            fprintf(stderr, "Failed to create return node.\n");
            return NULL;
        }

        break;
    }
    default: {
        fprintf(stderr, "Invalid token type=\"%s\".\n", decode_token_type(token->type));
        return NULL;
    }
    }  

    return jump_stmt_node;
}

static StmtNode* create_stmt_node(TokenVec* vec, int* index) {
    StmtNode* stmt_node = malloc(sizeof(StmtNode));
    stmt_node->node_type = ND_STMT;

    Token* token = vec->tokens[*index];

    printf("index=%d\n", *index);

    switch (token->type) {
    case TK_RETURN: {
        stmt_node->jump_stmt = create_jump_stmt_node(vec, index);
        if (stmt_node->jump_stmt == NULL) {
            fprintf(stderr, "Failed to create jump-statement node.\n");
            return NULL;
        }

        break;
    }
    default: {
        fprintf(stderr, "Invalid token type=\"%s\".\n", decode_token_type(token->type));
        return NULL;
    }
    }  

    return stmt_node;
}

static CompoundStmtNode* create_compound_stmt_node(TokenVec* vec, int* index) {
    CompoundStmtNode* compound_stmt_node = malloc(sizeof(CompoundStmtNode));
    compound_stmt_node->node_type = ND_COMPOUND_STMT;

    // {
    {
        Token* token = vec->tokens[*index];
        if (token->type != TK_LBRCKT) {
            fprintf(stderr, "Invalid token type=\"%s\".\n", decode_token_type(token->type));
            return NULL;
        }
        ++(*index);
    }

    // <statement>*
    {
        while (vec->tokens[*index]->type != TK_RBRCKT) {
            compound_stmt_node->stmt = create_nodevec();
            StmtNode* stmt_node = create_stmt_node(vec, index);   
            if (stmt_node == NULL) {
                fprintf(stderr, "Failed to create statement node.\n");
                return NULL;
            }

            nodevec_push_back(compound_stmt_node->stmt, (void*)(stmt_node));
        }
    }    

    // }
    {
        Token* token = vec->tokens[*index];
        if (token->type != TK_RBRCKT) {
            fprintf(stderr, "Invalid token type=\"%s\".\n", decode_token_type(token->type));
            return NULL;
        }
        ++(*index);
    }

    return compound_stmt_node;
}

static FuncDefNode* create_func_def_node(TokenVec* vec, int* index) {   
    FuncDefNode* func_def_node = malloc(sizeof(FuncDefNode));
    func_def_node->node_type = ND_FUNC_DEF;

    // type specifier
    {
        Token* token = vec->tokens[*index]; 
        switch (token->type) {
        case TK_INT: {
            func_def_node->type_specifier = TYPE_INT;
            break;
        }
        case TK_CHAR: {
            func_def_node->type_specifier = TYPE_CHAR;
            break;
        }
        case TK_VOID: {
            func_def_node->type_specifier = TYPE_VOID;
            break;
        }
        default: {
            fprintf(stderr, "Invalid token type=%d\n", token->type);
            return NULL;
        }
        }

        ++(*index);
    }

    // identifier
    {
        Token* token = vec->tokens[*index];
        if (token->type != TK_IDENT) {
            fprintf(stderr, "Invalid token type=%d\n", token->type);
            return NULL;
        }
        func_def_node->identifier = malloc(sizeof(char) * token->strlen);
        strncpy(func_def_node->identifier, token->str, token->strlen);
    
        ++(*index);
    }

    // (
    {
        Token* token = vec->tokens[*index];
        if (token->type != TK_LPAREN) {
            fprintf(stderr, "Invalid token type=%d\n", token->type);
            return NULL;
        }
        ++(*index);
    }

    // @todo parameter list

    // )
    {
        Token* token = vec->tokens[*index];
        if (token->type != TK_RPAREN) {
            fprintf(stderr, "Invalid token type=%d\n", token->type);
            return NULL;
        }
        ++(*index);
    }

    func_def_node->compound_stmt = create_compound_stmt_node(vec, index);   
    if (func_def_node->compound_stmt == NULL) {
        fprintf(stderr, "Failed to create compound-statement node\n");
        return NULL;
    }
  
    return func_def_node;
}

static TransUnitNode* create_trans_unit_node() {
    TransUnitNode* trans_unit_node = malloc(sizeof(TransUnitNode));
    trans_unit_node->node_type = ND_TRANS_UNIT;
    trans_unit_node->func_def  = create_nodevec();

    return trans_unit_node;
}

TransUnitNode* parse(TokenVec* vec) {
    TransUnitNode* trans_unit_node = create_trans_unit_node();
    
    int index = 0;
    while (index < vec->size) {
        FuncDefNode* func_def_node = create_func_def_node(vec, &index);
        if (func_def_node == NULL) {
            fprintf(stderr, "Failed to create function-definition node.\n");
            return NULL;
        }
       
        nodevec_push_back(trans_unit_node->func_def, func_def_node);
    }
           
    return trans_unit_node;
}
