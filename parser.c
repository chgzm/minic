#include "parser.h"

#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 

static ExprNode* create_expr_node(TokenVec* vec, int* index);

#if 0
static bool is_unary_operator(TokenVec* vec, int index) {
    Token* token = vec->tokens[index]; 
    if (token->type == TK_AMP 
     || token->type == TK_ASTER || token->type == TK_PLUS
     || token->type == TK_MINUS
     || token->type == TK_TILDE
     || token->type == TK_EXCLA
    ) {
        return true;
    } 
    else {
        return false;
    }
}
#endif

static ConstantNode* create_constant_node(TokenVec* vec, int* index) {
    ConstantNode* constant_node = malloc(sizeof(ConstantNode));

    const Token* token = vec->tokens[*index];
    ++(*index);
    
    switch (token->type) {
    case TK_NUM: {
        constant_node->const_type       = CONST_INT;
        constant_node->integer_constant = token->num;
        break; 
    }
    case TK_STR: {
        constant_node->const_type       = CONST_STR;
        strncpy(constant_node->character_constant, token->str, token->strlen);
        break; 
    }
    default: {
        error("Invalid token type=\"%s\".\n", decode_token_type(token->type));
        return NULL;
    }
    }
    
    return constant_node;
}      

static PrimaryExprNode* create_primary_expr_node(TokenVec* vec, int* index) {
    PrimaryExprNode* primary_expr_node = malloc(sizeof(PrimaryExprNode));
    primary_expr_node->constant_node = NULL;
    primary_expr_node->expr_node     = NULL;
    primary_expr_node->string        = NULL;
    primary_expr_node->identifier    = NULL;

    const Token* token = vec->tokens[*index];
    switch (token->type) {
    case TK_NUM:
    case TK_STR: {
        primary_expr_node->constant_node = create_constant_node(vec, index);
        if (primary_expr_node->constant_node == NULL) {
            error("Failed to create constant node.\n");
            return NULL;
        }

        break;
    }
    case TK_LPAREN: {
        ++(*index);
        primary_expr_node->expr_node = create_expr_node(vec, index);
        if (primary_expr_node->expr_node == NULL) {
            error("Failed to create expression-node.\n");
            return NULL;
        }
        
        Token* rparen_token = vec->tokens[*index];
        if (rparen_token->type != TK_RPAREN) {
            error("Invalid token type=\"%s\"\n", decode_token_type(rparen_token->type));
            return NULL; 
        }
        ++(*index);
         
        break;
    }
    default: {
        error("Invalid token type=\"%s\"\n", decode_token_type(token->type));
        return NULL;
    }
    }

    return primary_expr_node;
}

static PostfixExprNode* create_postfix_expr_node(TokenVec* vec, int* index) {
    PostfixExprNode* postfix_expr_node = malloc(sizeof(PostfixExprNode));
    postfix_expr_node->primary_expr_node    = NULL;
    postfix_expr_node->postfix_expr_node    = NULL;
    postfix_expr_node->expr_node            = NULL;
    postfix_expr_node->assignment_expr_node = NULL;
    postfix_expr_node->identifier           = NULL;

    postfix_expr_node->primary_expr_node = create_primary_expr_node(vec, index);
    if (postfix_expr_node->primary_expr_node == NULL) {
        error("Failed to create primary-expression node.\n");
        return NULL;
    }

    return postfix_expr_node;
}

static UnaryExprNode* create_unary_expr_node(TokenVec* vec, int* index) {
    UnaryExprNode* unary_expr_node = malloc(sizeof(UnaryExprNode));
    unary_expr_node->postfix_expr_node = NULL;
    unary_expr_node->unary_expr_node   = NULL;
    unary_expr_node->cast_expr_node    = NULL;

    unary_expr_node->postfix_expr_node = create_postfix_expr_node(vec, index);
    if (unary_expr_node->postfix_expr_node == NULL) {
        error("Failed to create postfix-expression node.\n");
        return NULL;
    }
    
    return unary_expr_node;
}

