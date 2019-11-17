#include "generator.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "util.h"

//
// global
//

static char* arg_registers[6] = {
    "rdi", "rsi", "rdx", "rcx", "r8", "r9"
}; 
static int label_index;
static int string_index;
static int current_offset;
static char* ret_label;
static Vector* localvar_list;
static Vector* globalvar_list;
static StrPtrMap* struct_map;  
static StrIntMap* enum_map;
static Stack* break_label_stack;
static Stack* continue_label_stack;
static Stack* current_stmt_label_stack;
static Stack* type_stack;
static IntStack* size_stack;

//
// forward declaration
//

static int calc_localvar_size_in_compound_stmt(const CompoundStmtNode* node);
static int calc_localvar_size_in_stmt(const StmtNode* node);
static void process_expr(const ExprNode* node);
static void process_expr_left(const ExprNode* node);
static void process_stmt(const StmtNode* node);
static void process_conditional_expr(const ConditionalExprNode* node);
static void process_compound_stmt(const CompoundStmtNode* node);
static void process_cast_expr(const CastExprNode* node);
static void process_declaration(const DeclarationNode* node);
static Type* process_type_specifier_in_local(const TypeSpecifierNode* node);
static int get_array_size_from_constant_expr(const ConditionalExprNode* node);

static const char* get_ident_from_direct_declarator(const DirectDeclaratorNode* node) {
    const DirectDeclaratorNode* current = node;
    while (current->direct_declarator_node != NULL) {
        current = current->direct_declarator_node;
    }

    return current->identifier;
}

static char* get_label() {
    char buf[8];
    snprintf(buf, 8, ".L%d", label_index++);
    return strdup(buf);
}

static char* get_string_label() {
    char buf[8];
    snprintf(buf, 8, ".LC%d", string_index++);
    return strdup(buf);
}

static int align_offset(int offset, int size) {
    if (size == 1) {
        return offset;
    }

    if (offset % 8 != 0) {
        offset += (8 - offset % 8);
    }

    return offset;
}

static const char* get_reg(const char* base, int size) {
    switch (size) {
    case 1: {
        if      (strcmp("ax", base) == 0) { return "al";  }
        else if (strcmp("di", base) == 0) { return "dil"; }
        else if (strcmp("si", base) == 0) { return "sil"; }
        else if (strcmp("dx", base) == 0) { return "dl";  }
        else if (strcmp("cx", base) == 0) { return "cl";  }
        else if (strcmp("r8", base) == 0) { return "r8b"; }
        else if (strcmp("r9", base) == 0) { return "r9b"; }
        else                              { return NULL;  }
    }
    case 8: {
        if      (strcmp("ax", base) == 0) { return "rax"; }
        else if (strcmp("di", base) == 0) { return "rdi"; }
        else if (strcmp("si", base) == 0) { return "rsi"; }
        else if (strcmp("dx", base) == 0) { return "rdx"; }
        else if (strcmp("cx", base) == 0) { return "rcx"; }
        else if (strcmp("r8", base) == 0) { return "r8";  }
        else if (strcmp("r9", base) == 0) { return "r9";  }
        else                              { return NULL;  }
    }
    default: {
        error("Invalid size=%d\n", size);
        return NULL;
    }    
    }
}

static LocalVar* get_localvar(const char* str) {
    for (int i = 0; i < localvar_list->size; ++i) {
        LocalVar* localvar = localvar_list->elements[i];
        if (strcmp(localvar->name, str) == 0) {
            return localvar;
        }
    }

    return NULL;
}

static GlobalVar* get_globalvar(const char* str) {
    for (int i = 0; i < globalvar_list->size; ++i) {
        GlobalVar* gv = globalvar_list->elements[i];
        if (strcmp(gv->name, str) == 0) {
            return gv;
        }
    }

    return NULL;
}

static void process_identifier_left(const char* identifier) {
    const LocalVar* lv = get_localvar(identifier);
    if (lv != NULL) {
        printf("  lea rax, [rbp-%d]\n", lv->offset);
        printf("  push rax\n");
        stack_push(type_stack, lv->type);
    } 
    else {
        const GlobalVar* gv = get_globalvar(identifier);    
        printf("  lea rax, %s[rip]\n", gv->name);
        printf("  push rax\n");
        stack_push(type_stack, gv->type);
    }
    intstack_push(size_stack, 8);
}

static void process_identifier_right(const char* identifier) {
    // enum 
    if (strintmap_contains(enum_map, identifier)) {
        printf("  push %d\n", strintmap_get(enum_map, identifier));
        return;
    } 

    // local variable
    const LocalVar* lv = get_localvar(identifier);
    if (lv != NULL) {
        printf("  lea rax, [rbp-%d]\n", lv->offset);
        if (lv->type->array_size == 0) { 
            if (lv->type->type_size == 1 && lv->type->ptr_count == 0) {
                printf("  movzx eax, BYTE PTR [rax]\n");
                // printf("  movzx eax, BYTE PTR [rbp-%d]\n", lv->offset);
            } else { 
                printf("  mov rax, [rax]\n");
                // printf("  mov rax, [rbp-%d]\n", lv->offset);
            }
        } 
        printf("  push rax\n");
        intstack_push(size_stack, lv->type->size);
        stack_push(type_stack, lv->type);

        return;
    } 

    // global variable
    const GlobalVar* gv = get_globalvar(identifier);
    printf("  lea rax, %s[rip]\n", gv->name);
    if (gv->type->array_size == 0) {
        if (gv->type->type_size == 1 && gv->type->ptr_count == 0) {
            printf("  movzx eax, BYTE PTR [rax]\n");
        } else { 
            printf("  mov rax, [rax]\n");
        }
    }
    printf("  push rax\n");
    intstack_push(size_stack, gv->type->size);
    stack_push(type_stack, gv->type);
}

