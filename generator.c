#include "generator.h"

#include <stdio.h> 
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "util.h"

static int current_offset;
static PtrVector* localvar_list;
static LocalVar* get_localvar(const char* str, int len);
static void process_expr(const ExprNode* node);

static void print_code(const char* fmt, ...) { va_list ap;
    va_start(ap, fmt);
    printf("  ");
    vprintf(fmt, ap);
    printf("\n");
}

static void print_header() {
    printf(".intel_syntax noprefix\n");
}

static void print_global(const TransUnitNode* node) {
    printf(".global main\n\n");
}

static void process_identifier(const char* identifier, int len) {
    const LocalVar* localvar = get_localvar(identifier, len);
    if (localvar != NULL) {
        print_code("mov rax, rbp");
        print_code("sub rax, %d", localvar->offset);
        print_code("mov rax, [rax]");
        print_code("push rax");
    }
}

static void process_constant_node(const ConstantNode* node) {
    switch (node->const_type) {
    case CONST_INT: {
        print_code("push %d", node->integer_constant);
        break;
    }
    case CONST_STR: {
        break;  
    }
    case CONST_FLOAT: {
        break;
    }
    default: {
        error("Invalid ConstType.\n");
        break;
    }
    }
}

static void process_primary_expr(const PrimaryExprNode* node) {
    // <constant>
    if (node->constant_node != NULL) {
        process_constant_node(node->constant_node);
    }
    // ( <expression> )
    else if (node->expr_node != NULL) {
        process_expr(node->expr_node);
    }
    // <identifier>
    else if (node->identifier != NULL) {
        process_identifier(node->identifier, node->identifier_len);  
    }
    else {
        // @todo
    }
}

static void process_postfix_expr(const PostfixExprNode* node) {
    // <primary-expression>
    if (node->primary_expr_node != NULL) {
        process_primary_expr(node->primary_expr_node);
    }
    else {
        // @todo
    }
}

static void process_unary_expr(const UnaryExprNode* node) {
    // <postfix-expression>
    if (node->postfix_expr_node != NULL) {
        process_postfix_expr(node->postfix_expr_node);
    }
    else {
        // @todo
    } 
}

static void process_cast_expr(const CastExprNode* node) {
    // <unary-expression>
    if (node->unary_expr_node != NULL) {
        process_unary_expr(node->unary_expr_node);    
    }
    // ( <type-name> ) <cast-expression>
    else {
        // @todo
    }
}

static void process_multiplicative_expr(const MultiPlicativeExprNode* node) {
    // <cast-expression>
    if (node->multiplicative_expr_node == NULL) {
        process_cast_expr(node->cast_expr_node);
    }
    //   <multiplicative-expression> * <cast-expression>
    // | <multiplicative-expression> / <cast-expression>
    // | <multiplicative-expression> % <cast-expression>
    else if (node->operator_type == OP_MUL) {
        process_multiplicative_expr(node->multiplicative_expr_node);
        process_cast_expr(node->cast_expr_node); 

        print_code("pop rdi");
        print_code("pop rax");
        print_code("imul rax, rdi");
        print_code("push rax");
    }
    else if (node->operator_type == OP_DIV) {
        process_multiplicative_expr(node->multiplicative_expr_node);
        process_cast_expr(node->cast_expr_node); 

        print_code("pop rdi");
        print_code("pop rax");
        print_code("cqo");
        print_code("idiv rdi");
        print_code("push rax");
    }
    else if (node->operator_type == OP_MOD) {
        process_multiplicative_expr(node->multiplicative_expr_node);
        process_cast_expr(node->cast_expr_node); 

        print_code("pop rdi");
        print_code("pop rax");
        print_code("cqo");
        print_code("idiv rdi");
        print_code("push rdx");
    }
}

static void process_additive_expr(const AdditiveExprNode* node) {
    // <multiplicative-expression>

    if (node->additive_expr_node == NULL) {
        process_multiplicative_expr(node->multiplicative_expr_node); 
    }
    //   <additive-expression> + <multiplicative-expression>
    else if (node->operator_type == OP_ADD) {
        process_additive_expr(node->additive_expr_node); 
        process_multiplicative_expr(node->multiplicative_expr_node); 

        print_code("pop rdi");
        print_code("pop rax");
        print_code("add rax, rdi");
        print_code("push rax");
    }
    // <additive-expression> - <multiplicative-expression>
    else if (node->operator_type == OP_SUB) {
        process_additive_expr(node->additive_expr_node); 
        process_multiplicative_expr(node->multiplicative_expr_node); 

        print_code("pop rdi");
        print_code("pop rax");
        print_code("sub rax, rdi");
        print_code("push rax");
    }
}