static CastExprNode* create_cast_expr_node(TokenVec* vec, int* index) {
    CastExprNode* cast_expr_node = malloc(sizeof(CastExprNode));
    cast_expr_node->unary_expr_node = NULL;
    cast_expr_node->cast_expr_node  = NULL;

    cast_expr_node->unary_expr_node = create_unary_expr_node(vec, index);
    if (cast_expr_node->unary_expr_node == NULL) {
        error("Failed to create unary-expression node.\n");
        return NULL;
    }

    return cast_expr_node;
}

static MultiPlicativeExprNode* create_multiplicative_expr_node(TokenVec* vec, int* index) {
    MultiPlicativeExprNode* multiplicative_expr_node = malloc(sizeof(MultiPlicativeExprNode));
    multiplicative_expr_node->operator_type            = OP_NONE;
    multiplicative_expr_node->cast_expr_node           = NULL;
    multiplicative_expr_node->multiplicative_expr_node = NULL;
    multiplicative_expr_node->cast_expr_node           = create_cast_expr_node(vec, index);
    if (multiplicative_expr_node->cast_expr_node == NULL) {
        error("Failed to create cast-expression node.\n");
        return NULL;
    }

    MultiPlicativeExprNode* current = multiplicative_expr_node;
    const Token* token = vec->tokens[*index];
    while (token->type == TK_ASTER || token->type == TK_SLASH || token->type == TK_PER) {
        ++(*index);

        MultiPlicativeExprNode* p_multiplicative_expr_node = malloc(sizeof(MultiPlicativeExprNode));
        p_multiplicative_expr_node->multiplicative_expr_node = current;
        p_multiplicative_expr_node->cast_expr_node           = create_cast_expr_node(vec, index);
        if (p_multiplicative_expr_node->cast_expr_node == NULL) {
            error("Failed to create cast-expression node.\n");
            return NULL; 
        }

        if      (token->type == TK_ASTER) { p_multiplicative_expr_node->operator_type = OP_MUL; }
        else if (token->type == TK_SLASH) { p_multiplicative_expr_node->operator_type = OP_DIV; }
        else if (token->type == TK_PER)   { p_multiplicative_expr_node->operator_type = OP_MOD; }

        token = vec->tokens[*index];
        current = p_multiplicative_expr_node;
    }

    return current;
}

static AdditiveExprNode* create_additive_expr_node(TokenVec* vec, int* index) {
    AdditiveExprNode* additive_expr_node = malloc(sizeof(AdditiveExprNode));
    additive_expr_node->operator_type            = OP_NONE;
    additive_expr_node->multiplicative_expr_node = NULL;
    additive_expr_node->additive_expr_node       = NULL;
    additive_expr_node->multiplicative_expr_node = create_multiplicative_expr_node(vec, index);
    if (additive_expr_node->multiplicative_expr_node == NULL) {
        error("Failed to create multiplicative-expression node.\n");
        return NULL;
    }

    AdditiveExprNode* current = additive_expr_node;
    const Token* token = vec->tokens[*index];
    while (token->type == TK_PLUS || token->type == TK_MINUS) {
        ++(*index);

        AdditiveExprNode* p_additive_expr_node = malloc(sizeof(AdditiveExprNode));
        p_additive_expr_node->operator_type            = (token->type == TK_PLUS) ? OP_ADD: OP_SUB;
        p_additive_expr_node->additive_expr_node       = current;
        p_additive_expr_node->multiplicative_expr_node = create_multiplicative_expr_node(vec, index);
        if (p_additive_expr_node->multiplicative_expr_node == NULL) {
            error("Failed to create multiplicative-expression node.\n");
            return NULL;
        }

        token= vec->tokens[*index];
        current = p_additive_expr_node;
    }

    return current;
}

static ShiftExprNode* create_shift_expr(TokenVec* vec, int* index) {
    ShiftExprNode* shift_expr_node = malloc(sizeof(ShiftExprNode));
    shift_expr_node->additive_expr_node = NULL;
    shift_expr_node->shift_expr_node    = NULL;
 
    shift_expr_node->additive_expr_node = create_additive_expr_node(vec, index);
    if (shift_expr_node->additive_expr_node == NULL) {
        error("Failed to additive-expression node.\n");
        return NULL;
    }

    return shift_expr_node;
} 

