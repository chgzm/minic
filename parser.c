#include "parser.h"

#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 

#include "util.h"

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

static bool is_unary_operator(TokenVec* vec, int index) {
    Token* token = vec->tokens[index]; 
    if (token->type == TK_AMP 
     || token->type == TK_ASTER 
     || token->type == TK_PLUS
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

static bool is_assign_expr(TokenVec* vec, int index) {
    return true;
}

static ConstantNode* create_constant_node(TokenVec* vec, int* index) {
    ConstantNode* constant_node = malloc(sizeof(ConstantNode));
    constant_node->node_type = ND_CONSTANT;

    Token* token = vec->tokens[*index];
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
    primary_expr_node->node_type     = ND_PRIMARY_EXPR;
    primary_expr_node->constant_node = NULL;
    primary_expr_node->expr_node     = NULL;
    primary_expr_node->string        = NULL;
    primary_expr_node->identifier    = NULL;


    primary_expr_node->constant_node = create_constant_node(vec, index);
    if (primary_expr_node->constant_node == NULL) {
        error("Failed to create constant node.\n");
        return NULL;
    }

    return primary_expr_node;
}

static PostfixExprNode* create_postfix_expr_node(TokenVec* vec, int* index) {
    PostfixExprNode* postfix_expr_node = malloc(sizeof(PostfixExprNode));
    postfix_expr_node->node_type            = ND_POSTFIX_EXPR;
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
    unary_expr_node->node_type = ND_UNARY_EXPR;
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
    cast_expr_node->node_type       = ND_CAST_EXPR;
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
    multiplicative_expr_node->node_type                = ND_MULTIPLICATIVE_EXPR;
    multiplicative_expr_node->operator_type            = OP_NONE;
    multiplicative_expr_node->cast_expr_node           = NULL;
    multiplicative_expr_node->multiplicative_expr_node = NULL;
    multiplicative_expr_node->cast_expr_node           = create_cast_expr_node(vec, index);
    if (multiplicative_expr_node->cast_expr_node == NULL) {
        error("Failed to create cast-expression node.\n");
        return NULL;
    }

    MultiPlicativeExprNode* current = multiplicative_expr_node;
    Token* token = vec->tokens[*index];
    while (token->type == TK_ASTER || token->type == TK_SLASH || token->type == TK_PER) {
        ++(*index);

        MultiPlicativeExprNode* p_multiplicative_expr_node = malloc(sizeof(MultiPlicativeExprNode));
        p_multiplicative_expr_node->node_type                = ND_MULTIPLICATIVE_EXPR;
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
    additive_expr_node->node_type                = ND_ADDITIVE_EXPR;
    additive_expr_node->operator_type            = OP_NONE;
    additive_expr_node->multiplicative_expr_node = NULL;
    additive_expr_node->additive_expr_node       = NULL;
    additive_expr_node->multiplicative_expr_node = create_multiplicative_expr_node(vec, index);
    if (additive_expr_node->multiplicative_expr_node == NULL) {
        error("Failed to create multiplicative-expression node.\n");
        return NULL;
    }

    AdditiveExprNode* current = additive_expr_node;
    Token* token = vec->tokens[*index];
    while (token->type == TK_PLUS || token->type == TK_MINUS) {
        ++(*index);

        AdditiveExprNode* p_additive_expr_node = malloc(sizeof(AdditiveExprNode));
        p_additive_expr_node->node_type                = ND_ADDITIVE_EXPR;
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
    shift_expr_node->node_type          = ND_SHIFT_EXPR;
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
    relational_expr_node->node_type            = ND_RELATIONAL_EXPR;
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
    equality_expr_node->node_type          = ND_EQUALITY_EXPR;
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
    and_expr_node->node_type          = ND_AND_EXPR;
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
    exclusive_or_expr_node->node_type              = ND_EXCLUSIVE_OR_EXPR;
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
    inclusive_or_expr_node->node_type              = ND_INCLUSIVE_OR_EXPR;
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
    logical_and_expr_node->node_type              = ND_LOGICAL_AND_EXPR;
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
    logical_or_expr_node->node_type             = ND_LOGICAL_OR_EXPR;
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
    conditional_expr_node->node_type             = ND_CONDITIONAL_EXPR;
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
    assign_expr_node->node_type             = ND_ASSIGN_EXPR;
    assign_expr_node->conditional_expr_node = NULL;
    assign_expr_node->unary_expr_node       = NULL;
    assign_expr_node->assign_expr_node      = NULL;
   
    assign_expr_node->conditional_expr_node = create_conditional_expr_node(vec, index);
    if (assign_expr_node->conditional_expr_node == NULL) {
        error("Failed to create conditional-expression node.\n");
        return NULL;
    }

    return assign_expr_node; 
}

static ExprNode* create_expr_node(TokenVec* vec, int* index) {
    ExprNode* expr_node = malloc(sizeof(ExprNode));
    expr_node->node_type        = ND_EXPR;
    expr_node->assign_expr_node = NULL;
    expr_node->expr_node        = NULL;

    if (is_assign_expr(vec, *index)) {
        expr_node->assign_expr_node = create_assign_expr_node(vec, index);
        if (expr_node->assign_expr_node == NULL) {
            error("Failed to create assignment-expression node.\n");
            return NULL;
        }
    } 
    else {
        expr_node->expr_node = create_expr_node(vec, index);
        if (expr_node->expr_node == NULL) {
            error("Failed to create expression node.\n");
            return NULL;
        }

        Token* token = vec->tokens[*index];
        ++(*index);

        if (token->type != TK_COMMA) {
            error("Invalid token type=\"%s\".\n", decode_token_type(token->type));
            return NULL;
        }

        expr_node->assign_expr_node = create_assign_expr_node(vec, index);
        if (expr_node->assign_expr_node == NULL) {
            error("Failed to create assignment-expression node.\n");
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
            error("Failed to create expression node.\n");
            return NULL;
        }
    }

    Token* token = vec->tokens[*index];
    ++(*index);

    if (token->type != TK_SEMICOL) {
        error("Invalid token type=\"%s\".\n", decode_token_type(token->type));
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

static StmtNode* create_stmt_node(TokenVec* vec, int* index) {
    StmtNode* stmt_node = malloc(sizeof(StmtNode));
    stmt_node->node_type = ND_STMT;

    Token* token = vec->tokens[*index];

    switch (token->type) {
    case TK_RETURN: {
        stmt_node->jump_stmt = create_jump_stmt_node(vec, index);
        if (stmt_node->jump_stmt == NULL) {
            error("Failed to create jump-statement node.\n");
            return NULL;
        }

        break;
    }
    default: {
        error("Invalid token type=\"%s\".\n", decode_token_type(token->type));
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
            error("Invalid token type=\"%s\".\n", decode_token_type(token->type));
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
                error("Failed to create statement node.\n");
                return NULL;
            }

            nodevec_push_back(compound_stmt_node->stmt, (void*)(stmt_node));
        }
    }    

    // }
    {
        Token* token = vec->tokens[*index];
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
            error("Invalid token type=%d\n", token->type);
            return NULL;
        }
        }

        ++(*index);
    }

    // identifier
    {
        Token* token = vec->tokens[*index];
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
        Token* token = vec->tokens[*index];
        if (token->type != TK_LPAREN) {
            error("Invalid token type=%d\n", token->type);
            return NULL;
        }
        ++(*index);
    }

    // @todo parameter list

    // )
    {
        Token* token = vec->tokens[*index];
        if (token->type != TK_RPAREN) {
            error("Invalid token type=%d\n", token->type);
            return NULL;
        }
        ++(*index);
    }

    func_def_node->compound_stmt = create_compound_stmt_node(vec, index);   
    if (func_def_node->compound_stmt == NULL) {
        error("Failed to create compound-statement node\n");
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
            error("Failed to create function-definition node.\n");
            return NULL;
        }
       
        nodevec_push_back(trans_unit_node->func_def, func_def_node);
    }
           
    return trans_unit_node;
}

//
// debug
// 

const char* decode_node_type(int node_type) {
    switch (node_type) {
    case ND_TRANS_UNIT:    { return "ND_TRANS_UNIT";    }
    case ND_FUNC_DEF:      { return "ND_FUNC_DEF";      }
    case ND_COMPOUND_STMT: { return "ND_COMPOUND_STMT"; }
    case ND_EXPR:          { return "ND_EXPR";          }
    case ND_RETURN:        { return "ND_RETURN";        }
    case ND_JUMP_STMT:     { return "ND_JUMP_STMT";     }
    case ND_STMT:          { return "ND_STMT";          }
    default:               { return "INVALID";          }
    }
}

