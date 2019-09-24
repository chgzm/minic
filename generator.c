#include "generator.h"

#include <stdio.h> 
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "util.h"

static char* arg_registers[] = {
    "rdi", "rsi", "rdx", "rcx", "r8", "r9"
};

static int current_offset;
static PtrVector* localvar_list;
static LocalVar* get_localvar(const char* str, int len);
static void process_expr(const ExprNode* node);
static void process_conditional_expr(const ConditionalExprNode* node);

static void print_code(const char* fmt, ...) { 
    va_list ap;
    va_start(ap, fmt);
    printf("  ");
    vprintf(fmt, ap);
    printf("\n");
}

static void print_header() {
    printf(".intel_syntax noprefix\n");
}

static void print_global(const TransUnitNode* node) {
    printf(".global");
    for (int i = 0; i < node->external_decl_nodes->size; ++i) {
        const ExternalDeclNode*     external_decl_node     = (ExternalDeclNode*)(node->external_decl_nodes->elements[i]);
        const FuncDefNode*          func_def_node          = external_decl_node->func_def_node;
        const DeclaratorNode*       declarator_node        = func_def_node->declarator_node;
        const DirectDeclaratorNode* direct_declarator_node = declarator_node->direct_declarator_node;

        if (i != 0) {
            printf(",");
        }
        printf(" %s", direct_declarator_node->direct_declarator_node->identifier); // @todo
    } 
    printf("\n\n");
}

static void process_identifier_left(const char* identifier, int len) {
    const LocalVar* localvar = get_localvar(identifier, len);
    if (localvar != NULL) {
        print_code("mov rax, rbp");
        print_code("sub rax, %d", localvar->offset);
        print_code("push rax");
    }
}

static void process_identifier_right(const char* identifier, int len) {
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

static void process_primary_expr_left(const PrimaryExprNode* node) {
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
        process_identifier_left(node->identifier, node->identifier_len);  
    }
    else {
        // @todo
    }
}

static void process_primary_expr_right(const PrimaryExprNode* node) {
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
        process_identifier_right(node->identifier, node->identifier_len);  
    }
    else {
        // @todo
    }
}

static void process_postfix_expr_left(const PostfixExprNode* node) {
    switch (node->postfix_expr_type) {
    // <primary-expression>
    case PS_PRIMARY: {
        process_primary_expr_left(node->primary_expr_node);
        break;
    }
    default: {
        // @todo
        break;
    }
    }
}

static void process_postfix_expr_right(const PostfixExprNode* node) {
    switch (node->postfix_expr_type) {
    // <primary-expression>
    case PS_PRIMARY: {
        process_primary_expr_right(node->primary_expr_node);
        break;
    }
    // <postfix-expression> ( {assignment-expression}* )
    case PS_LPAREN: {
        for (int i = 0; i < node->assign_expr_nodes->size; ++i) {
            const AssignExprNode* assign_expr_node = (const AssignExprNode*)(node->assign_expr_nodes->elements[i]);
            process_conditional_expr(assign_expr_node->conditional_expr_node);

            print_code("pop rax");
            print_code("mov %s, rax", arg_registers[i]);
        }

        const char* identifier = node->postfix_expr_node->primary_expr_node->identifier; 
        print_code("call %s", identifier);
        print_code("push rax");

        break;
    }
    default: {
        // @todo
        break;
    }
    }
}

static void process_unary_expr_left(const UnaryExprNode* node) {
    // <postfix-expression>
    if (node->postfix_expr_node != NULL) {
        process_postfix_expr_left(node->postfix_expr_node);
    }
    else {
        // @todo
    } 
}

static void process_unary_expr_right(const UnaryExprNode* node) {
    // <postfix-expression>
    if (node->postfix_expr_node != NULL) {
        process_postfix_expr_right(node->postfix_expr_node);
    }
    else {
        // @todo
    } 
}

static void process_cast_expr(const CastExprNode* node) {
    // <unary-expression>
    if (node->unary_expr_node != NULL) {
        process_unary_expr_right(node->unary_expr_node);    
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
        process_unary_expr_left(node->unary_expr_node);
        process_assign_expr(node->assign_expr_node);

        switch (node->assign_operator) {
        case OP_ASSIGN: {
            print_code("pop rdi");
            print_code("pop rax");
            print_code("mov [rax], rdi");
            print_code("push rax");

            break;
        } 
        case OP_MUL_EQ: 
        case OP_DIV_EQ: 
        case OP_MOD_EQ: 
        case OP_ADD_EQ: 
        case OP_SUB_EQ: 
        case OP_AND_EQ: 
        case OP_XOR_EQ: 
        case OP_OR_EQ: {
            // @todo
            break;
        }
        default: {
            break;
        }
        }
    }
}

