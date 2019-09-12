#include "generator.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

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

static void process_expr(ExprNode* node) {
    print_code("push %d", node->integer_constant);
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
    if (node->jump_stmt != NULL) {
        process_jump_stmt(node->jump_stmt);
    }
}

static void process_compound_stmt(CompoundStmtNode* node) {
    for (int i = 0; i < node->stmt->size; ++i) {
        process_stmt((StmtNode*)(node->stmt->nodes[i]));
    }
}

static void process_func_def(FuncDefNode* node) {
    printf("%s:\n", node->identifier);
   
    if (node->compound_stmt != NULL) {
        process_compound_stmt(node->compound_stmt);
    } 
}

void gen(TransUnitNode* node) {
    print_header();
    print_global(node);
    
    for (int i = 0; i < node->func_def->size; ++i) {
        FuncDefNode* func_def_node = (FuncDefNode*)(node->func_def->nodes[i]);
        process_func_def(func_def_node);
    } 
}
