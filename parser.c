#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ExprNode* create_expr_node(TokenVec* vec, int* index);
static bool is_declaration(TokenVec* vec, int index);
static bool is_storage_class_specifier(TokenVec* vec, int index);
static bool is_type_qualifier(TokenVec* vec, int index);
static bool is_type_specifier(TokenVec* vec, int index);

#if 0
static bool is_unary_operator(TokenVec* vec, int index) {
    Token* token = vec->tokens[index]; if (token->type == TK_AMP
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
    primary_expr_node->constant_node  = NULL;
    primary_expr_node->expr_node      = NULL;
    primary_expr_node->string         = NULL;
    primary_expr_node->identifier     = NULL;
    primary_expr_node->identifier_len = 0;

    const Token* token = vec->tokens[*index];
    switch (token->type) {
    case TK_IDENT: {
        primary_expr_node->identifier     = malloc(sizeof(char) * token->strlen);
        primary_expr_node->identifier_len = token->strlen;
        strncpy(primary_expr_node->identifier, token->str, token->strlen);
        ++(*index);

        break;
    }
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
    postfix_expr_node->identifier_len       = 0;

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
    if (assign_expr_node->unary_expr_node == NULL) {
        error("Failed to create unary-expression node.\n");
        return NULL;
    }

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

        ++(*index);
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

static JumpStmtNode* create_jump_stmt_node(TokenVec* vec, int* index) {
    JumpStmtNode* jump_stmt_node = malloc(sizeof(JumpStmtNode));
    jump_stmt_node->identifier = NULL;
    jump_stmt_node->expr_node  = NULL;

    const Token* token = vec->tokens[*index];
    ++(*index);

    switch (token->type) {
    case TK_GOTO: {
        break;
    }
    case TK_CONTINUE: {
        break;
    }
    case TK_BREAK: {
        break;
    }
    case TK_RETURN: {
        jump_stmt_node->jump_type = JMP_RETURN;
        if (vec->tokens[*index]->type != TK_SEMICOL) {
            jump_stmt_node->expr_node = create_expr_node(vec, index);
            if (jump_stmt_node->expr_node == NULL) {
                error("Failed to create expression node.\n");
                return NULL;
            }

            if (vec->tokens[*index]->type != TK_SEMICOL) {
                error("Invalid token type=\"%s\".\n", decode_token_type(token->type));
                return NULL;
            }
        }

        ++(*index);
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

static InitializerNode* create_initializer_node(TokenVec* vec, int* index) {
    InitializerNode* initializer_node = malloc(sizeof(InitializerNode));
    initializer_node->initializer_list_node = NULL; // @todo

    initializer_node->assign_expr_node = create_assign_expr_node(vec, index);
    if (initializer_node->assign_expr_node == NULL) {
        error("Failed to create assignment-expression node.\n");
        return NULL;
    }

    return initializer_node;
}

static DirectDeclaratorNode* create_direct_declarator_node(TokenVec* vec, int* index) {
    DirectDeclaratorNode* direct_declarator_node = malloc(sizeof(DirectDeclaratorNode));
    direct_declarator_node->identifier             = NULL;
    direct_declarator_node->identifier_len         = 0;
    direct_declarator_node->declarator_node        = NULL; // @todo
    direct_declarator_node->direct_declarator_node = NULL; // @todo
    direct_declarator_node->constant_expr_node     = NULL; // @todo
    direct_declarator_node->param_type_list_node   = NULL; // @todo
    direct_declarator_node->identifier_list        = NULL; // @todo

    const Token* token = vec->tokens[*index];
    if (token->type == TK_IDENT) {
        direct_declarator_node->identifier     = malloc(sizeof(char) * token->strlen);
        direct_declarator_node->identifier_len = token->strlen;
        strncpy(direct_declarator_node->identifier, token->str, token->strlen);
        ++(*index);
    }

    return direct_declarator_node;
}

static DeclaratorNode* create_declarator_node(TokenVec* vec, int* index) {
    DeclaratorNode* declarator_node = malloc(sizeof(DeclaratorNode));
    declarator_node->pointer_node = NULL; // @todo

    declarator_node->direct_declarator_node = create_direct_declarator_node(vec, index);
    if (declarator_node->direct_declarator_node == NULL) {
        error("Failed to create direct-declarator node.\n");
        return NULL;
    }

    return declarator_node;
}

static InitDeclaratorNode* create_init_declarator_node(TokenVec* vec, int* index) {
    InitDeclaratorNode* init_declarator_node = malloc(sizeof(InitDeclaratorNode));
    init_declarator_node->initializer_node = NULL;

    init_declarator_node->declarator_node = create_declarator_node(vec, index);
    if (init_declarator_node->declarator_node == NULL) {
        error("Failed to create declarator-node.\n");
        return NULL;
    }

    const Token* token = vec->tokens[*index];
    if (token->type == TK_ASSIGN) {
        ++(*index);

        init_declarator_node->initializer_node = create_initializer_node(vec, index);
        if (init_declarator_node->initializer_node == NULL) {
            error("Failed to create initializer node.\n");
            return NULL;
        }
    }

    return init_declarator_node;
}

static TypeQualifierNode* create_type_qualifier_node(TokenVec* vec, int* index) {
    TypeQualifierNode* type_qualifier_node = malloc(sizeof(TypeQualifierNode));

    const Token* token = vec->tokens[*index];
    switch (token->type) {
    case TK_STRUCT:   { type_qualifier_node->type_qualifier = TQ_CONST;    break; }
    case TK_VOLATILE: { type_qualifier_node->type_qualifier = TQ_VOLATILE; break; }
    default: {
        error("Invalid token type=\"%s\"\n", decode_token_type(token->type));
        return NULL;
    }
    }
    ++(*index);

    return type_qualifier_node;
}

static TypeSpecifierNode* create_type_specifier_node(TokenVec* vec, int* index) {
    TypeSpecifierNode* type_specifier_node = malloc(sizeof(TypeSpecifierNode));
    type_specifier_node->type_specifier                 = TYPE_NONE;
    type_specifier_node->struct_or_union_specifier_node = NULL;
    type_specifier_node->enum_specifier_node            = NULL;
    type_specifier_node->typedef_name_node              = NULL;

    const Token* token = vec->tokens[*index];
    switch (token->type) {
    case TK_VOID:     { type_specifier_node->type_specifier = TYPE_VOID;     break; }
    case TK_CHAR:     { type_specifier_node->type_specifier = TYPE_CHAR;     break; }
    case TK_SHORT:    { type_specifier_node->type_specifier = TYPE_SHORT;    break; }
    case TK_INT:      { type_specifier_node->type_specifier = TYPE_INT;      break; }
    case TK_LONG:     { type_specifier_node->type_specifier = TYPE_LONG;     break; }
    case TK_FLOAT:    { type_specifier_node->type_specifier = TYPE_FLOAT;    break; }
    case TK_DOUBLE:   { type_specifier_node->type_specifier = TYPE_DOUBLE;   break; }
    case TK_SIGNED:   { type_specifier_node->type_specifier = TYPE_SIGNED;   break; }
    case TK_UNSIGNED: { type_specifier_node->type_specifier = TYPE_UNSIGNED; break; }
    case TK_STRUCT:   { break; } // @todo
    case TK_UNION:    { break; } // @todo
    case TK_ENUM:     { break; } // @todo
    case TK_TYPEDEF:  { break; } // @todo
    default: {
        error("Invalid token type=\"%s\"\n", decode_token_type(token->type));
        return NULL;
    }
    }
    ++(*index);

    return type_specifier_node;
}

static StorageClassSpecifierNode* create_storage_class_specifier_node(TokenVec* vec, int* index) {
    StorageClassSpecifierNode* storage_class_specifier_node = malloc(sizeof(StorageClassSpecifierNode));
    const Token* token = vec->tokens[*index];
    if      (token->type == TK_AUTO)     { storage_class_specifier_node->storage_class_specifier = SC_AUTO;     }
    else if (token->type == TK_REGISTER) { storage_class_specifier_node->storage_class_specifier = SC_REGISTER; }
    else if (token->type == TK_STATIC)   { storage_class_specifier_node->storage_class_specifier = SC_STATIC;   }
    else if (token->type == TK_EXTERN)   { storage_class_specifier_node->storage_class_specifier = SC_EXTERN;   }
    else if (token->type == TK_TYPEDEF)  { storage_class_specifier_node->storage_class_specifier = SC_TYPEDEF;  }
    else {
        error("Invalid token type=\"%s\"\n", decode_token_type(token->type));
        return NULL;
    }
    ++(*index);

    return storage_class_specifier_node;
}

static DeclSpecifierNode* create_decl_specifier_node(TokenVec* vec, int* index) {
    DeclSpecifierNode* decl_specifier_node = malloc(sizeof(DeclSpecifierNode));
    decl_specifier_node->storage_class_specifier_node = NULL;
    decl_specifier_node->type_specifier_node          = NULL;
    decl_specifier_node->type_qualifier_node          = NULL;

    if (is_storage_class_specifier(vec, *index)) {
        decl_specifier_node->storage_class_specifier_node = create_storage_class_specifier_node(vec, index);
        if (decl_specifier_node->storage_class_specifier_node == NULL) {
            error("Failed to create storage-class-specifier node.\n");
            return NULL;
        }
    }
    else if (is_type_specifier(vec, *index)) {
        decl_specifier_node->type_specifier_node = create_type_specifier_node(vec, index);
        if (decl_specifier_node->type_specifier_node == NULL) {
            error("Failed to create type-specifier node.\n");
            return NULL;
        }
    }
    else if (is_type_qualifier(vec, *index)) {
        decl_specifier_node->type_qualifier_node = create_type_qualifier_node(vec, index);
        if (decl_specifier_node->type_qualifier_node == NULL) {
            error("Failed to create type-qualifier node.\n");
            return NULL;
        }
    }

    return decl_specifier_node;
}

static DeclarationNode* create_declaration_node(TokenVec* vec, int* index) {
    DeclarationNode* declaration_node = malloc(sizeof(DeclarationNode));
    declaration_node->decl_specifier_nodes  = create_ptr_vector();
    declaration_node->init_declarator_nodes = create_ptr_vector();

    while (is_declaration(vec, *index)) {
        DeclSpecifierNode* decl_specifier_node = create_decl_specifier_node(vec, index);
        if (decl_specifier_node == NULL) {
            error("Failed to create declaration-specifier node.\n");
            return NULL;
        }

        ptr_vector_push_back(declaration_node->decl_specifier_nodes, decl_specifier_node);
    }

    const Token* token = vec->tokens[*index];
    while (token->type != TK_SEMICOL) {
        InitDeclaratorNode* init_declarator_node = create_init_declarator_node(vec, index);
        if (init_declarator_node == NULL) {
            error("Failed to create init-declarator node.\n");
            return NULL;
        }

        ptr_vector_push_back(declaration_node->init_declarator_nodes, init_declarator_node);
        token = vec->tokens[*index];
    }
    ++(*index);

    return declaration_node;
}

static bool is_storage_class_specifier(TokenVec* vec, int index) {
    const Token* token = vec->tokens[index];

    return (token->type == TK_AUTO
         || token->type == TK_REGISTER
         || token->type == TK_STATIC
         || token->type == TK_EXTERN
         || token->type == TK_TYPEDEF
    );
}

static bool is_type_qualifier(TokenVec* vec, int index) {
    const Token* token = vec->tokens[index];
    return (token->type == TK_CONST || token->type == TK_VOLATILE);
}

static bool is_type_specifier(TokenVec* vec, int index) {
    const Token* token = vec->tokens[index];

    return (token->type == TK_VOID
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
   );
}

static bool is_declaration(TokenVec* vec, int index) {
    return (is_storage_class_specifier(vec, index)
         || is_type_specifier(vec, index)
         || is_type_qualifier(vec, index)
    );
}

static CompoundStmtNode* create_compound_stmt_node(TokenVec* vec, int* index) {
    CompoundStmtNode* compound_stmt_node = malloc(sizeof(CompoundStmtNode));
    compound_stmt_node->declaration_nodes = create_ptr_vector();
    compound_stmt_node->stmt_nodes = create_ptr_vector();

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
            DeclarationNode* declaration_node = create_declaration_node(vec, index);
            if (declaration_node == NULL) {
                error("Failed to create declaration-node.\n");
                return NULL;
            }

            ptr_vector_push_back(compound_stmt_node->declaration_nodes, (void*)(declaration_node));
        }
    }

    // <statement>*
    {
        while (vec->tokens[*index]->type != TK_RBRCKT) {
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

//
// debug
//
#if 0
void dump_nodes(const TransUnitNode* node);

static void dump_ExternalDeclNode(const ExternalDeclNode* node, int indent);
static void dump_FuncDefNode(const FuncDefNode* node, int indent);
static void dump_DeclSpecifierNode(const DeclSpecifierNode* node, int indent);
static void dump_StorageClassSpecifierNode(const StorageClassSpecifierNode* node, int indent);
static void dump_TypeSpecifierNode(const TypeSpecifierNode* node, int indent);
static void dump_StructOrUnionSpecifierNode(const StructOrUnionSpecifierNode* node, int indent);
static void dump_StructOrUnionNode(const StructOrUnionNode* node, int indent);
static void dump_StructDeclNode(const StructDeclNode* node, int indent);
static void dump_DeclaratorNode(const DeclaratorNode* node, int indent);
static void dump_PointerNode(const PointerNode* node, int indent);
static void dump_TypeQualifierNode(const TypeQualifierNode* node, int indent);
static void dump_DirectDeclaratorNode(const DirectDeclaratorNode* node, int indent);
static void dump_ConstantExprNode(const ConstantExprNode* node, int indent);
static void dump_ConditionalExprNode(const ConditionalExprNode* node, int indent);
static void dump_LogicalOrExprNode(const LogicalOrExprNode* node, int indent);
static void dump_LogicalAndExprNode(const LogicalAndExprNode* node, int indent);
static void dump_InclusiveOrExprNode(const InclusiveOrExprNode* node, int indent);
static void dump_ExclusiveOrExprNode(const ExclusiveOrExprNode* node, int indent);
static void dump_AndExprNode(const AndExprNode* node, int indent);
static void dump_EqualityExprNode(const EqualityExprNode* node, int indent);
static void dump_RelationalExprNode(const RelationalExprNode* node, int indent);
static void dump_ShiftExprNode(const ShiftExprNode* node, int indent);
static void dump_AdditiveExprNode(const AdditiveExprNode* node, int indent);
static void dump_MultiPlicativeExprNode(const MultiPlicativeExprNode* node, int indent);
static void dump_CastExprNode(const CastExprNode* node, int indent);
static void dump_UnaryExprNode(const UnaryExprNode* node, int indent);
static void dump_PostfixExprNode(const PostfixExprNode* node, int indent);
static void dump_PrimaryExprNode(const PrimaryExprNode* node, int indent);
static void dump_ConstantNode(const ConstantNode* node, int indent);
static void dump_ExprNode(const ExprNode* node, int indent);
static void dump_AssignExprNode(const AssignExprNode* node, int indent);
static void dump_TypeNameNode(const TypeNameNode* node, int indent);
static void dump_ParamTypeListNode(const ParamTypeListNode* node, int indent);
static void dump_ParamListNode(const ParamListNode* node, int indent);
static void dump_ParamDeclarationNode(const ParamDeclarationNode* node, int indent);
static void dump_AbstractDeclaratorNode(const AbstractDeclaratorNode* node, int indent);
static void dump_DirectAbstractDeclaratorNode(const DirectAbstractDeclaratorNode* node, int indent);
static void dump_EnumSpecifierNode(const EnumSpecifierNode* node, int indent);
static void dump_EnumeratorListNode(const EnumeratorListNode* node, int indent);
static void dump_EnumeratorNode(const EnumeratorNode* node, int indent);
static void dump_TypedefNameNode(const TypedefNameNode* node, int indent);
static void dump_DeclarationNode(const DeclarationNode* node, int indent);
static void dump_InitDeclaratorNode(const InitDeclaratorNode* node, int indent);
static void dump_InitializerNode(const InitializerNode* node, int indent);
static void dump_InitializerListNode(const InitializerListNode* node, int indent);
static void dump_CompoundStmtNode(const CompoundStmtNode* node, int indent);
static void dump_StmtNode(const StmtNode* node, int indent);
static void dump_LabeledStmtNode(const LabeledStmtNode* node, int indent);
static void dump_ExprStmtNode(const ExprStmtNode* node, int indent);
static void dump_SelectStmtNode(const SelectStmtNode* node, int indent);
static void dump_IterStmtNode(const IterStmtNode* node, int indent);
static void dump_JumpStmtNode(const JumpStmtNode* node, int indent);
static void print_name(const char* s, int indent);

void dump_nodes(const TransUnitNode* node) {
    printf("TransUnitNode\n");

    for (int i = 0; i < node->external_decl_nodes->size; ++i) {
        const ExternalDeclNode* external_decl_node = (const ExternalDeclNode*)(node->external_decl_nodes->elements[i]);
        dump_ExternalDeclNode(external_decl_node, 2);
    }
}

static void dump_ExternalDeclNode(const ExternalDeclNode* node, int indent) {
    print_name("ExternalDeclNode", indent);

    if (node->func_def_node    != NULL) { 
        dump_FuncDefNode(node->func_def_node, indent + 2);
    }

    if (node->declaration_node != NULL) {
        dump_DeclarationNode(node->declaration_node, indent + 2);
    }
}

static void dump_FuncDefNode(const FuncDefNode* node, int indent) {
    print_name("FuncDefNode", indent);

    if (node->compound_stmt_node != NULL) {
        dump_CompoundStmtNode(node->compound_stmt_node, indent + 2);
    }
}

static void dump_DeclSpecifierNode(const DeclSpecifierNode* node, int indent) {
    print_name("DeclSpecifierNode", indent);
}

static void dump_StorageClassSpecifierNode(const StorageClassSpecifierNode* node, int indent) {
    print_name("StorageClassSpecifierNode", indent);
}

static void dump_TypeSpecifierNode(const TypeSpecifierNode* node, int indent) {
    print_name("TypeSpecifierNode", indent);
}

static void dump_StructOrUnionSpecifierNode(const StructOrUnionSpecifierNode* node, int indent) {
    print_name("StructOrUnionSpecifierNode", indent);
}

static void dump_StructOrUnionNode(const StructOrUnionNode* node, int indent) {
    print_name("StructOrUnionNode", indent);
}

static void dump_StructDeclNode(const StructDeclNode* node, int indent) {
    print_name("StructDeclNode", indent);
}

static void dump_DeclaratorNode(const DeclaratorNode* node, int indent) {
    print_name("PointerNode", indent);
}

static void dump_TypeQualifierNode(const TypeQualifierNode* node, int indent) {
    print_name("TypeQualifierNode", indent);
}

static void dump_DirectDeclaratorNode(const DirectDeclaratorNode* node, int indent) {
    print_name("DirectDeclaratorNode", indent);
}

static void dump_ConstantExprNode(const ConstantExprNode* node, int indent) {
    print_name("ConstantExprNode", indent);
}

static void dump_ConditionalExprNode(const ConditionalExprNode* node, int indent) {
    print_name("ConditionalExprNode", indent);
}

static void dump_LogicalOrExprNode(const LogicalOrExprNode* node, int indent) {
    print_name("LogicalOrExprNode", indent);
}

static void dump_LogicalAndExprNode(const LogicalAndExprNode* node, int indent) {
    print_name("LogicalAndExprNode", indent);
}

static void dump_InclusiveOrExprNode(const InclusiveOrExprNode* node, int indent) {
    print_name("InclusiveOrExprNode", indent);
}

static void dump_ExclusiveOrExprNode(const ExclusiveOrExprNode* node, int indent) {
    print_name("ExclusiveOrExprNode", indent);
}

static void dump_AndExprNode(const AndExprNode* node, int indent) {
    print_name("AndExprNode", indent);
}

static void dump_EqualityExprNode(const EqualityExprNode* node, int indent) {
    print_name("EqualityExprNode", indent);
}

static void dump_RelationalExprNode(const RelationalExprNode* node, int indent) {
    print_name("RelationalExprNode", indent);
}

static void dump_ShiftExprNode(const ShiftExprNode* node, int indent) {
    print_name("ShiftExprNode", indent);
}

static void dump_AdditiveExprNode(const AdditiveExprNode* node, int indent) {
    print_name("AdditiveExprNode", indent);
}

static void dump_MultiPlicativeExprNode(const MultiPlicativeExprNode* node, int indent) {
    print_name("MultiPlicativeExprNode", indent);
}

static void dump_CastExprNode(const CastExprNode* node, int indent) {
    print_name("CastExprNode", indent);
}

static void dump_UnaryExprNode(const UnaryExprNode* node, int indent) {
    print_name("UnaryExprNode", indent);
}

static void dump_PostfixExprNode(const PostfixExprNode* node, int indent) {
    print_name("PostfixExprNode", indent);
}

static void dump_PrimaryExprNode(const PrimaryExprNode* node, int indent) {
    print_name("PrimaryExprNode", indent);
}

static void dump_ConstantNode(const ConstantNode* node, int indent) {
    print_name("ConstantNode", indent);
}

static void dump_ExprNode(const ExprNode* node, int indent) {
    print_name("ExprNode", indent);

    if (node->assign_expr_node != NULL) {
        dump_AssignExprNode(node->assign_expr_node, indent + 2);
    } 

    if (node->expr_node != NULL) {
        dump_ExprNode(node->expr_node, indent + 2);
    } 
}

static void dump_AssignExprNode(const AssignExprNode* node, int indent) {
    print_name("AssignExprNode", indent);

    if (node->conditional_expr_node != NULL) {
        dump_ConditionalExprNode(node->conditional_expr_node, indent + 2);
    }

    if (node->unary_expr_node != NULL) {
        dump_UnaryExprNode(node->unary_expr_node, indent + 2);
        print_name(fmt("Assignment-Operator=%d", node->assign_operator), indent + 2);
        dump_AssignExprNode(node->assign_expr_node, indent + 2);
    }
}

static void dump_TypeNameNode(const TypeNameNode* node, int indent) {
    print_name("TypeNameNode", indent);
}

static void dump_ParamTypeListNode(const ParamTypeListNode* node, int indent) {
    print_name("ParamTypeListNode", indent);
}

static void dump_ParamListNode(const ParamListNode* node, int indent) {
    print_name("ParamListNode", indent);
}

static void dump_ParamDeclarationNode(const ParamDeclarationNode* node, int indent) {
    print_name("ParamDeclarationNode", indent);
}

static void dump_AbstractDeclaratorNode(const AbstractDeclaratorNode* node, int indent) {
    print_name("AbstractDeclaratorNode", indent);
}

static void dump_DirectAbstractDeclaratorNode(const DirectAbstractDeclaratorNode* node, int indent) {
    print_name("DirectAbstractDeclaratorNode", indent);
}

static void dump_EnumSpecifierNode(const EnumSpecifierNode* node, int indent) {
    print_name("EnumSpecifierNode", indent);
}

static void dump_EnumeratorListNode(const EnumeratorListNode* node, int indent) {
    print_name("EnumeratorListNode", indent);
}

static void dump_EnumeratorNode(const EnumeratorNode* node, int indent) {
    print_name("EnumeratorNode", indent);
}

static void dump_TypedefNameNode(const TypedefNameNode* node, int indent) {
    print_name("TypedefNameNode", indent);
}

static void dump_DeclarationNode(const DeclarationNode* node, int indent) {
    print_name("DeclarationNode", indent);
}

static void dump_InitDeclaratorNode(const InitDeclaratorNode* node, int indent) {
    print_name("InitDeclaratorNode", indent);
}

static void dump_InitializerNode(const InitializerNode* node, int indent) {
    print_name("InitializerNode", indent);
}

static void dump_InitializerListNode(const InitializerListNode* node, int indent) {
    print_name("InitializerListNode", indent);
}

static void dump_CompoundStmtNode(const CompoundStmtNode* node, int indent) {
    print_name("CompoundStmtNode", indent);
    for (int i = 0; i < node->declaration_nodes->size; ++i) {
        const DeclarationNode* declaration_node = (const DeclarationNode*)(node->declaration_nodes->elements[i]);
        dump_DeclarationNode(declaration_node, indent + 2);
    }

    for (int i = 0; i < node->stmt_nodes->size; ++i) {
        const StmtNode* stmt_node = (const StmtNode*)(node->stmt_nodes->elements[i]);
        dump_StmtNode(stmt_node, indent + 2);
    }
}

static void dump_StmtNode(const StmtNode* node, int indent) {
    print_name("StmtNode", indent);

    if (node->expr_stmt_node != NULL) {
        dump_ExprStmtNode(node->expr_stmt_node, indent + 2);    
    }

    if (node->jump_stmt_node != NULL) {
        dump_JumpStmtNode(node->jump_stmt_node, indent + 2);
    }
}

static void dump_LabeledStmtNode(const LabeledStmtNode* node, int indent) {
    print_name("LabeledStmtNode", indent);
}

static void dump_ExprStmtNode(const ExprStmtNode* node, int indent) {
    print_name("ExprStmtNode", indent);

    if (node->expr_node != NULL) {
        dump_ExprNode(node->expr_node, indent + 2);
    }
}

static void dump_SelectStmtNode(const SelectStmtNode* node, int indent) {
    print_name("SelectStmtNode", indent);
}

static void dump_IterStmtNode(const IterStmtNode* node, int indent) {
    print_name("IterStmtNode", indent);
}

static void dump_JumpStmtNode(const JumpStmtNode* node, int indent) {
    print_name("JumpStmtNode", indent);
}

static void print_name(const char* s, int indent) {
    for (int i = 0; i < indent; ++i) {
        putchar(' ');
    }
    printf("%s\n", s);
}
#endif