static void process_expr(const ExprNode* node) {
    // <assignment-expression>
    if (node->expr_node == NULL) {
        process_assign_expr(node->assign_expr_node);
    }
    // <expression> , <assignment-expression>
    else {
        const ExprNode* current = node;
        while (current->expr_node != NULL) {
            process_assign_expr(current->assign_expr_node);
            current = current->expr_node;
        }
    }
}

static void process_expr_stmt(const ExprStmtNode* node) {
    if (node->expr_node != NULL) {
        process_expr(node->expr_node);
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
    if (node->expr_stmt_node != NULL) {
        process_expr_stmt(node->expr_stmt_node);
    }
    else if (node->jump_stmt_node != NULL) {
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

static int count_args_in_func_def(const FuncDefNode* node) {
    const DeclaratorNode* declarator_node = node->declarator_node;
    if (declarator_node == NULL) {
        return 0;
    }
    
    const DirectDeclaratorNode* direct_declarator_node = declarator_node->direct_declarator_node;
    const ParamTypeListNode*    param_type_list_node   = direct_declarator_node->param_type_list_node;
    if (param_type_list_node == NULL) {
        return 0;
    }

    int cnt = 0;
    const ParamListNode* current = param_type_list_node->param_list_node;
    while (current != NULL) {
        ++cnt;
        current = current->param_list_node;
    }

    return cnt;
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

static void process_args(const ParamListNode* node, int arg_index) {
    if (node->param_list_node != NULL) {
        process_args(node->param_list_node, arg_index + 1);
    }

    const ParamDeclarationNode* param_declaration_node = node->param_declaration_node;
    const DeclaratorNode* declarator_node = param_declaration_node->declarator_node;
    if (declarator_node == NULL) {
        return;
    }
   
    const DirectDeclaratorNode* direct_declarator_node = declarator_node->direct_declarator_node; 
    LocalVar* localvar = malloc(sizeof(LocalVar)); 
    localvar->offset   = current_offset;
    localvar->name_len = direct_declarator_node->identifier_len;
    localvar->name     = malloc(sizeof(char) * localvar->name_len);
    strncpy(localvar->name, direct_declarator_node->identifier, localvar->name_len); 
    ptr_vector_push_back(localvar_list, localvar);

    current_offset += 8;

    print_code("mov rax, rbp");
    print_code("sub rax, %d", localvar->offset);
    print_code("mov [rax], %s", arg_registers[arg_index]);
}

static void process_func_declarator(const DeclaratorNode* node) {
    const DirectDeclaratorNode* direct_declarator_node = node->direct_declarator_node;
    const ParamTypeListNode* param_type_list_node = direct_declarator_node->param_type_list_node;
    if (param_type_list_node == NULL) {
        return;
    }

    const ParamListNode* param_list_node = param_type_list_node->param_list_node;
    process_args(param_list_node, 0);
}

static void process_func_def(const FuncDefNode* node) {
    localvar_list = create_ptr_vector();
    current_offset = 8;

    const DeclaratorNode* declarator_node = node->declarator_node;
    const DirectDeclaratorNode* direct_declarator_node = declarator_node->direct_declarator_node;
    printf("%s:\n", direct_declarator_node->direct_declarator_node->identifier); // @todo

    const int localvar_count = count_localvars_in_func_def(node);
    const int arg_count      = count_args_in_func_def(node);

    print_code("push rbp"); 
    print_code("mov rbp, rsp"); 
    print_code("sub rsp, %d", (localvar_count + arg_count) * 8); 

    process_func_declarator(declarator_node);

    process_compound_stmt(node->compound_stmt_node);

    print_code("mov rsp, rbp"); 
    print_code("pop rbp"); 

    print_code("ret");

    free(localvar_list);
    current_offset = 8;
}

static void process_external_decl(const ExternalDeclNode* node) {
    if (node->func_def_node != NULL) {
        process_func_def(node->func_def_node);
    }
}

void gen(const TransUnitNode* node) {
    print_header();
    print_global(node);
    
    for (int i = 0; i < node->external_decl_nodes->size; ++i) {
        const ExternalDeclNode* external_decl_node = (ExternalDeclNode*)(node->external_decl_nodes->elements[i]);
        process_external_decl(external_decl_node);
    } 
}