static RelationalExprNode* create_relational_expr_node(TokenVec* vec, int* index) {
    RelationalExprNode* relational_expr_node = malloc(sizeof(RelationalExprNode));
    relational_expr_node->shift_expr_node      = NULL;
    relational_expr_node->relational_expr_node = NULL;

    relational_expr_node->shift_expr_node = create_shift_expr(vec, index);
    if (relational_expr_node->shift_expr_node == NULL) {
        error("Failed to create shift-expression node.\n");
        return NULL;
    }

    return relational_expr_node;
}

static EqualityExprNode* create_equality_expr_node(TokenVec* vec, int* index) {
    EqualityExprNode* equality_expr_node = malloc(sizeof(EqualityExprNode));
    equality_expr_node->equality_expr_node = NULL;

    equality_expr_node->relational_expr_node = create_relational_expr_node(vec, index);
    if (equality_expr_node->relational_expr_node == NULL) {
        error("Failed to create relational-expression node.\n");
        return NULL;
    }

    return equality_expr_node;
}

static AndExprNode* create_and_expr_node(TokenVec* vec, int* index) {
    AndExprNode* and_expr_node = malloc(sizeof(AndExprNode));
    and_expr_node->equality_expr_node = NULL;
    and_expr_node->and_expr_node      = NULL;

    and_expr_node->equality_expr_node = create_equality_expr_node(vec, index);
    if (and_expr_node->equality_expr_node == NULL) {
        error("Failed to create equality-expression node.\n");
        return NULL;
    }

    return and_expr_node;
}

static ExclusiveOrExprNode* create_exclusive_or_expr_node(TokenVec* vec, int* index) {
    ExclusiveOrExprNode* exclusive_or_expr_node = malloc(sizeof(ExclusiveOrExprNode));
    exclusive_or_expr_node->and_expr_node          = NULL;
    exclusive_or_expr_node->exclusive_or_expr_node = NULL;

    exclusive_or_expr_node->and_expr_node = create_and_expr_node(vec, index);
    if (exclusive_or_expr_node->and_expr_node == NULL) {
        error("Failed to create and-expression node.\n");
        return NULL;
    }

    return exclusive_or_expr_node;
} 

static InclusiveOrExprNode* create_inclusive_or_expr_node(TokenVec* vec, int* index) {    
    InclusiveOrExprNode* inclusive_or_expr_node = malloc(sizeof(InclusiveOrExprNode));
    inclusive_or_expr_node->exclusive_or_expr_node = NULL;
    inclusive_or_expr_node->inclusive_or_expr_node = NULL;
  
    inclusive_or_expr_node->exclusive_or_expr_node = create_exclusive_or_expr_node(vec, index);
    if (inclusive_or_expr_node->exclusive_or_expr_node  == NULL) {
        error("Failed to create exclusive-or-expression node.\n");
        return NULL;
    }

    return inclusive_or_expr_node;  
}

static LogicalAndExprNode* create_logical_and_expr_node(TokenVec* vec, int* index) {
    LogicalAndExprNode* logical_and_expr_node = malloc(sizeof(LogicalAndExprNode));
    logical_and_expr_node->inclusive_or_expr_node = NULL;
    logical_and_expr_node->logical_and_expr_node  = NULL;

    logical_and_expr_node->inclusive_or_expr_node = create_inclusive_or_expr_node(vec, index);
    if (logical_and_expr_node->inclusive_or_expr_node == NULL) {
        error("Failed to create inclusive-or-expression node.\n");
        return NULL;
    }

    return logical_and_expr_node;
} 