static void process_shift_expr(const ShiftExprNode* node) {
    // <additive-expression>
    if (node->shift_expr_node == NULL) {
        process_additive_expr(node->additive_expr_node); 
    }
    //   <shift-expression> << <additive-expression>
    // | <shift-expression> >> <additive-expression>
    else {
        // @todo
    }
}

static void process_relational_expr(const RelationalExprNode* node) {
    // <shift-expression>
    if (node->relational_expr_node == NULL) {
        process_shift_expr(node->shift_expr_node);
    }
    //   <relational-expression> <  <shift-expression>
    // | <relational-expression> >  <shift-expression> 
    // | <relational-expression> <= <shift-expression> 
    // | <relational-expression> >= <shift-expression> 
    else {
        // @todo
    }
}

static void process_equality_expr(const EqualityExprNode* node) {
    // <relational-expression>
    if (node->equality_expr_node == NULL) {
        process_relational_expr(node->relational_expr_node);
    }
    //   <equality-expression> == <relational-expression> 
    // | <equality-expression> != <relational-expression>  
    else {
        // @todo
    }
}

static void process_and_expr(const AndExprNode* node) {
    // <equality-expression>
    if (node->and_expr_node == NULL) {
        process_equality_expr(node->equality_expr_node);
    }
    // 
    else {
        // @todo
    }
}    

static void process_exclusive_or_expr(const ExclusiveOrExprNode* node) {
    // <and-expression>
    if (node->exclusive_or_expr_node == NULL) {
        process_and_expr(node->and_expr_node); 
    }
    // <exclusive-or-expression> ^ <and-expression>
    else {
        // @todo
    }
}

static void process_inclusive_or_expr(const InclusiveOrExprNode* node) {
    // <exclusive-or-expression>
    if (node->inclusive_or_expr_node == NULL) {
        process_exclusive_or_expr(node->exclusive_or_expr_node);
    } 
    // <inclusive-or-expression> | <exclusive-or-expression>
    else {
        // @todo
    }
}   

static void process_logical_and_expr(const LogicalAndExprNode* node) {
    // <inclusive-or-expression>
    if (node->logical_and_expr_node == NULL) {
        process_inclusive_or_expr(node->inclusive_or_expr_node);
    }
    // <logical-and-expression> && <inclusive-or-expression>
    else {
        // @todo
    }
}

static void process_logical_or_expr(const LogicalOrExprNode* node) {
    // <logical-and-expression>
    if (node->logical_or_expr_node == NULL) {
        process_logical_and_expr(node->logical_and_expr_node);
    }
    // <logical-or-expression> || <logical-and-expression>
    else {
        // @todo
    }
}

static void process_conditional_expr(const ConditionalExprNode* node) {
    // <logical-or-expression>
    if (node->conditional_expr_node == NULL) {
        process_logical_or_expr(node->logical_or_expr_node);
    }
    // <logical-or-expression> ? <expression> : <conditional-expression>
    else {
        // @todo
    }
}

static void process_assign_expr(const AssignExprNode* node) {
    // <conditional-expression>
    if (node->conditional_expr_node != NULL) {
        process_conditional_expr(node->conditional_expr_node);    
    }
    // <unary-expression> <assignment-operator> <assignment-expression>
    else {
        // @todo
    }
}

static void process_expr(const ExprNode* node) {
    // <assignment-expression>
    if (node->expr_node == NULL) {
        process_assign_expr(node->assign_expr_node);
    }
    // <expression> , <assignment-expression>
    else {
        // @todo
    }
}

static void process_jump_stmt(const JumpStmtNode* node) {
    switch (node->jump_type) {
    case JMP_GOTO:
    case JMP_CONTINUE:
    case JMP_BREAK: {
        // @todo
        break;
    }
    case JMP_RETURN: {
        if (node->expr_node != NULL) {
            process_expr(node->expr_node);
        }
        print_code("pop rax");
        // print_code("ret");

        break;
    }
    default: {
        break;
    }
    }
}