static void process_constant_node(const ConstantNode* node) {
    switch (node->const_type) {
    case CONST_BYTE: {
        printf("  push %d\n", node->integer_constant);
        intstack_push(size_stack, 1);
        break;
    }
    case CONST_INT: {
        printf("  push %d\n", node->integer_constant);
        intstack_push(size_stack, 8);
        break;
    }
    case CONST_STR: {
        const char* label = get_string_label();
        printf(".data\n");
        printf("%s:\n", label);
        printf("  .string \"%s\"\n", node->character_constant);
        printf(".text\n");
        printf("  lea rax, %s[rip]\n", label);
        printf("  push rax\n");
        // intstack_push(size_stack, 8);
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
    // constant
    if (node->constant_node != NULL) {
        process_constant_node(node->constant_node);
    }
    // ( expression )
    else if (node->expr_node != NULL) {
        process_expr_left(node->expr_node);
    }
    // identifier
    else if (node->identifier != NULL) {
        process_identifier_left(node->identifier);
    }
    else {
        // @todo
    }
}

static void process_primary_expr_right(const PrimaryExprNode* node) {
    // constant
    if (node->constant_node != NULL) {
        process_constant_node(node->constant_node);
    }
    // ( expression )
    else if (node->expr_node != NULL) {
        process_expr(node->expr_node);
    }
    // identifier
    else if (node->identifier != NULL) {
        process_identifier_right(node->identifier);
    }
    else {
        // @todo
    }
}

static void process_postfix_expr_left(const PostfixExprNode* node) {
    switch (node->postfix_expr_type) {
    // primary-expression
    case PS_PRIMARY: {
        process_primary_expr_left(node->primary_expr_node);
        break;
    }
    // postfix-expression [ expression ]
    case PS_LSQUARE: {
        process_postfix_expr_left(node->postfix_expr_node);

        Type* type = stack_top(type_stack);
        stack_pop(type_stack);

        process_expr(node->expr_node);

        printf("  pop rdi\n");
        printf("  pop rax\n");
        if (type->array_size) {
            if (type->ptr_count > 0) {
                printf("  imul rdi, 8\n");
                printf("  mov rax, [rax]\n");
            } else {
                printf("  imul rdi, %d\n", type->type_size);
            }
        } else {
            if (type->ptr_count > 1) {
                printf("  imul rdi, 8\n");
                printf("  mov rax, [rax]\n");
            } else {
                printf("  imul rdi, %d\n", type->type_size);
                printf("  mov rax, [rax]\n");
            }
        }

        printf("  add rax, rdi\n");
        printf("  push rax\n");

        stack_push(type_stack, type);
        // intstack_push(size_stack, 8);
        break;
    }
    // postfix-expression . identifier
    case PS_DOT: {
        process_postfix_expr_left(node->postfix_expr_node);

        Type* type = stack_top(type_stack);
        stack_pop(type_stack);

        const StructInfo* struct_info = type->struct_info;
        const FieldInfo* field_info = strptrmap_get(struct_info->field_info_map, node->identifier);

        printf("  pop rax\n");
        printf("  add rax, %d\n", field_info->offset);

        printf("  push rax\n");
        // intstack_push(size_stack, 8);

        break;        
    }
    // postfix-expression -> identifier
    case PS_ARROW: {
        process_postfix_expr_left(node->postfix_expr_node);

        Type* type = stack_top(type_stack);
        stack_pop(type_stack);

        const StructInfo* struct_info = type->struct_info;
        const FieldInfo* field_info = strptrmap_get(struct_info->field_info_map, node->identifier);
        stack_push(type_stack, field_info->type);

        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  add rax, %d\n", field_info->offset);
        printf("  push rax\n");

        break;        
    }
    // postfix-expression ++
    case PS_INC: {
        process_postfix_expr_left(node->postfix_expr_node);
        printf("  pop rax\n");
        printf("  mov rdi, [rax]\n");
        printf("  push rdi\n");
        printf("  push 1\n");
        printf("  pop rdi\n");
        printf("  add [rax], rdi\n");
        break;                       
    }
    // postfix-expression --
    case PS_DEC: {
        process_postfix_expr_left(node->postfix_expr_node);
        printf("  pop rax\n");
        printf("  mov rdi, [rax]\n");
        printf("  push rdi\n");
        printf("  push 1\n");
        printf("  pop rdi\n");
        printf("  sub [rax], rdi\n");
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
    // primary-expression
    case PS_PRIMARY: {
        process_primary_expr_right(node->primary_expr_node);
        break;
    }
    // postfix-expression ( {assignment-expression}* )
    case PS_LPAREN: {
        for (int i = 0; i < node->assign_expr_nodes->size; ++i) {
            const AssignExprNode* assign_expr_node = node->assign_expr_nodes->elements[i];
            process_conditional_expr(assign_expr_node->conditional_expr_node);
        }

        for (int i = node->assign_expr_nodes->size - 1; i >= 0; --i) {
            printf("  pop rax\n");
            printf("  mov %s, rax\n", arg_registers[i]);
        }

        const char* identifier = node->postfix_expr_node->primary_expr_node->identifier;
        printf("  mov rax, 0\n");
        printf("  call %s\n", identifier);
        printf("  push rax\n");

        break;
    }
    // postfix-expression [ expression ]
    case PS_LSQUARE: {
        process_postfix_expr_right(node->postfix_expr_node);

        Type* type = stack_top(type_stack);
        stack_pop(type_stack);

        process_expr(node->expr_node);

        printf("  pop rdi\n");
        printf("  pop rax\n");

        if (type->array_size > 0) {
            if (type->ptr_count > 0) {
                printf("  imul rdi, 8\n");
            } else {
                printf("  imul rdi, %d\n", type->type_size);
            }

            printf("  add rax, rdi\n");
            printf("  mov rax, [rax]\n");
        }
        else {
            if (type->ptr_count > 1) {
                printf("  imul rdi, 8\n");
            } else {
                printf("  imul rdi, %d\n", type->type_size);
            }
            printf("  add rax, rdi\n");
            if (type->type_size == 1 && type->ptr_count < 2) {
                printf("  movzx eax, BYTE PTR [rax]\n");
            } else {
                printf("  mov rax, [rax]\n");
            } 
        }
        printf("  push rax\n");

        break;
    }
    // postfix-expression . identifier
    case PS_DOT: {
        process_postfix_expr_left(node->postfix_expr_node);

        Type* type = stack_top(type_stack);
        stack_pop(type_stack);

        const StructInfo* struct_info = type->struct_info;
        const FieldInfo* field_info = strptrmap_get(struct_info->field_info_map, node->identifier);

        printf("  pop rax\n");
        printf("  add rax, %d\n", field_info->offset);
        printf("  push [rax]\n");
        break;        
    }
    // postfix-expression -> identifier
    case PS_ARROW: {
        process_postfix_expr_left(node->postfix_expr_node);

        Type* type = stack_top(type_stack);
        stack_pop(type_stack);

        const StructInfo* struct_info = type->struct_info;
        const FieldInfo* field_info = strptrmap_get(struct_info->field_info_map, node->identifier);
        stack_push(type_stack, field_info->type);

        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  add rax, %d\n", field_info->offset);
        printf("  push [rax]\n");

        break;        
    }
    // postfix-expression ++
    case PS_INC: {
        process_postfix_expr_left(node->postfix_expr_node);
        printf("  pop rax\n");
        printf("  mov rdi, [rax]\n");
        printf("  push rdi\n");
        printf("  push 1\n");
        printf("  pop rdi\n");
        printf("  add [rax], rdi\n");
        break;                       
    }
    // postfix-expression --
    case PS_DEC: {
        process_postfix_expr_left(node->postfix_expr_node);
        printf("  pop rax\n");
        printf("  mov rdi, [rax]\n");
        printf("  push rdi\n");
        printf("  push 1\n");
        printf("  pop rdi\n");
        printf("  sub [rax], rdi\n");
        break;                       
    }
    default: {
        // @todo
        break;
    }
    }
}

static void process_unary_expr_left(const UnaryExprNode* node) {
    switch (node->type) {
    // postfix-expression
    case UN_NONE: {
        process_postfix_expr_left(node->postfix_expr_node);
        break;
    }
    // ++ unary-expression
    case UN_INC: {
        // @todo
        break;
    }
    case UN_DEC: {
        break;
    }
    case UN_OP: {
        switch (node->op_type) {
        case OP_MUL: {
            process_cast_expr(node->cast_expr_node);

            break;
        }
        default: {
            break;
        }
        }

        break;
    }
    case UN_SIZEOF_IDENT: 
    case UN_SIZEOF_TYPE: {
        break;
    }
    default: {
        break;
    }
    }
}

static void process_cast_expr_left(const CastExprNode* node) {
    // unary-expression
    if (node->unary_expr_node != NULL) {
        process_unary_expr_left(node->unary_expr_node);
    }
    // ( type-name ) cast-expression
    else {
        // @todo
    }
}

static void process_unary_expr_right(const UnaryExprNode* node) {
    switch (node->type) {
    // postfix-expression
    case UN_NONE: {
        process_postfix_expr_right(node->postfix_expr_node);
        break;
    }
    // ++ unary-expression
    case UN_INC: {
        process_unary_expr_left(node->unary_expr_node);

        printf("  pop rax\n");
        printf("  mov rdi, [rax]\n");
        printf("  add rdi, 1\n");
        printf("  mov [rax], rdi\n");

        break;
    }
    // -- unary-expression
    case UN_DEC: {
        process_unary_expr_left(node->unary_expr_node);

        printf("  pop rax\n");
        printf("  mov rdi, [rax]\n");
        printf("  sub rdi, 1\n");
        printf("  mov [rax], rdi\n");

        break;
    }
    // unary-operator cast-expression
    case UN_OP: {
        switch (node->op_type) {
        case OP_AND: {
            process_cast_expr_left(node->cast_expr_node);
            break;
        }
        case OP_MUL: {
            process_cast_expr(node->cast_expr_node);
            printf("  pop rax\n");
            printf("  mov rax, [rax]\n");
            printf("  push rax\n");
            break;
        }
        case OP_ADD: {
            break;
        }
        case OP_SUB: {
            process_cast_expr(node->cast_expr_node);
            printf("  pop rdi\n");
            printf("  mov rax, 0\n");
            printf("  sub rax, rdi\n");
            printf("  push rax\n");
            break;
        }
        case OP_TILDE: {
            break;
        }
        case OP_EXCLA: {
            process_cast_expr(node->cast_expr_node);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  sete al\n");
            printf("  movzb rax, al\n");
            printf("  push rax\n");
            break;
        }
        default: {
            break;
        }
        }

        break;
    }
    case UN_SIZEOF_IDENT: {
        int size = 0;
        const LocalVar* lv = get_localvar(node->sizeof_name);
        if (lv != NULL) {
            size = lv->type->type_size;
        } 
        else {
            const GlobalVar* gv = get_globalvar(node->sizeof_name);
            size = gv->type->type_size;
        }

        printf("  push %d\n", size);

        break;
    }
    case UN_SIZEOF_TYPE: {
        const TypeNameNode* type_name_node = node->type_name_node;
        int size = 0;
        if (type_name_node->is_pointer) {
            size = 8;
        }
        else {
            const TypeSpecifierNode* type_specifier_node = type_name_node->specifier_qualifier_node->type_specifier_node; 
            switch (type_specifier_node->type_specifier) {
            case TYPE_CHAR:   { size = 1; break; }
            case TYPE_INT:    { size = 8; break; }
            case TYPE_DOUBLE: { size = 8; break; } 
            case TYPE_STRUCT: { 
                const StructSpecifierNode* struct_specifier_node = type_specifier_node->struct_specifier_node;
                const char* ident = struct_specifier_node->identifier;
                const StructInfo* struct_info = strptrmap_get(struct_map, ident);
                size = struct_info->size;
                break;                                   
            }
            case TYPE_TYPEDEFNAME: {
                const StructInfo* struct_info = strptrmap_get(struct_map, type_specifier_node->struct_name);
                size = struct_info->size;
                break;
            }
            default: {
                break;
            }
            }
        }

        printf("  push %d\n", size);

        break;
    }
    default: {
        break;
    }
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

        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  imul rax, rdi\n");
        printf("  push rax\n");
    }
    else if (node->operator_type == OP_DIV) {
        process_multiplicative_expr(node->multiplicative_expr_node);
        process_cast_expr(node->cast_expr_node);

        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  cqo\n");
        printf("  idiv rdi\n");
        printf("  push rax\n");
    }
    else if (node->operator_type == OP_MOD) {
        process_multiplicative_expr(node->multiplicative_expr_node);
        process_cast_expr(node->cast_expr_node);

        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  cqo\n");
        printf("  idiv rdi\n");
        printf("  push rdx\n");
    }
}

static void process_additive_expr(const AdditiveExprNode* node) {
    // <multiplicative-expression>
    if (node->additive_expr_node == NULL) {
        process_multiplicative_expr(node->multiplicative_expr_node);
    }
    //  <additive-expression> + <multiplicative-expression>
    else if (node->operator_type == OP_ADD) {
        process_additive_expr(node->additive_expr_node);
        process_multiplicative_expr(node->multiplicative_expr_node);

        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  add rax, rdi\n");
        printf("  push rax\n");
    }
    // <additive-expression> - <multiplicative-expression>
    else if (node->operator_type == OP_SUB) {
        process_additive_expr(node->additive_expr_node);
        process_multiplicative_expr(node->multiplicative_expr_node);

        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  sub rax, rdi\n");
        printf("  push rax\n");
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
    switch (node->cmp_type) {
    // <shift-expression>
    case CMP_NONE: {
        process_shift_expr(node->shift_expr_node);
        break;
    }
    // <relational-expression> <  <shift-expression>
    case CMP_LT: {
        process_relational_expr(node->relational_expr_node);
        process_shift_expr(node->shift_expr_node);

        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        printf("  push rax\n");

        break;
    }
    // <relational-expression> >  <shift-expression>
    case CMP_GT: {
        process_relational_expr(node->relational_expr_node);
        process_shift_expr(node->shift_expr_node);

        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  cmp rax, rdi\n");
        printf("  setg al\n");
        printf("  movzb rax, al\n");
        printf("  push rax\n");

        break;
    }
    // <relational-expression> <= <shift-expression>
    case CMP_LE: {
        process_relational_expr(node->relational_expr_node);
        process_shift_expr(node->shift_expr_node);

        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
        printf("  push rax\n");

        break;
    }
    // <relational-expression> >= <shift-expression>
    case CMP_GE: {
        process_relational_expr(node->relational_expr_node);
        process_shift_expr(node->shift_expr_node);

        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  cmp rax, rdi\n");
        printf("  setge al\n");
        printf("  movzb rax, al\n");
        printf("  push rax\n");

        break;
    }
    default: {
        break;
    }
    }
}

static void process_equality_expr(const EqualityExprNode* node) {
    switch (node->cmp_type) {
    // <relational-expression>
    case CMP_NONE: {
        process_relational_expr(node->relational_expr_node);
        break;
    }
    // <equality-expression> == <relational-expression>
    case CMP_EQ: {
        process_equality_expr(node->equality_expr_node);
        process_relational_expr(node->relational_expr_node);

        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  cmp rax, rdi\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        printf("  push rax\n");

        break;
    }
    // <equality-expression> != <relational-expression>
    case CMP_NE: {
        process_equality_expr(node->equality_expr_node);
        process_relational_expr(node->relational_expr_node);

        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movzb rax, al\n");
        printf("  push rax\n");

        break;
    }
    default:{
        break;
    }
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
        char* label1 = get_label();
        char* label2 = get_label();
        process_logical_and_expr(node->logical_and_expr_node);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je %s\n", label1);
        process_inclusive_or_expr(node->inclusive_or_expr_node);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je %s\n", label1);
        printf("  push 1\n");
        printf("  jmp %s\n", label2);
        printf("%s:\n", label1);
        printf("  push 0\n");
        printf("%s:\n", label2);
    }
}

static void process_logical_or_expr(const LogicalOrExprNode* node) {
    // <logical-and-expression>
    if (node->logical_or_expr_node == NULL) {
        process_logical_and_expr(node->logical_and_expr_node);
    }
    // <logical-or-expression> || <logical-and-expression>
    else {
        process_logical_or_expr(node->logical_or_expr_node);
        process_logical_and_expr(node->logical_and_expr_node);

        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  or rax, rdi\n");
        printf("  push rax\n");
    }
}

static void process_conditional_expr(const ConditionalExprNode* node) {
    // <logical-or-expression>
    if (node->conditional_expr_node == NULL) {
        process_logical_or_expr(node->logical_or_expr_node);
    }
    // <logical-or-expression> ? <expression> : <conditional-expression>
    else {
        const char* label1 = get_label();
        const char* label2 = get_label();
 
        process_logical_or_expr(node->logical_or_expr_node);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je %s\n", label1);
        process_expr(node->expr_node);
        printf("  jmp %s\n", label2);
        printf("%s:\n", label1);
        process_conditional_expr(node->conditional_expr_node);
        printf("%s:\n", label2);
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
            printf("  pop rdi\n");
            printf("  pop rax\n");

            const int size = intstack_top(size_stack); 
            intstack_pop(size_stack);
            printf("  mov [rax], %s\n", get_reg("di", size));

            break;
        }
        case OP_MUL_EQ: {
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  mov rsi, [rax]\n");
            printf("  imul rdi, rsi\n");
            printf("  mov [rax], rdi\n");

            break;
        }
        case OP_DIV_EQ: {
            printf("  pop rdi\n");
            printf("  pop rsi\n");
            printf("  mov rax, [rsi]\n");
            printf("  cqo\n");
            printf("  idiv rdi\n");
            printf("  mov [rsi], rax\n");

            break;
        }
        case OP_MOD_EQ: {
            printf("  pop rdi\n");
            printf("  pop rsi\n");
            printf("  mov rax, [rsi]\n");
            printf("  cqo\n");
            printf("  idiv rdi\n");
            printf("  mov [rsi], rdx\n");

            break;
        }
        case OP_ADD_EQ: {
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  add [rax], rdi\n");

            break;
        }
        case OP_SUB_EQ: {
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  sub [rax], rdi\n");

            break;
        }
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

static void process_expr_left(const ExprNode* node) {
    process_unary_expr_left(node
               ->assign_expr_node
               ->conditional_expr_node
               ->logical_or_expr_node
               ->logical_and_expr_node
               ->inclusive_or_expr_node
               ->exclusive_or_expr_node
               ->and_expr_node
               ->equality_expr_node
               ->relational_expr_node
               ->shift_expr_node
               ->additive_expr_node
               ->multiplicative_expr_node
               ->cast_expr_node
               ->unary_expr_node
    );
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
    case JMP_CONTINUE: {
        const char* label1 = stack_top(continue_label_stack);
        printf("  jmp %s\n", label1);
        break;
    }
    case JMP_BREAK: {
        const char* label2 = stack_top(break_label_stack);
        printf("  jmp %s\n", label2);
        break;
    }
    case JMP_RETURN: {
        if (node->expr_node != NULL) {
            process_expr(node->expr_node);
        }
        printf("  pop rax\n");
        if (ret_label == NULL) {
            ret_label = get_label();
        }
        printf("  jmp %s\n", ret_label);
        break;
    }
    default: {
        break;
    }
    }
}

static void process_selection_stmt(const SelectionStmtNode* node) {
    switch (node->selection_type) {
    case SELECT_IF: {
        const char* label1 = get_label();

        process_expr(node->expr_node);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je %s\n", label1);
        process_stmt(node->stmt_node_0);
        printf("%s:\n", label1);

        break;
    }
    case SELECT_IF_ELSE: {
        const char* label2 = get_label();
        const char* label3 = get_label();

        process_expr(node->expr_node);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je %s\n", label2);
        process_stmt(node->stmt_node_0);
        printf("  jmp %s\n", label3);
        printf("%s:\n", label2);
        process_stmt(node->stmt_node_1);
        printf("%s:\n", label3);

        break;
    }
    case SELECT_SWITCH: {
        char* label4 = get_label();
        stack_push(break_label_stack, label4);
        stack_push(current_stmt_label_stack, get_label()); 

        process_expr(node->expr_node);
        process_stmt(node->stmt_node_0);

        printf("%s:\n", label4);
        stack_pop(break_label_stack);
        stack_pop(current_stmt_label_stack); 
        break;
    }
    default: {
        break;
    }
    }
}

static void process_itr_stmt(const ItrStmtNode* node) {
    switch (node->itr_type) {
    case ITR_WHILE: {
        char* label1 = get_label();
        char* label2 = get_label();
        stack_push(continue_label_stack, label1);
        stack_push(break_label_stack, label2);

        printf("%s:\n", label1);
        process_expr(node->expr_node_0);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je %s\n", label2);
        process_stmt(node->stmt_node);
        printf("  jmp %s\n", label1);
        printf("%s:\n", label2);

        stack_pop(continue_label_stack);
        stack_pop(break_label_stack);
        break;
    }
    case ITR_FOR: {
        char* label3 = get_label();
        char* label4 = get_label();
        char* label5 = get_label();
        stack_push(continue_label_stack, label4);
        stack_push(break_label_stack, label5);

        if (node->declaration_nodes->size != 0) {
            for (int i = 0; i < node->declaration_nodes->size; ++i) {
                process_declaration(node->declaration_nodes->elements[i]);
            }
        }
        else if (node->expr_node_0 != NULL) {
            process_expr(node->expr_node_0);
        }
        printf("%s:\n", label3);
        if (node->expr_node_1 != NULL) {
            process_expr(node->expr_node_1);
        }

        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je %s\n", label5);

        process_stmt(node->stmt_node);

        printf("%s:\n", label4);
        if (node->expr_node_2 != NULL) {
            process_expr(node->expr_node_2);
        }

        printf("  jmp %s\n", label3);
        printf("%s:\n", label5);

        stack_pop(continue_label_stack);
        stack_pop(break_label_stack);
        break;
    }
    default: {
        break;
    }
    }
}

static void process_labeled_stmt(const LabeledStmtNode* node) {
    switch (node->labeled_stmt_type) {
    case LABELED_CASE: {
        process_conditional_expr(node->conditional_expr_node);

        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  push rax\n");
        printf("  cmp rax, rdi\n");
        char* current_stmt_label = stack_top(current_stmt_label_stack);
        if (node->stmt_node->labeled_stmt_node == NULL) {
            const char* label = get_label();
            printf("  jne %s\n", label);
            printf("%s:\n", current_stmt_label);
            process_stmt(node->stmt_node);
            printf("%s:\n", label); 
           
            stack_pop(current_stmt_label_stack);
            stack_push(current_stmt_label_stack, get_label()); 
        } else {
            printf("  je %s\n", current_stmt_label);
            process_labeled_stmt(node->stmt_node->labeled_stmt_node);
        }

        break;
    }
    case LABELED_DEFAULT: {
        process_stmt(node->stmt_node);
        break;
    }
    default: {
        break;
    }
    } 
}

static void process_stmt(const StmtNode* node) {
    if (node->labeled_stmt_node != NULL) {
        process_labeled_stmt(node->labeled_stmt_node);
    }
    else if (node->expr_stmt_node != NULL) {
        process_expr_stmt(node->expr_stmt_node);
    }
    else if (node->itr_stmt_node != NULL) {
        process_itr_stmt(node->itr_stmt_node);
    }
    else if (node->compound_stmt_node != NULL) {
        process_compound_stmt(node->compound_stmt_node);
    }
    else if (node->jump_stmt_node != NULL) {
        process_jump_stmt(node->jump_stmt_node);
    }
    else if (node->selection_stmt_node != NULL) {
        process_selection_stmt(node->selection_stmt_node);
    }
}

static const DirectDeclaratorNode* get_identifier_direct_declarator(const DirectDeclaratorNode* node) {
    const DirectDeclaratorNode* current = node;
    while (current->direct_declarator_node != NULL) {
        current = current->direct_declarator_node;
    }

    return current;
}

static void process_declaration(const DeclarationNode* node) {
    Type* type = NULL;
    for (int i = 0; i < node->decl_specifier_nodes->size; ++i) {
        const DeclSpecifierNode* decl_specifier_node = node->decl_specifier_nodes->elements[i];
        const TypeSpecifierNode* type_specifier_node = decl_specifier_node->type_specifier_node;
        if (type_specifier_node == NULL) {
            continue;
        }

        type = process_type_specifier_in_local(type_specifier_node);
        break;
    }

    // add local-variable to list
    for (int i = 0; i < node->init_declarator_nodes->size; ++i) {
        LocalVar* lv = malloc(sizeof(LocalVar));
        lv->type     = type;

        const InitDeclaratorNode* init_declarator_node = node->init_declarator_nodes->elements[i];
        const DeclaratorNode* declarator_node  = init_declarator_node->declarator_node;

        const PointerNode* pointer_node = declarator_node->pointer_node;
        if (pointer_node != NULL) {
            lv->type->ptr_count = pointer_node->count;
            lv->type->size      = 8;
        } else {
            lv->type->size = lv->type->type_size;
        }

        const DirectDeclaratorNode* direct_declarator_node = declarator_node->direct_declarator_node;
        const ConditionalExprNode*  conditional_expr_node  = direct_declarator_node->conditional_expr_node;
        if (conditional_expr_node == NULL) {
            lv->type->array_size = 0;
            current_offset += lv->type->size;
        } else {
            lv->type->array_size = get_array_size_from_constant_expr(conditional_expr_node);
            current_offset += (lv->type->array_size * lv->type->size);

        }
        lv->offset = align_offset(current_offset, lv->type->size);

        const DirectDeclaratorNode* ident_node = get_identifier_direct_declarator(direct_declarator_node);
        lv->name = strdup(ident_node->identifier);
        vector_push_back(localvar_list, lv);

        if (init_declarator_node->initializer_node != NULL) {
            const InitializerNode* initializer_node = init_declarator_node->initializer_node;
            printf("  lea rax, [rbp-%d]\n", lv->offset);
            printf("  push rax\n");
            // intstack_push(size_stack, 8);

            if (initializer_node->assign_expr_node != NULL) {
                process_assign_expr(initializer_node->assign_expr_node);
            }

            printf("  pop rdi\n");
            printf("  pop rax\n");
            if (lv->type->size == 1) {
                printf("  mov [rax], dil\n");
            } else {
                printf("  mov [rax], rdi\n");
            }
        }
    }
}

static void process_block_item(const BlockItemNode* node) {
    if (node->declaration_node != NULL) {
        process_declaration(node->declaration_node);
    } else {
        process_stmt(node->stmt_node);
    }
}

static void process_compound_stmt(const CompoundStmtNode* node) {
    for (int i = 0; i < node->block_item_nodes->size; ++i) {
        process_block_item(node->block_item_nodes->elements[i]);
    }
}

static int calc_arg_size(const FuncDefNode* node) {
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

    return cnt * 8;
}

static int get_array_size_from_constant_expr(const ConditionalExprNode* node) {
    return node->logical_or_expr_node
               ->logical_and_expr_node
               ->inclusive_or_expr_node
               ->exclusive_or_expr_node
               ->and_expr_node
               ->equality_expr_node
               ->relational_expr_node
               ->shift_expr_node
               ->additive_expr_node
               ->multiplicative_expr_node
               ->cast_expr_node
               ->unary_expr_node
               ->postfix_expr_node
               ->primary_expr_node
               ->constant_node
               ->integer_constant;
}

static int calc_localvar_size_in_labeled_stmt(const LabeledStmtNode* node) {
    return calc_localvar_size_in_stmt(node->stmt_node);
}

static int calc_localvar_size_in_selection_stmt(const SelectionStmtNode* node) {
    int size = 0;
    size += calc_localvar_size_in_stmt(node->stmt_node_0);

    if (node->stmt_node_1 != NULL) {
        size += calc_localvar_size_in_stmt(node->stmt_node_1);
    }

    return size;
}

static int calc_localvar_size_in_itr_stmt(const ItrStmtNode* node) {
    int size = 0;
    for (int i = 0; i < node->declaration_nodes->size; ++i) {
        const DeclarationNode* declaration_node = node->declaration_nodes->elements[i];
        const Vector* init_declarator_nodes = declaration_node->init_declarator_nodes;

        for (int j = 0; j < init_declarator_nodes->size; ++j) {
            const InitDeclaratorNode* init_declarator_node = init_declarator_nodes->elements[j];
            const DeclaratorNode* declarator_node = init_declarator_node->declarator_node;
            const DirectDeclaratorNode* direct_declarator_node = declarator_node->direct_declarator_node;

            if (direct_declarator_node->conditional_expr_node == NULL) {
                size += 8;
            } else {
                const int array_size = get_array_size_from_constant_expr(direct_declarator_node->conditional_expr_node);
                size += (array_size * 8);
            }
        }
    }

    size += calc_localvar_size_in_stmt(node->stmt_node);

    return size;
}

static int calc_localvar_size_in_stmt(const StmtNode* node) {
    if (node->labeled_stmt_node != NULL) {
        return calc_localvar_size_in_labeled_stmt(node->labeled_stmt_node);
    }
    else if (node->compound_stmt_node != NULL) {
        return calc_localvar_size_in_compound_stmt(node->compound_stmt_node);
    }
    else if (node->selection_stmt_node != NULL) {
        return calc_localvar_size_in_selection_stmt(node->selection_stmt_node);
    }
    else if (node->itr_stmt_node != NULL) {
        return calc_localvar_size_in_itr_stmt(node->itr_stmt_node);
    }

    return 0;
}

static int get_varsize_from_decl_specifier_nodes(const Vector* nodes) {
    for (int i = 0; i < nodes->size; ++i) {
        const DeclSpecifierNode* decl_specifier_node = nodes->elements[i];
        if (decl_specifier_node->type_specifier_node == NULL) {
            continue;
        }

        const TypeSpecifierNode* type_specifier_node = decl_specifier_node->type_specifier_node;
        if (type_specifier_node->type_specifier == TYPE_STRUCT) {
            const StructSpecifierNode* struct_specifier_node = type_specifier_node->struct_specifier_node;
            const char* ident = struct_specifier_node->identifier;
            const StructInfo* struct_info = strptrmap_get(struct_map, ident);
            return (struct_info->field_info_map->size * 8);
        } 
        else if (type_specifier_node->type_specifier == TYPE_TYPEDEFNAME) {
            const StructInfo* struct_info = strptrmap_get(struct_map, type_specifier_node->struct_name);
            return (struct_info->field_info_map->size * 8);
        } 
        else {
            return 8; // @todo
        }
    }

    return 0;
}

static int calc_localvar_size_in_declaration(const DeclarationNode* node) {
    // int var_size = 0;
    // const Vector* decl_specifier_nodes = node->decl_specifier_nodes;
    // for (int i = 0; i < decl_specifier_nodes->size; ++i) {
    //     const DeclSpecifierNode* decl_specifier_node = decl_specifier_nodes->elements[i];
    //     if (decl_specifier_node->type_specifier_node == NULL) {
    //         continue;
    //     }

    //     const TypeSpecifierNode* type_specifier_node = decl_specifier_node->type_specifier_node;
    //     if (type_specifier_node->type_specifier == TYPE_STRUCT) {
    //         const StructSpecifierNode* struct_specifier_node = type_specifier_node->struct_specifier_node;
    //         const char* ident = struct_specifier_node->identifier;
    //         const StructInfo* struct_info = strptrmap_get(struct_map, ident);
    //         var_size = struct_info->field_info_map->size * 8;
    //     } 
    //     else if (type_specifier_node->type_specifier == TYPE_TYPEDEFNAME) {
    //         const StructInfo* struct_info = strptrmap_get(struct_map, type_specifier_node->struct_name);
    //         var_size = struct_info->field_info_map->size * 8;
    //     } 
    //     else {
    //         var_size = 8; // @todo
    //     }

    //     break;
    // }

    const int var_size = get_varsize_from_decl_specifier_nodes(node->decl_specifier_nodes);
    int size = 0;
    const Vector* init_declarator_nodes = node->init_declarator_nodes;
    for (int i = 0; i < init_declarator_nodes->size; ++i) {
        const InitDeclaratorNode* init_declarator_node = init_declarator_nodes->elements[i];
        const DeclaratorNode* declarator_node = init_declarator_node->declarator_node;
        const DirectDeclaratorNode* direct_declarator_node = declarator_node->direct_declarator_node;

        if (declarator_node->pointer_node != NULL) {
            size += 8;
        }
        else if (direct_declarator_node->conditional_expr_node == NULL) {
            size += var_size;
        } 
        else {
            const int array_size = get_array_size_from_constant_expr(direct_declarator_node->conditional_expr_node);
            size += (array_size * var_size);
        }
    }

    return size;
}

static int calc_localvar_size_in_compound_stmt(const CompoundStmtNode* node) {
    if (node == NULL) {
        return 0;
    }

    int size = 0;
    for (int i = 0; i < node->block_item_nodes->size; ++i) {
        const BlockItemNode* block_item_node = node->block_item_nodes->elements[i];
        if (block_item_node->declaration_node != NULL) {
            size += calc_localvar_size_in_declaration(block_item_node->declaration_node);
        } else {
            size += calc_localvar_size_in_stmt(block_item_node->stmt_node);
        }
    }

    return size;
}

static Type* process_type_specifier_in_local(const TypeSpecifierNode* node) {
    Type* type = calloc(1, sizeof(Type));

    switch (node->type_specifier) {
    case TYPE_VOID: { 
        type->base_type = VAR_VOID;
        type->type_size = 8;
        return type;
    }
    case TYPE_CHAR: { 
        type->base_type = VAR_CHAR;
        type->type_size = 1;
        return type;
    }
    case TYPE_INT: {
        type->base_type = VAR_INT;
        type->type_size = 8;
        return type;
    }
    case TYPE_DOUBLE: { 
        type->base_type = VAR_DOUBLE;
        type->type_size = 8;
        return type;
    }
    case TYPE_STRUCT: { 
        type->base_type = VAR_STRUCT;   

        type->struct_info = strptrmap_get(struct_map, node->struct_specifier_node->identifier); 
        if (type->struct_info == NULL) {
            error("Invalid sturct name=\"%s\"\n", node->struct_specifier_node->identifier);
            return NULL;
        }

        type->type_size = type->struct_info->size;
        return type;
    } 
    case TYPE_TYPEDEFNAME: { 
        type->base_type = VAR_STRUCT;   

        type->struct_info = strptrmap_get(struct_map, node->struct_name);
        if (type->struct_info == NULL) {
            error("Invalid sturct name=\"%s\"\n", node->struct_name);
            return NULL;
        }

        type->type_size = type->struct_info->size;
        return type;
    } 
    default: { 
        return NULL;
    }
    }
}

static int count_param_list_node(const ParamListNode* node) {
    int cnt = 0;
    const ParamListNode* current = node;
    while (current != NULL) {
        current = current->param_list_node;
        ++cnt;
    }

    return cnt;
}

static void process_param_list_node(const ParamListNode* node, int arg_index) {
    const ParamDeclarationNode* param_declaration_node = node->param_declaration_node;
    const DeclaratorNode* declarator_node = param_declaration_node->declarator_node;
    if (declarator_node == NULL) {
        return;
    }

    // create type
    Type* type = NULL;
    for (int i = 0; i < param_declaration_node->decl_spec_nodes->size; ++i) {
        const DeclSpecifierNode* decl_specifier_node = param_declaration_node->decl_spec_nodes->elements[i];
        const TypeSpecifierNode* type_specifier_node = decl_specifier_node->type_specifier_node;
        if (type_specifier_node == NULL) {
            continue;
        }

        type = process_type_specifier_in_local(type_specifier_node);
        break;
    }

    current_offset += 8;

    const DirectDeclaratorNode* direct_declarator_node = declarator_node->direct_declarator_node;
    LocalVar* lv = malloc(sizeof(LocalVar));
    lv->type     = type;
    lv->offset   = current_offset;
    lv->name     = strdup(direct_declarator_node->identifier);
    vector_push_back(localvar_list, lv);

    const PointerNode* pointer_node = declarator_node->pointer_node;
    if (pointer_node != NULL) {
        lv->type->ptr_count = pointer_node->count;
        lv->type->size      = 8;
    } else {
        lv->type->size = lv->type->type_size;
    }

    printf("  mov [rbp-%d], %s\n", lv->offset, arg_registers[arg_index]);
}

static void process_args(const ParamListNode* node) {
    const int param_count = count_param_list_node(node);

    const ParamListNode* current = node;
    int index = 1;
    while (current != NULL) {
        process_param_list_node(current, param_count - index);
        current = current->param_list_node;
        ++index;
    }
}

static void process_func_declarator(const DeclaratorNode* node) {
    const DirectDeclaratorNode* direct_declarator_node = node->direct_declarator_node;
    const ParamTypeListNode* param_type_list_node = direct_declarator_node->param_type_list_node;
    if (param_type_list_node == NULL) {
        return;
    }

    const ParamListNode* param_list_node = param_type_list_node->param_list_node;
    process_args(param_list_node);
}

static void process_func_def(const FuncDefNode* node) {
    localvar_list = create_vector();
    current_offset = 0;

    const DeclaratorNode* declarator_node = node->declarator_node;
    const DirectDeclaratorNode* direct_declarator_node = declarator_node->direct_declarator_node;
    printf(".global %s\n", get_ident_from_direct_declarator(direct_declarator_node));
    printf("%s:\n",        get_ident_from_direct_declarator(direct_declarator_node));

    const int localvar_size = calc_localvar_size_in_compound_stmt(node->compound_stmt_node);
    const int arg_size      = calc_arg_size(node);

    // prologue
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", (localvar_size + arg_size));

    process_func_declarator(declarator_node);
    process_compound_stmt(node->compound_stmt_node);

    // epilogue
    if (ret_label != NULL) {
        printf("%s:\n", ret_label);
    }
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");

    free(localvar_list);
    free(ret_label);
    ret_label = NULL;
    current_offset = 0;
}

static Type* process_type_specifier_in_global(const TypeSpecifierNode* node) {
    Type* type = calloc(1, sizeof(Type));

    switch (node->type_specifier) {
    case TYPE_VOID: { 
        type->base_type = VAR_VOID;
        type->type_size = 8;
        return type;
    }
    case TYPE_CHAR: { 
        type->base_type = VAR_CHAR;
        type->type_size = 1;
        return type;
    }
    case TYPE_INT: {
        type->base_type = VAR_INT;
        type->type_size = 8;
        return type;
    }
    case TYPE_DOUBLE: { 
        type->base_type = VAR_DOUBLE;
        type->type_size = 8;
        return type;
    }
    case TYPE_STRUCT: { 
        type->base_type = VAR_STRUCT;   
        type->type_size = 0;

        const StructSpecifierNode* struct_specifier_node = node->struct_specifier_node;         

        const char* struct_name = struct_specifier_node->identifier;
        const Vector* struct_declaration_nodes = struct_specifier_node->struct_declaration_nodes;
        //
        //  "struct" identifier { {struct-declaration}+ }
        //  
        if (struct_declaration_nodes->size != 0) {
            type->struct_info = strptrmap_get(struct_map, struct_name);
            if (type->struct_info == NULL) { 
                type->struct_info = malloc(sizeof(StructInfo));
                type->struct_info->field_info_map = create_strptrmap(1024);
                type->struct_info->size           = struct_declaration_nodes->size * 8; // @todo
                strptrmap_put(struct_map, struct_name, type->struct_info);
            } else {
                type->struct_info->field_info_map = create_strptrmap(1024);
                type->struct_info->size           = struct_declaration_nodes->size * 8; // @todo
            }

            int offset = 0;
            for (int i = 0; i < struct_declaration_nodes->size; ++i) {
                const StructDeclarationNode* struct_declaration_node  = struct_declaration_nodes->elements[i];
                const Vector* specifier_qualifier_nodes = struct_declaration_node->specifier_qualifier_nodes;

                Type* field_type = NULL;
                for (int j = 0; j < specifier_qualifier_nodes->size; ++j) {
                    const SpecifierQualifierNode* specifier_qualifier_node = specifier_qualifier_nodes->elements[j];
                    const TypeSpecifierNode*      type_specifier_node      = specifier_qualifier_node->type_specifier_node;
                    if (type_specifier_node == NULL) {
                        continue;
                    }

                    field_type = process_type_specifier_in_global(type_specifier_node);
                    break;
                }

                const PointerNode* pointer_node = struct_declaration_node->pointer_node;
                if (pointer_node != NULL) {
                    field_type->ptr_count = pointer_node->count;
                    field_type->size      = 8; 
                } else {
                    field_type->size = field_type->type_size;
                }

                FieldInfo* field_info = malloc(sizeof(FieldInfo));
                field_info->type      = field_type;

                field_info->offset = offset;
                offset += field_info->type->size;

                strptrmap_put(type->struct_info->field_info_map, struct_declaration_node->identifier, field_info);
            }
            type->struct_info->size = type->struct_info->field_info_map->size * 8;
        }
        //
        // "struct" identifier
        //
        else {
            // @todo
        }

        return type;
    }
    case TYPE_TYPEDEFNAME: {
        type->base_type = VAR_STRUCT;   
        StructInfo* struct_info = strptrmap_get(struct_map, node->struct_name);
        if (struct_info == NULL) {
            struct_info = calloc(1, sizeof(StructInfo));
            strptrmap_put(struct_map, node->struct_name, struct_info);
        }
        type->struct_info = struct_info;
        type->type_size   = struct_info->size;
        return type;
    }
    default: {
        return NULL;
    }
    }
}

static int get_int_constant(const AssignExprNode* node) {
    return node->conditional_expr_node
               ->logical_or_expr_node
               ->logical_and_expr_node
               ->inclusive_or_expr_node
               ->exclusive_or_expr_node
               ->and_expr_node
               ->equality_expr_node
               ->relational_expr_node
               ->shift_expr_node
               ->additive_expr_node
               ->multiplicative_expr_node
               ->cast_expr_node
               ->unary_expr_node
               ->postfix_expr_node
               ->primary_expr_node
               ->constant_node
               ->integer_constant;
}

static const char* get_character_constant(const AssignExprNode* node) {
    return node->conditional_expr_node
               ->logical_or_expr_node
               ->logical_and_expr_node
               ->inclusive_or_expr_node
               ->exclusive_or_expr_node
               ->and_expr_node
               ->equality_expr_node
               ->relational_expr_node
               ->shift_expr_node
               ->additive_expr_node
               ->multiplicative_expr_node
               ->cast_expr_node
               ->unary_expr_node
               ->postfix_expr_node
               ->primary_expr_node
               ->constant_node
               ->character_constant;
}


static bool is_int_constant(const AssignExprNode* node) {
     return (CONST_INT == node->conditional_expr_node
                              ->logical_or_expr_node
                              ->logical_and_expr_node
                              ->inclusive_or_expr_node
                              ->exclusive_or_expr_node
                              ->and_expr_node
                              ->equality_expr_node
                              ->relational_expr_node
                              ->shift_expr_node
                              ->additive_expr_node
                              ->multiplicative_expr_node
                              ->cast_expr_node
                              ->unary_expr_node
                              ->postfix_expr_node
                              ->primary_expr_node
                              ->constant_node
                              ->const_type);
}

static void process_global_declaration(const DeclarationNode* node) {
    // create type
    Type* type = NULL;
    for (int i = 0; i < node->decl_specifier_nodes->size; ++i) {
        const DeclSpecifierNode* decl_specifier_node = node->decl_specifier_nodes->elements[i];
        const TypeSpecifierNode* type_specifier_node = decl_specifier_node->type_specifier_node;
        if (type_specifier_node == NULL) {
            continue;
        }

        type = process_type_specifier_in_global(type_specifier_node);
        break;
    }

    for (int i = 0; i < node->init_declarator_nodes->size; ++i) {
        GlobalVar* gv       = malloc(sizeof(GlobalVar));
        gv->type            = type;
        gv->type->ptr_count = 0;

        const InitDeclaratorNode* init_declarator_node = node->init_declarator_nodes->elements[i];
        const DeclaratorNode* declarator_node  = init_declarator_node->declarator_node;

        const PointerNode* pointer_node = declarator_node->pointer_node;
        if (pointer_node != NULL) {
            gv->type->ptr_count = pointer_node->count;
            gv->type->size      = 8;
        } else {
            gv->type->size = gv->type->type_size;
        }

        const DirectDeclaratorNode* direct_declarator_node = declarator_node->direct_declarator_node;
        const ConditionalExprNode*  conditional_expr_node  = direct_declarator_node->conditional_expr_node;
        if (conditional_expr_node == NULL) {
            gv->type->array_size = 0;
        } else {
            gv->type->array_size = get_array_size_from_constant_expr(conditional_expr_node);
        }

        const DirectDeclaratorNode* ident_node = get_identifier_direct_declarator(direct_declarator_node);
        if (init_declarator_node->initializer_node != NULL) {
            printf(".global %s\n", ident_node->identifier);
        } else {
            printf(".comm %s,8,8\n", ident_node->identifier);
        }

        gv->name = strdup(ident_node->identifier);
        vector_push_back(globalvar_list, gv);

        if (init_declarator_node->initializer_node != NULL) {
            const InitializerNode* initializer_node = init_declarator_node->initializer_node;

            if (initializer_node->assign_expr_node != NULL) {
                if (is_int_constant(initializer_node->assign_expr_node)) {
                    const int int_constant = get_int_constant(initializer_node->assign_expr_node);
                    printf(".data\n");
                    printf("%s:\n", gv->name);
                    printf("  .quad %d\n", int_constant); 
                    printf(".text\n");
                } 
                else {
                    const char* label = get_string_label();
                    printf(".data\n");
                    printf("%s:\n", label);
                    printf("  .string \"%s\"\n", get_character_constant(initializer_node->assign_expr_node));
                    printf("%s:\n", gv->name);
                    printf("  .quad %s\n", label); 
                    printf(".text\n");
                }
            } 
            else {
                InitializerListNode* initializer_list_node = initializer_node->initializer_list_node;
                const InitializerNode* first = initializer_list_node->initializer_nodes->elements[0];
                if (is_int_constant(first->assign_expr_node)) {
                    printf(".data\n");
                    printf("%s:\n", gv->name);

                    for (int j = 0; j < initializer_list_node->initializer_nodes->size; ++j) {
                        const InitializerNode* init = initializer_list_node->initializer_nodes->elements[j];
                        const int int_constant = get_int_constant(init->assign_expr_node);
                        printf("  .quad %d\n", int_constant); 
                    }
                    printf(".text\n");
                } else {
                    Vector* label_list = create_vector();
                    printf(".data\n");
                    for (int j = 0; j < initializer_list_node->initializer_nodes->size; ++j) {
                        const InitializerNode* init = initializer_list_node->initializer_nodes->elements[j];
                        char* label = get_string_label();
                        printf("%s:\n", label);
                        printf("  .string \"%s\"\n", get_character_constant(init->assign_expr_node));

                        vector_push_back(label_list, label);
                   }

                   printf("%s:\n", gv->name);
                   for (int j = 0; j < label_list->size; ++j) {
                       char* l = label_list->elements[j];
                       printf("  .quad %s\n", l);
                   }
                   printf(".text\n");
                }
            }
        }
    }
}

static void process_enum_specifier(const EnumSpecifierNode* node) {
    const EnumeratorListNode* enumerator_list_node = node->enumerator_list_node;
    const Vector* identifiers = enumerator_list_node->identifiers;

    for (int i = 0; i < identifiers->size; ++i) {
        strintmap_put(enum_map, identifiers->elements[i], i);
    }
}

static void process_external_decl(const ExternalDeclNode* node) {
    if (node->enum_specifier_node != NULL) {
        process_enum_specifier(node->enum_specifier_node);
    }

    if (node->declaration_node != NULL) {
        process_global_declaration(node->declaration_node);
    }

    if (node->func_def_node != NULL) {
        process_func_def(node->func_def_node);
    }
}

void gen(const TransUnitNode* node) {
    printf(".intel_syntax noprefix\n");

    // init
    label_index              = 2;
    break_label_stack        = create_stack();
    continue_label_stack     = create_stack();
    type_stack               = create_stack();
    current_stmt_label_stack = create_stack();
    size_stack               = create_intstack();
    globalvar_list           = create_vector();
    struct_map               = create_strptrmap(1024);
    enum_map                 = create_strintmap(1024);

    // gen
    for (int i = 0; i < node->external_decl_nodes->size; ++i) {
        process_external_decl(node->external_decl_nodes->elements[i]);
    }
}