static LogicalOrExprNode* create_logical_or_expr_node(TokenVec* vec, int* index) {
    LogicalOrExprNode* logical_or_expr_node = malloc(sizeof(LogicalOrExprNode));
    logical_or_expr_node->logical_or_expr_node  = NULL;
    logical_or_expr_node->logical_and_expr_node = NULL;
  
    logical_or_expr_node->logical_and_expr_node = create_logical_and_expr_node(vec, index);
    if (logical_or_expr_node->logical_and_expr_node == NULL) {
        error("Failed to create logical-and-expression node.\n");
        return NULL;
    }
    
    return logical_or_expr_node;
}  

static ConditionalExprNode* create_conditional_expr_node(TokenVec* vec, int* index) {
    ConditionalExprNode* conditional_expr_node = malloc(sizeof(ConditionalExprNode));
    conditional_expr_node->logical_or_expr_node  = NULL;
    conditional_expr_node->logical_and_expr_node = NULL;
    conditional_expr_node->conditional_expr_node = NULL;
   
    conditional_expr_node->logical_or_expr_node = create_logical_or_expr_node(vec, index);
    if (conditional_expr_node->logical_or_expr_node == NULL) {
        error("Failed to create logical-or-expression node.\n");
        return NULL;
    }

    return conditional_expr_node; 
}

static AssignExprNode* create_assign_expr_node(TokenVec* vec, int* index) {
    AssignExprNode* assign_expr_node = malloc(sizeof(AssignExprNode));
    assign_expr_node->conditional_expr_node = NULL;
    assign_expr_node->unary_expr_node       = NULL;
    assign_expr_node->assign_expr_node      = NULL;
    assign_expr_node->assign_operator       = OP_NONE;
  
    const int buf = *index;
    assign_expr_node->unary_expr_node = create_unary_expr_node(vec, index);
    const Token* token = vec->tokens[*index];
    if (token->type == TK_ASSIGN || token->type == TK_MUL_EQ || token->type == TK_DIV_EQ 
     || token->type == TK_MOD_EQ || token->type == TK_ADD_EQ || token->type == TK_SUB_EQ 
     || token->type == TK_AND_EQ || token->type == TK_XOR_EQ || token->type == TK_OR_EQ) {
        if      (token->type == TK_ASSIGN) { assign_expr_node->assign_operator = OP_ASSIGN; }
        else if (token->type == TK_MUL_EQ) { assign_expr_node->assign_operator = OP_MUL_EQ; }
        else if (token->type == TK_DIV_EQ) { assign_expr_node->assign_operator = OP_DIV_EQ; }
        else if (token->type == TK_MOD_EQ) { assign_expr_node->assign_operator = OP_MOD_EQ; }
        else if (token->type == TK_ADD_EQ) { assign_expr_node->assign_operator = OP_ADD_EQ; }
        else if (token->type == TK_SUB_EQ) { assign_expr_node->assign_operator = OP_SUB_EQ; }
        else if (token->type == TK_AND_EQ) { assign_expr_node->assign_operator = OP_AND_EQ; }
        else if (token->type == TK_XOR_EQ) { assign_expr_node->assign_operator = OP_XOR_EQ; }
        else if (token->type == TK_OR_EQ)  { assign_expr_node->assign_operator = OP_OR_EQ;  } 
 
        assign_expr_node->assign_expr_node = create_assign_expr_node(vec, index);
        if (assign_expr_node->assign_expr_node == NULL) {
            error("Failed to create assign-expression node.\n");
            return NULL;
        }
    } else {
        assign_expr_node->unary_expr_node = NULL;
        *index = buf;

        assign_expr_node->conditional_expr_node = create_conditional_expr_node(vec, index);
        if (assign_expr_node->conditional_expr_node == NULL) {
            error("Failed to create conditional-expression node.\n");
            return NULL;
        }
    }

    return assign_expr_node; 
}