static void process_stmt(const StmtNode* node) {
    if (node->jump_stmt_node != NULL) {
        process_jump_stmt(node->jump_stmt_node);
    }
}

static void process_initializer(const InitializerNode* node) {
    if (node->assign_expr_node != NULL) {
        process_assign_expr(node->assign_expr_node);
    }

    print_code("pop rdi");
    print_code("pop rax");
    print_code("mov [rax], rdi");
}

static LocalVar* get_localvar(const char* str, int len) {
    for (int i = 0; i < localvar_list->size; ++i) {
        LocalVar* localvar = (LocalVar*)(localvar_list->elements[i]);
        if (strncmp(localvar->name, str, len) == 0) {
            return localvar;
        }
    }

    return NULL;
}

static void process_direct_declarator(const DirectDeclaratorNode* node) {
    LocalVar* localvar = get_localvar(node->identifier, node->identifier_len);
    if (localvar == NULL) {
        localvar           = malloc(sizeof(LocalVar));
        localvar->offset   = current_offset;
        localvar->name_len = node->identifier_len;
        localvar->name     = malloc(sizeof(char) * localvar->name_len);
        strncpy(localvar->name, node->identifier, localvar->name_len); 
        ptr_vector_push_back(localvar_list, localvar);

        current_offset += 8;
    }

    print_code("mov rax, rbp"); 
    print_code("sub rax, %d", localvar->offset); 
    print_code("push rax");
}

static void process_declarator(const DeclaratorNode* node) {
    if (node->direct_declarator_node != NULL) { 
        process_direct_declarator(node->direct_declarator_node);
    }
}

static void process_init_declarator_nodes(const InitDeclaratorNode* node) {
    if (node->declarator_node != NULL) {
        process_declarator(node->declarator_node);
    }

    if (node->initializer_node != NULL) {
        process_initializer(node->initializer_node);
    }
}

static void process_declaration(const DeclarationNode* node) {
    // for (int i = 0; i < node->declaration-specifier_node->size; ++i) {
    // }

    for (int i = 0; i < node->init_declarator_nodes->size; ++i) {
        process_init_declarator_nodes(node->init_declarator_nodes->elements[i]);
    }
}

static void process_compound_stmt(const CompoundStmtNode* node) {
    for (int i = 0; i < node->declaration_nodes->size; ++i) {
        process_declaration((const DeclarationNode*)(node->declaration_nodes->elements[i]));
    }

    for (int i = 0; i < node->stmt_nodes->size; ++i) {
        process_stmt((const StmtNode*)(node->stmt_nodes->elements[i]));
    }
}

static int count_localvars_in_func_def(const FuncDefNode* node) {
    const CompoundStmtNode* compound_stmt_node = node->compound_stmt_node;
    if (compound_stmt_node == NULL) {
        return 0;
    }

    int cnt = 0;
    for (int i = 0; i < compound_stmt_node->declaration_nodes->size; ++i) {
        const DeclarationNode* declaration_node = (DeclarationNode*)(compound_stmt_node->declaration_nodes->elements[i]);
        cnt += declaration_node->init_declarator_nodes->size;
    }
    
    return cnt; 
}

static void process_func_def(const FuncDefNode* node) {
    localvar_list = create_ptr_vector();
    current_offset = 8;

    printf("%s:\n", node->identifier);

    const int localvar_count = count_localvars_in_func_def(node);
    if (localvar_count != 0) {
        print_code("push rbp"); 
        print_code("mov rbp, rsp"); 
        print_code("sub rsp, %d", localvar_count * 8); 
    }

    if (node->compound_stmt_node != NULL) {
        process_compound_stmt(node->compound_stmt_node);
    } 

    if (localvar_count != 0) {
        print_code("mov rsp, rbp"); 
        print_code("pop rbp"); 
    }

    print_code("ret");

    free(localvar_list);
    current_offset = 8;
}

static void process_external_decl(const ExternalDeclNode* node) {
    if (node->func_def_node != NULL) {
        process_func_def(node->func_def_node);
    }
}

void gen(TransUnitNode* node) {
    print_header();
    print_global(node);
    
    for (int i = 0; i < node->external_decl_nodes->size; ++i) {
        const ExternalDeclNode* external_decl_node = (ExternalDeclNode*)(node->external_decl_nodes->elements[i]);
        process_external_decl(external_decl_node);
    } 
}
