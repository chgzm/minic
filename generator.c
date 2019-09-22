#include "generator.h"

#include <stdio.h> 
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "util.h"

static void process_expr(ExprNode* node);

static void print_code(char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    printf("  ");
    vprintf(fmt, ap);
    printf("\n");
}

static void print_header() {
    printf(".intel_syntax noprefix\n");
}

static void print_global(TransUnitNode* node) {
    printf(".global main\n\n");
}

static void process_constant_node(ConstantNode* node) {
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

static void process_primary_expr(PrimaryExprNode* node) {
    // <constant>
    if (node->constant_node != NULL) {
        process_constant_node(node->constant_node);
    }
    // ( <expression> )
    else if (node->expr_node != NULL) {
        process_expr(node->expr_node);
    }
    else {
        // @todo
    }
}

static void process_postfix_expr(PostfixExprNode* node) {
    // <primary-expression>
    if (node->primary_expr_node != NULL) {
        process_primary_expr(node->primary_expr_node);
    }
    else {
        // @todo
    }
}

static void process_unary_expr(UnaryExprNode* node) {
    // <postfix-expression>
    if (node->postfix_expr_node != NULL) {
        process_postfix_expr(node->postfix_expr_node);
    }
    else {
        // @todo
    } 
}

static void process_cast_expr(CastExprNode* node) {
    // <unary-expression>
    if (node->unary_expr_node != NULL) {
        process_unary_expr(node->unary_expr_node);    
    }
    // ( <type-name> ) <cast-expression>
    else {
        // @todo
    }
}

static void process_multiplicative_expr(MultiPlicativeExprNode* node) {
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

static void process_additive_expr(AdditiveExprNode* node) {
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

static void process_shift_expr(ShiftExprNode* node) {
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

static void process_relational_expr(RelationalExprNode* node) {
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

static void process_equality_expr(EqualityExprNode* node) {
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

static void process_and_expr(AndExprNode* node) {
    // <equality-expression>
    if (node->and_expr_node == NULL) {
        process_equality_expr(node->equality_expr_node);
    }
    // 
    else {
        // @todo
    }
}    

static void process_exclusive_or_expr(ExclusiveOrExprNode* node) {
    // <and-expression>
    if (node->exclusive_or_expr_node == NULL) {
        process_and_expr(node->and_expr_node); 
    }
    // <exclusive-or-expression> ^ <and-expression>
    else {
        // @todo
    }
}

static void process_inclusive_or_expr(InclusiveOrExprNode* node) {
    // <exclusive-or-expression>
    if (node->inclusive_or_expr_node == NULL) {
        process_exclusive_or_expr(node->exclusive_or_expr_node);
    } 
    // <inclusive-or-expression> | <exclusive-or-expression>
    else {
        // @todo
    }
}   

static void process_logical_and_expr(LogicalAndExprNode* node) {
    // <inclusive-or-expression>
    if (node->logical_and_expr_node == NULL) {
        process_inclusive_or_expr(node->inclusive_or_expr_node);
    }
    // <logical-and-expression> && <inclusive-or-expression>
    else {
        // @todo
    }
}

static void process_logical_or_expr(LogicalOrExprNode* node) {
    // <logical-and-expression>
    if (node->logical_or_expr_node == NULL) {
        process_logical_and_expr(node->logical_and_expr_node);
    }
    // <logical-or-expression> || <logical-and-expression>
    else {
        // @todo
    }
}

static void process_conditional_expr(ConditionalExprNode* node) {
    // <logical-or-expression>
    if (node->conditional_expr_node == NULL) {
        process_logical_or_expr(node->logical_or_expr_node);
    }
    // <logical-or-expression> ? <expression> : <conditional-expression>
    else {
        // @todo
    }
}

static void process_assign_expr(AssignExprNode* node) {
    // <conditional-expression>
    if (node->conditional_expr_node != NULL) {
        process_conditional_expr(node->conditional_expr_node);    
    }
    // <unary-expression> <assignment-operator> <assignment-expression>
    else {
        // @todo
    }
}

static void process_expr(ExprNode* node) {
    // <assignment-expression>
    if (node->expr_node == NULL) {
        process_assign_expr(node->assign_expr_node);
    }
    // <expression> , <assignment-expression>
    else {
        // @todo
    }
}

static void process_return(ReturnNode* node) {
    if (node->expr != NULL) {
        process_expr(node->expr);
    }

    print_code("pop rax");
    print_code("ret");
}

static void process_jump_stmt(JumpStmtNode* node) {
    if (node->ret != NULL) {
        process_return(node->ret);
    }
}

static void process_stmt(StmtNode* node) {
    if (node->jump_stmt_node != NULL) {
        process_jump_stmt(node->jump_stmt_node);
    }
}

static void process_compound_stmt(CompoundStmtNode* node) {
    for (int i = 0; i < node->stmt->size; ++i) {
        process_stmt((StmtNode*)(node->stmt->nodes[i]));
    }
}

static void process_func_def(FuncDefNode* node) {
    printf("%s:\n", node->identifier);
   
    if (node->compound_stmt_node != NULL) {
        process_compound_stmt(node->compound_stmt_node);
    } 
}

static void process_external_decl(ExternalDeclNode* node) {
    if (node->func_def_node != NULL) {
        process_func_def(node->func_def_node);
    }
}

void gen(TransUnitNode* node) {
    print_header();
    print_global(node);
    
    for (int i = 0; i < node->external_decl_nodes->size; ++i) {
        ExternalDeclNode* external_decl_node = (ExternalDeclNode*)(node->external_decl_nodes->nodes[i]);
        process_external_decl(external_decl_node);
    } 
}