static ExprNode* create_expr_node(TokenVec* vec, int* index) {
    ExprNode* expr_node = malloc(sizeof(ExprNode));
    expr_node->assign_expr_node = NULL;
    expr_node->expr_node        = NULL;

    expr_node->assign_expr_node = create_assign_expr_node(vec, index);
    if (expr_node->assign_expr_node == NULL) {
        error("Failed to create assignment-expression node.\n");
        return NULL;
    }
    
    ExprNode* current = expr_node; 
    const Token* token = vec->tokens[*index];
    while (token->type == TK_COMMA) {
        ++(*index);
        ExprNode* p_expr_node = malloc(sizeof(ExprNode));
        p_expr_node->expr_node        = current;
        p_expr_node->assign_expr_node = create_assign_expr_node(vec, index);
        if (p_expr_node->assign_expr_node == NULL) {
            error("Failed to create assing-expression node.\n");
            return NULL;
        }

        token = vec->tokens[*index];
        current = p_expr_node;
    }
   
    return current;
}

static ReturnNode* create_return_node(TokenVec* vec, int* index) {
    ReturnNode* return_node = malloc(sizeof(ReturnNode));

    if (vec->tokens[*index]->type != TK_SEMICOL) {
        return_node->expr = create_expr_node(vec, index);  
        if (return_node->expr == NULL) {
            error("Failed to create expression node.\n");
            return NULL;
        }
    }

    const Token* token = vec->tokens[*index];
    ++(*index);

    if (token->type != TK_SEMICOL) {
        error("Invalid token type=\"%s\".\n", decode_token_type(token->type));
        return NULL;
    }

    return return_node;
}

static JumpStmtNode* create_jump_stmt_node(TokenVec* vec, int* index) {
    JumpStmtNode* jump_stmt_node = malloc(sizeof(JumpStmtNode));

    const Token* token = vec->tokens[*index];
    ++(*index);

    switch (token->type) {
    case TK_RETURN: {
        jump_stmt_node->ret = create_return_node(vec, index);
        if (jump_stmt_node->ret == NULL) {
            error("Failed to create return node.\n");
            return NULL;
        }

        break;
    }
    default: {
        error("Invalid token type=\"%s\".\n", decode_token_type(token->type));
        return NULL;
    }
    }  

    return jump_stmt_node;
}

static ExprStmtNode* create_expr_stmt_node(TokenVec* vec, int* index) {
    ExprStmtNode* expr_stmt_node = malloc(sizeof(ExprStmtNode)); 
   
    const Token* token = vec->tokens[*index];
    if (token->type == TK_SEMICOL) {
        expr_stmt_node->expr_node = NULL;
        ++(*index);
    }
    else {
        expr_stmt_node->expr_node = create_expr_node(vec, index);
        if (expr_stmt_node->expr_node == NULL) {
            error("Failed to create expression-node.\n");
            return NULL;
        }
        token = vec->tokens[*index];
        if (token->type != TK_SEMICOL) {
            error("Invalid token type=\"%s\".\n", decode_token_type(token->type));
            return NULL;
        }
        ++(*index);
    }

    return expr_stmt_node;
}

static StmtNode* create_stmt_node(TokenVec* vec, int* index) {
    StmtNode* stmt_node = malloc(sizeof(StmtNode));

    const Token* token = vec->tokens[*index];

    switch (token->type) {
    case TK_RETURN: {
        stmt_node->jump_stmt_node = create_jump_stmt_node(vec, index);
        if (stmt_node->jump_stmt_node == NULL) {
            error("Failed to create jump-statement node.\n");
            return NULL;
        }

        break;
    }
    default: {
        stmt_node->expr_stmt_node = create_expr_stmt_node(vec, index);
        if (stmt_node->expr_stmt_node == NULL) {
            error("Failed to create expression-statement node.\n");
            return NULL;
        }

        break;
    }
    }  

    return stmt_node;
}

static bool is_declaration(TokenVec* vec, int index) {
    const Token* token = vec->tokens[index];

    return (token->type == TK_AUTO
         || token->type == TK_REGISTER
         || token->type == TK_STATIC
         || token->type == TK_EXTERN
         || token->type == TK_TYPEDEF
         || token->type == TK_VOID
         || token->type == TK_CHAR
         || token->type == TK_SHORT
         || token->type == TK_INT
         || token->type == TK_LONG
         || token->type == TK_FLOAT
         || token->type == TK_DOUBLE
         || token->type == TK_SIGNED
         || token->type == TK_UNSIGNED
         || token->type == TK_STRUCT
         || token->type == TK_UNION
         || token->type == TK_CONST
         || token->type == TK_VOLATILE
    );
}

static CompoundStmtNode* create_compound_stmt_node(TokenVec* vec, int* index) {
    CompoundStmtNode* compound_stmt_node = malloc(sizeof(CompoundStmtNode));

    // {
    {
        const Token* token = vec->tokens[*index];
        if (token->type != TK_LBRCKT) {
            error("Invalid token type=\"%s\".\n", decode_token_type(token->type));
            return NULL;
        }
        ++(*index);
    }

    // <declaration>*
    {
        while (is_declaration(vec, *index)) {

        }
    }

    // <statement>*
    {
        while (vec->tokens[*index]->type != TK_RBRCKT) {
            compound_stmt_node->stmt_nodes = create_ptr_vector();
            StmtNode* stmt_node = create_stmt_node(vec, index);   
            if (stmt_node == NULL) {
                error("Failed to create statement node.\n");
                return NULL;
            }

            ptr_vector_push_back(compound_stmt_node->stmt_nodes, (void*)(stmt_node));
        }
    }    

    // }
    {
        const Token* token = vec->tokens[*index];
        if (token->type != TK_RBRCKT) {
            error("Invalid token type=\"%s\".\n", decode_token_type(token->type));
            return NULL;
        }
        ++(*index);
    }

    return compound_stmt_node;
}

static FuncDefNode* create_func_def_node(TokenVec* vec, int* index) {   
    FuncDefNode* func_def_node = malloc(sizeof(FuncDefNode));

    // type specifier
    {
        const Token* token = vec->tokens[*index]; 
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
            error("Invalid token type=%d\n", token->type);
            return NULL;
        }
        }

        ++(*index);
    }

    // identifier
    {
        const Token* token = vec->tokens[*index];
        if (token->type != TK_IDENT) {
            error("Invalid token type=%d\n", token->type);
            return NULL;
        }
        func_def_node->identifier = malloc(sizeof(char) * token->strlen);
        strncpy(func_def_node->identifier, token->str, token->strlen);
    
        ++(*index);
    }

    // (
    {
        const Token* token = vec->tokens[*index];
        if (token->type != TK_LPAREN) {
            error("Invalid token type=%d\n", token->type);
            return NULL;
        }
        ++(*index);
    }

    // @todo parameter list

    // )
    {
        const Token* token = vec->tokens[*index];
        if (token->type != TK_RPAREN) {
            error("Invalid token type=%d\n", token->type);
            return NULL;
        }
        ++(*index);
    }

    func_def_node->compound_stmt_node = create_compound_stmt_node(vec, index);   
    if (func_def_node->compound_stmt_node == NULL) {
        error("Failed to create compound-statement node\n");
        return NULL;
    }
  
    return func_def_node;
}

static ExternalDeclNode* create_external_decl_node(TokenVec* vec, int* index) {
    ExternalDeclNode* external_decl_node = malloc(sizeof(ExternalDeclNode));
    external_decl_node->declaration_node = NULL;

    external_decl_node->func_def_node = create_func_def_node(vec, index);
    if (external_decl_node->func_def_node == NULL) {
        error ("Failed to create function-definition node.\n");
        return NULL;
    }

    return external_decl_node;
}

static TransUnitNode* create_trans_unit_node() {
    TransUnitNode* trans_unit_node = malloc(sizeof(TransUnitNode));
    trans_unit_node->external_decl_nodes = create_ptr_vector();

    return trans_unit_node;
}

TransUnitNode* parse(TokenVec* vec) {
    TransUnitNode* trans_unit_node = create_trans_unit_node();
    
    int index = 0;
    while (index < vec->size) {
        ExternalDeclNode* external_decl_node = create_external_decl_node(vec, &index);
        if (external_decl_node == NULL) {
            error("Failed to create external-declaration node.\n");
            return NULL;
        }
       
        ptr_vector_push_back(trans_unit_node->external_decl_nodes, external_decl_node);
    }
           
    return trans_unit_node;
}
