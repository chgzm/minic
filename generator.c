#include "generator.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "util.h"

//
// global
//

static char* arg_registers[] = {
    "rdi", "rsi", "rdx", "rcx", "r8", "r9"
}; 
static int label_index;
static int string_index;
static int current_offset;
static Vector* localvar_list;
static Vector* globalvar_list;
static StrPtrMap* struct_map;  
static StrIntMap*  enum_map;
static char* ret_label;
static Stack* break_label_stack;
static Stack* continue_label_stack;

//
// forward declaration
//

static LocalVar*  get_localvar(const char* str, int len);
static GlobalVar* get_globalvar(const char* str, int len);
static void process_expr(const ExprNode* node);
static void process_stmt(const StmtNode* node);
static void process_conditional_expr(const ConditionalExprNode* node);
static void process_compound_stmt(const CompoundStmtNode* node);
static void process_cast_expr(const CastExprNode* node);
static void process_declaration(const DeclarationNode* node);
static Type* process_type_specifier_in_local(const TypeSpecifierNode* node);
static int get_array_size_from_constant_expr(const ConditionalExprNode* node);

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

static const char* get_ident_from_direct_declarator(const DirectDeclaratorNode* node) {
    const DirectDeclaratorNode* current = node;
    while (current->direct_declarator_node != NULL) {
        current = current->direct_declarator_node;
    }

    return current->identifier;
}

static void print_global(const TransUnitNode* node) {
    for (int i = 0; i < node->external_decl_nodes->size; ++i) {
        const ExternalDeclNode* external_decl_node = node->external_decl_nodes->elements[i];
        if (external_decl_node->func_def_node != NULL) {
            const FuncDefNode*          func_def_node          = external_decl_node->func_def_node;
            const DeclaratorNode*       declarator_node        = func_def_node->declarator_node;
            const DirectDeclaratorNode* direct_declarator_node = declarator_node->direct_declarator_node;
            printf(".global %s\n", get_ident_from_direct_declarator(direct_declarator_node));
        }
        else if (external_decl_node->declaration_node != NULL) {
            const DeclarationNode* declaration_node = external_decl_node->declaration_node;
            for (int j = 0; j < declaration_node->init_declarator_nodes->size; ++j) {
                const InitDeclaratorNode*   init_declarator_node   = declaration_node->init_declarator_nodes->elements[j];
                const DeclaratorNode*       declarator_node        = init_declarator_node->declarator_node;
                const DirectDeclaratorNode* direct_declarator_node = declarator_node->direct_declarator_node;
                printf(".global %s\n", get_ident_from_direct_declarator(direct_declarator_node));
            }
        }
    }
}

static const char* get_label() {
    const char* label = fmt(".L%d", label_index);
    ++label_index;
    return label;
}

static const char* get_string_label() {
     const char* label = fmt(".LC%d", string_index);
    ++string_index;
    return label;
}

static int align_offset(int offset) {
    if (offset % 8 != 0) {
        offset += (8 - offset % 8);
    }

    return offset;
}

static void process_identifier_left(const char* identifier, int len) {
    const LocalVar* lv = get_localvar(identifier, len);
    if (lv != NULL) {
        print_code("mov rax, rbp");
        print_code("sub rax, %d", lv->offset);
        print_code("push rax");
    } 
    else {
        const GlobalVar* gv = get_globalvar(identifier, len);    
        print_code("lea rax, %s[rip]", gv->name);
        print_code("push rax");
    }
}

static void process_identifier_right(const char* identifier, int len) {
    // enum 
    if (strintmap_contains(enum_map, identifier)) {
        print_code("push %d", strintmap_get(enum_map, identifier));
        return;
    } 

    // local variable
    const LocalVar* lv = get_localvar(identifier, len);
    if (lv != NULL) {
        print_code("mov rax, rbp");
        print_code("sub rax, %d", lv->offset);
        if (lv->type->array_size == 0) { 
            print_code("mov rax, [rax]");
        } 
        print_code("push rax");
        return;
    } 
    // global variable
    else {
        const GlobalVar* gv = get_globalvar(identifier, len);
        print_code("mov rax, %s[rip]", gv->name);
        print_code("push rax");
    }
}

static void process_identifier_addr(const char* identifier, int len) {
    const LocalVar* lv = get_localvar(identifier, len);
    if (lv != NULL) {
        print_code("mov rax, rbp");
        print_code("sub rax, %d", lv->offset);
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
        const char* label = get_string_label();
        printf(".data\n");
        printf("%s:\n", label);
        print_code(".string \"%s\"", node->character_constant);
        print_code(".text\n");
        print_code("lea rax, %s[rip]", label);
        print_code("push rax");
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
        process_expr(node->expr_node);
    }
    // identifier
    else if (node->identifier != NULL) {
        process_identifier_left(node->identifier, node->identifier_len);
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
        process_identifier_right(node->identifier, node->identifier_len);
    }
    else {
        // @todo
    }
}

static void process_primary_expr_addr(const PrimaryExprNode* node) {
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
        process_identifier_addr(node->identifier, node->identifier_len);
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
    // <postfix-expression> [ <expression> ]
    case PS_LSQUARE: {
        process_postfix_expr_left(node->postfix_expr_node);
        process_expr(node->expr_node);
        const char* identifier = node->postfix_expr_node->primary_expr_node->identifier;
        const LocalVar* lv = get_localvar(identifier, node->postfix_expr_node->primary_expr_node->identifier_len);

        print_code("pop rdi");
        print_code("pop rax");
        // print_code("imul rdi, %d", lv->type->type_size);
        print_code("imul rdi, 8");
        if (lv->type->ptr_count != 0) { 
            print_code("mov rax, [rax]");
        } 
        print_code("sub rax, rdi");
        print_code("push rax");

        break;
    }
    // postfix-expression . identifier
    case PS_DOT: {
        const char* identifier = node->postfix_expr_node->primary_expr_node->identifier;
        const int len = node->postfix_expr_node->primary_expr_node->identifier_len;
        const LocalVar* lv = get_localvar(identifier, len);
    
        const StructInfo* struct_info = lv->type->struct_info;
        const FieldInfo* field_info = strptrmap_get(struct_info->field_info_map, node->identifier);

        print_code("mov rax, rbp");
        print_code("sub rax, %d", lv->offset + field_info->offset);
        print_code("push rax");

        break;        
    }
    // postfix-expression -> identifier
    case PS_ARROW: {
        process_postfix_expr_left(node->postfix_expr_node);

        const char* identifier = node->postfix_expr_node->primary_expr_node->identifier;
        const int len = node->postfix_expr_node->primary_expr_node->identifier_len;
        const LocalVar* lv = get_localvar(identifier, len);

        const StructInfo* struct_info = lv->type->struct_info;
        const FieldInfo* field_info = strptrmap_get(struct_info->field_info_map, node->identifier);

        print_code("pop rax");
        print_code("mov rax, [rax]");
        print_code("sub rax, %d", field_info->offset);
        print_code("push rax");

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
            const AssignExprNode* assign_expr_node = (const AssignExprNode*)(node->assign_expr_nodes->elements[i]);
            process_conditional_expr(assign_expr_node->conditional_expr_node);
        }

        for (int i = 0; i < node->assign_expr_nodes->size; ++i) {
            print_code("pop rax");
            print_code("mov %s, rax", arg_registers[i]);
        }

        const char* identifier = node->postfix_expr_node->primary_expr_node->identifier;
        print_code("mov rax, 0");
        print_code("call %s", identifier);
        print_code("push rax");

        break;
    }
    // postfix-expression [ expression ]
    case PS_LSQUARE: {
        // const char* identifier = node->postfix_expr_node->primary_expr_node->identifier;
        // LocalVar* lv = get_localvar(identifier, node->postfix_expr_node->primary_expr_node->identifier_len);

        process_postfix_expr_right(node->postfix_expr_node);
        process_expr(node->expr_node);
        print_code("pop rdi");
        print_code("pop rax");
        // print_code("imul rdi, %d", lv->type->type_size);
        print_code("imul rdi, 8");
        print_code("sub rax, rdi");
        print_code("mov rax, [rax]");
        print_code("push rax");

        break;
    }
    // postfix-expression . identifier
    case PS_DOT: {
        const char* identifier = node->postfix_expr_node->primary_expr_node->identifier;
        const int len = node->postfix_expr_node->primary_expr_node->identifier_len;
        const LocalVar* lv = get_localvar(identifier, len);
    
        const StructInfo* struct_info = lv->type->struct_info;
        const FieldInfo* field_info = strptrmap_get(struct_info->field_info_map, node->identifier);

        print_code("mov rax, rbp");
        print_code("sub rax, %d", lv->offset + field_info->offset);
        print_code("push [rax]");

        break;        
    }
    // postfix-expression -> identifier
    case PS_ARROW: {
        process_postfix_expr_left(node->postfix_expr_node);

        const char* identifier = node->postfix_expr_node->primary_expr_node->identifier;
        const int len = node->postfix_expr_node->primary_expr_node->identifier_len;
        const LocalVar* lv = get_localvar(identifier, len);

        const StructInfo* struct_info = lv->type->struct_info;
        const FieldInfo* field_info = strptrmap_get(struct_info->field_info_map, node->identifier);

        print_code("pop rax");
        print_code("mov rax, [rax]");
        print_code("sub rax, %d", field_info->offset);
        print_code("push [rax]");

        break;        
    }

    default: {
        // @todo
        break;
    }
    }
}

static void process_postfix_expr_addr(const PostfixExprNode* node) {
    switch (node->postfix_expr_type) {
    // primary-expression
    case PS_PRIMARY: {
        process_primary_expr_addr(node->primary_expr_node);
        break;
    }
    // postfix-expression ( {assignment-expression}* )
    case PS_LPAREN: {
        break;
    }
    // postfix-expression [ expression ]
    case PS_LSQUARE: {
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
    case UN_SIZEOF: {
        break;
    }
    default: {
        break;
    }
    }
}

static void process_unary_expr_addr(const UnaryExprNode* node) {
    switch (node->type) {
    // postfix-expression
    case UN_NONE: {
        process_postfix_expr_addr(node->postfix_expr_node);
        break;
    }
    // ++ unary-expression
    case UN_INC: {
        break;
    }
    case UN_DEC: {
        break;
    }
    case UN_SIZEOF: {
        break;
    }
    default: {
        break;
    }
    }
}


static void process_cast_expr_addr(const CastExprNode* node) {
    // unary-expression
    if (node->unary_expr_node != NULL) {
        process_unary_expr_addr(node->unary_expr_node);
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

        print_code("pop rax");
        print_code("mov rdi, [rax]");
        print_code("add rdi, 1");
        print_code("mov [rax], rdi");

        break;
    }
    // -- unary-expression
    case UN_DEC: {
        process_unary_expr_left(node->unary_expr_node);

        print_code("pop rax");
        print_code("mov rdi, [rax]");
        print_code("sub rdi, 1");
        print_code("mov [rax], rdi");

        break;
    }
    // unary-operator cast-expression
    case UN_OP: {
        switch (node->op_type) {
        case OP_AND: {
            process_cast_expr_addr(node->cast_expr_node);

            break;
        }
        case OP_MUL: {
            process_cast_expr(node->cast_expr_node);
            print_code("pop rax");
            print_code("mov rax, [rax]");
            print_code("push rax");

            break;
        }
        case OP_ADD: {
            break;
        }
        case OP_SUB: {
            break;
        }
        case OP_TILDE: {
            break;
        }
        case OP_EXCLA: {
            break;
        }
        default: {
            break;
        }
        }

        break;
    }
    case UN_SIZEOF: {
        int size = 0;
        switch (node->sizeof_type) {
        case SIZEOFTYPE_CHAR:   { size = 1; break; }
        case SIZEOFTYPE_SHORT:  { size = 2; break; }
        case SIZEOFTYPE_INT:    { size = 4; break; }
        case SIZEOFTYPE_LONG:   { size = 8; break; }
        case SIZEOFTYPE_FLOAT:  { size = 4; break; }
        case SIZEOFTYPE_DOUBLE: { size = 8; break; } 
        case SIZEOFTYPE_IDENT:  { 
            const LocalVar* lv = get_localvar(node->sizeof_name, node->sizeof_name_len);
            if (lv != NULL) {
                size = lv->type->type_size;
                break;
            } 

            const GlobalVar* gv = get_globalvar(node->sizeof_name, node->sizeof_name_len);
            if (gv != NULL) {
                size = gv->type->type_size;
                break;
            } 

            break;                                   
        }
        default: {
            break;
        }
        }

        print_code("push %d", size);

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

        print_code("pop rdi");
        print_code("pop rax");
        print_code("cmp rax, rdi");
        print_code("setl al");
        print_code("movzb rax, al");
        print_code("push rax");

        break;
    }
    // <relational-expression> >  <shift-expression>
    case CMP_GT: {
        process_relational_expr(node->relational_expr_node);
        process_shift_expr(node->shift_expr_node);

        print_code("pop rdi");
        print_code("pop rax");
        print_code("cmp rax, rdi");
        print_code("setg al");
        print_code("movzb rax, al");
        print_code("push rax");

        break;
    }
    // <relational-expression> <= <shift-expression>
    case CMP_LE: {
        process_relational_expr(node->relational_expr_node);
        process_shift_expr(node->shift_expr_node);

        print_code("pop rdi");
        print_code("pop rax");
        print_code("cmp rax, rdi");
        print_code("setle al");
        print_code("movzb rax, al");
        print_code("push rax");

        break;
    }
    // <relational-expression> >= <shift-expression>
    case CMP_GE: {
        process_relational_expr(node->relational_expr_node);
        process_shift_expr(node->shift_expr_node);

        print_code("pop rdi");
        print_code("pop rax");
        print_code("cmp rax, rdi");
        print_code("setge al");
        print_code("movzb rax, al");
        print_code("push rax");

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

        print_code("pop rdi");
        print_code("pop rax");
        print_code("cmp rax, rdi");
        print_code("sete al");
        print_code("movzb rax, al");
        print_code("push rax");

        break;
    }
    // <equality-expression> != <relational-expression>
    case CMP_NE: {
        process_equality_expr(node->equality_expr_node);
        process_relational_expr(node->relational_expr_node);

        print_code("pop rdi");
        print_code("pop rax");
        print_code("cmp rax, rdi");
        print_code("setne al");
        print_code("movzb rax, al");
        print_code("push rax");

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
        process_logical_and_expr(node->logical_and_expr_node);
        process_inclusive_or_expr(node->inclusive_or_expr_node);

        print_code("pop rdi");
        print_code("pop rax");
        print_code("and rax, rdi");
        print_code("push rax");
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

        print_code("pop rdi");
        print_code("pop rax");
        print_code("or rax, rdi");
        print_code("push rax");
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

            break;
        }
        case OP_MUL_EQ: {
            print_code("pop rdi");
            print_code("pop rax");
            print_code("mov rsi, [rax]");
            print_code("imul rdi, rsi");
            print_code("mov [rax], rdi");

            break;
        }
        case OP_DIV_EQ: {
            print_code("pop rdi");
            print_code("pop rsi");
            print_code("mov rax, [rsi]");
            print_code("cqo");
            print_code("idiv rdi");
            print_code("mov [rsi], rax");

            break;
        }
        case OP_MOD_EQ: {
            print_code("pop rdi");
            print_code("pop rsi");
            print_code("mov rax, [rsi]");
            print_code("cqo");
            print_code("idiv rdi");
            print_code("mov [rsi], rdx");

            break;
        }
        case OP_ADD_EQ: {
            print_code("pop rdi");
            print_code("pop rax");
            print_code("add [rax], rdi");

            break;
        }
        case OP_SUB_EQ: {
            print_code("pop rdi");
            print_code("pop rax");
            print_code("sub [rax], rdi");

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
        const char* label = stack_top(continue_label_stack);
        print_code("jmp %s", label);

        break;
    }
    case JMP_BREAK: {
        const char* label = stack_top(break_label_stack);
        print_code("jmp %s", label);

        break;
    }
    case JMP_RETURN: {
        if (node->expr_node != NULL) {
            process_expr(node->expr_node);
        }
        print_code("pop rax");
        if (ret_label == NULL) {
            ret_label = (char*)(get_label());
        }
        print_code("jmp %s", ret_label);

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
        const char* label = get_label();

        process_expr(node->expr_node);
        print_code("pop rax");
        print_code("cmp rax, 0");
        print_code("je %s", label);
        process_stmt(node->stmt_node[0]);
        printf("%s:\n", label);

        break;
    }
    case SELECT_IF_ELSE: {
        const char* label1 = get_label();
        const char* label2 = get_label();

        process_expr(node->expr_node);
        print_code("pop rax");
        print_code("cmp rax, 0");
        print_code("je %s", label1);
        process_stmt(node->stmt_node[0]);
        print_code("jmp %s", label2);
        printf("%s:\n", label1);
        process_stmt(node->stmt_node[1]);
        printf("%s:\n", label2);

        break;
    }
    case SELECT_SWITCH: {
        const char* label = get_label();
        stack_push(break_label_stack, (void*)(label));

        process_expr(node->expr_node);
        process_stmt(node->stmt_node[0]);

        printf("%s:\n", label);

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
        const char* label1 = get_label();
        const char* label2 = get_label();
        stack_push(continue_label_stack, (void*)(label1));
        stack_push(break_label_stack, (void*)(label2));

        printf("%s:\n", label1);
        process_expr(node->expr_node[0]);
        print_code("pop rax");
        print_code("cmp rax, 0");
        print_code("je %s", label2);
        process_stmt(node->stmt_node);
        print_code("jmp %s", label1);
        printf("%s:\n", label2);

        stack_pop(continue_label_stack);
        stack_pop(break_label_stack);
        break;
    }
    case ITR_FOR: {
        const char* label1 = get_label();
        const char* label2 = get_label();
        const char* label3 = get_label();
        stack_push(continue_label_stack, (void*)(label2));
        stack_push(break_label_stack, (void*)(label3));

        if (node->declaration_nodes->size != 0) {
            for (int i = 0; i < node->declaration_nodes->size; ++i) {
                process_declaration(node->declaration_nodes->elements[i]);
            }
        }
        else if (node->expr_node[0] != NULL) {
            process_expr(node->expr_node[0]);
        }
        printf("%s:\n", label1);
        if (node->expr_node[1] != NULL) {
            process_expr(node->expr_node[1]);
        }

        print_code("pop rax");
        print_code("cmp rax, 0");
        print_code("je %s", label3);

        process_stmt(node->stmt_node);

        printf("%s:\n", label2);
        if (node->expr_node[2] != NULL) {
            process_expr(node->expr_node[2]);
        }

        print_code("jmp %s", label1);
        printf("%s:\n", label3);

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

        const char* label = get_label();
        print_code("pop rdi");
        print_code("pop rax");
        print_code("push rax");
        print_code("cmp rax, rdi");
        print_code("jne %s", label);
        process_stmt(node->stmt_node);
        printf("%s:\n", label); 

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

static LocalVar* get_localvar(const char* str, int len) {
    for (int i = 0; i < localvar_list->size; ++i) {
        LocalVar* localvar = localvar_list->elements[i];
        if (strncmp(localvar->name, str, len) == 0) {
            return localvar;
        }
    }

    return NULL;
}

static GlobalVar* get_globalvar(const char* str, int len) {
    for (int i = 0; i < globalvar_list->size; ++i) {
        GlobalVar* gv = globalvar_list->elements[i];
        if (strncmp(gv->name, str, len) == 0) {
            return gv;
        }
    }

    return NULL;
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
        lv->offset   = current_offset;
        lv->type     = type;

        const InitDeclaratorNode* init_declarator_node = node->init_declarator_nodes->elements[i];
        const DeclaratorNode* declarator_node  = init_declarator_node->declarator_node;

        const PointerNode* pointer_node = declarator_node->pointer_node;
        if (pointer_node != NULL) {
            lv->type->ptr_count = pointer_node->count;
        }

        const DirectDeclaratorNode* direct_declarator_node = declarator_node->direct_declarator_node;
        const ConditionalExprNode*  conditional_expr_node  = direct_declarator_node->conditional_expr_node;
        if (conditional_expr_node == NULL) {
            lv->type->array_size = 0;
        } else {
            lv->type->array_size = get_array_size_from_constant_expr(conditional_expr_node);
        }

        const DirectDeclaratorNode* ident_node = get_identifier_direct_declarator(direct_declarator_node);
        lv->name_len = ident_node->identifier_len;
        lv->name     = malloc(sizeof(char) * lv->name_len);
        strncpy(lv->name, ident_node->identifier, lv->name_len);
        vector_push_back(localvar_list, lv);

        if (lv->type->array_size == 0) {
             current_offset += lv->type->type_size;
             current_offset = align_offset(current_offset);
        } else { 
            current_offset += (lv->type->array_size * 8);
        }

        if (init_declarator_node->initializer_node != NULL) {
            const InitializerNode* initializer_node = init_declarator_node->initializer_node;
            print_code("mov rax, rbp");
            print_code("sub rax, %d", lv->offset);
            print_code("push rax");

            if (initializer_node->assign_expr_node != NULL) {
                process_assign_expr(initializer_node->assign_expr_node);
            }

            print_code("pop rdi");
            print_code("pop rax");
            print_code("mov [rax], rdi");
        }
    }
}

static void process_compound_stmt(const CompoundStmtNode* node) {
    for (int i = 0; i < node->declaration_nodes->size; ++i) {
        process_declaration(node->declaration_nodes->elements[i]);
    }

    for (int i = 0; i < node->stmt_nodes->size; ++i) {
        process_stmt(node->stmt_nodes->elements[i]);
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

static int calc_localvar_size_in_compound_stmt(const CompoundStmtNode* node) {
    if (node == NULL) {
        return 0;
    }

    int size = 0;

    //
    // declaration
    // 
    for (int i = 0; i < node->declaration_nodes->size; ++i) {
        const DeclarationNode* declaration_node = (const DeclarationNode*)(node->declaration_nodes->elements[i]);

        int var_size = 0;
        const Vector* decl_specifier_nodes = declaration_node->decl_specifier_nodes;
        for (int j = 0; j < decl_specifier_nodes->size; ++j) {
            const DeclSpecifierNode* decl_specifier_node = (const DeclSpecifierNode*)(decl_specifier_nodes->elements[j]);
            if (decl_specifier_node->type_specifier_node != NULL) {
                const TypeSpecifierNode* type_specifier_node = decl_specifier_node->type_specifier_node;

                if (type_specifier_node->type_specifier == TYPE_STRUCT) {
                    const StructSpecifierNode* struct_specifier_node = type_specifier_node->struct_specifier_node;
                    const char* ident = struct_specifier_node->identifier;
                    const StructInfo* struct_info = strptrmap_get(struct_map, ident);
                    var_size = struct_info->field_info_map->size * 8;
                } 
                else if (type_specifier_node->type_specifier == TYPE_TYPEDEFNAME) {
                    const StructInfo* struct_info = strptrmap_get(struct_map, type_specifier_node->struct_name);
                    var_size = struct_info->field_info_map->size * 8;
                } else {
                    var_size = 8; // @todo
                }

                break;
            }
        }

        const Vector* init_declarator_nodes = declaration_node->init_declarator_nodes;
        for (int j = 0; j < init_declarator_nodes->size; ++j) {
            const InitDeclaratorNode* init_declarator_node = (const InitDeclaratorNode*)(init_declarator_nodes->elements[j]);
            const DeclaratorNode* declarator_node = (const DeclaratorNode*)(init_declarator_node->declarator_node);
            const DirectDeclaratorNode* direct_declarator_node = (const DirectDeclaratorNode*)(declarator_node->direct_declarator_node);

            if (direct_declarator_node->conditional_expr_node == NULL) {
                size += var_size;
            } else {
                const int array_size = get_array_size_from_constant_expr(direct_declarator_node->conditional_expr_node);
                size += (array_size * var_size);
            }
        }
    }

    //
    // statement
    // 
    for (int i = 0; i < node->stmt_nodes->size; ++i) {
        const StmtNode* stmt_node = node->stmt_nodes->elements[i];
        size += calc_localvar_size_in_compound_stmt(stmt_node->compound_stmt_node);

        if (stmt_node->itr_stmt_node == NULL) {
            continue;
        }

        const ItrStmtNode* itr_stmt_node = stmt_node->itr_stmt_node;
        for (int j = 0; j < itr_stmt_node->declaration_nodes->size; ++j) {
            const DeclarationNode* declaration_node = (const DeclarationNode*)(itr_stmt_node->declaration_nodes->elements[j]);
            const Vector* init_declarator_nodes = declaration_node->init_declarator_nodes;

            for (int k = 0; k < init_declarator_nodes->size; ++k) {
                const InitDeclaratorNode* init_declarator_node = (const InitDeclaratorNode*)(init_declarator_nodes->elements[k]);
                const DeclaratorNode* declarator_node = (const DeclaratorNode*)(init_declarator_node->declarator_node);
                const DirectDeclaratorNode* direct_declarator_node = (const DirectDeclaratorNode*)(declarator_node->direct_declarator_node);

                if (direct_declarator_node->conditional_expr_node == NULL) {
                    size += 8;
                } else {
                    const int array_size = get_array_size_from_constant_expr(direct_declarator_node->conditional_expr_node);
                    size += (array_size * 8);
                }
            }
        }
    }

    return size;
}

static Type* process_type_specifier_in_local(const TypeSpecifierNode* node) {
    Type* type = malloc(sizeof(Type));
    type->array_size  = 0;
    type->ptr_count   = 0;
    type->struct_info = NULL;

    switch (node->type_specifier) {
    case TYPE_VOID: { 
        type->base_type = VAR_VOID;
        type->type_size = 0;
        return type;
    }
    case TYPE_CHAR: { 
        type->base_type = VAR_CHAR;
        type->type_size = 1;
        return type;
    }
    case TYPE_SHORT: { 
        type->base_type = VAR_SHORT;
        type->type_size = 2;
        return type;
    }
    case TYPE_INT: {
        type->base_type = VAR_INT;
        type->type_size = 4;
        return type;
    }
    case TYPE_LONG: { 
        type->base_type = VAR_LONG;
        type->type_size = 8;
        return type;
    }
    case TYPE_FLOAT: {
        type->base_type = VAR_FLOAT;
        type->type_size = 4;
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
            exit(-1);
        }

        type->type_size = type->struct_info->size;
        return type;
    } 
    case TYPE_TYPEDEFNAME: { 
        type->base_type = VAR_STRUCT;   

        type->struct_info = strptrmap_get(struct_map, node->struct_name);
        if (type->struct_info == NULL) {
            error("Invalid sturct name=\"%s\"\n", node->struct_name);
            exit(-1);
        }

        type->type_size = type->struct_info->size;
        return type;
    } 
    default: { 
        return NULL;
    }
    }
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

    const DirectDeclaratorNode* direct_declarator_node = declarator_node->direct_declarator_node;
    LocalVar* lv = malloc(sizeof(LocalVar));
    lv->type     = type;
    lv->offset   = current_offset;
    lv->name_len = direct_declarator_node->identifier_len;
    lv->name     = malloc(sizeof(char) * lv->name_len);
    strncpy(lv->name, direct_declarator_node->identifier, lv->name_len);
    vector_push_back(localvar_list, lv);

    const PointerNode* pointer_node = declarator_node->pointer_node;
    if (pointer_node != NULL) {
        lv->type->ptr_count = pointer_node->count;
    }

    current_offset += 8;

    print_code("mov rax, rbp");
    print_code("sub rax, %d", lv->offset);
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
    localvar_list = create_vector();
    current_offset = 8;

    const DeclaratorNode* declarator_node = node->declarator_node;
    const DirectDeclaratorNode* direct_declarator_node = declarator_node->direct_declarator_node;
    printf("%s:\n", direct_declarator_node->direct_declarator_node->identifier); // @todo

    const int localvar_size = calc_localvar_size_in_compound_stmt(node->compound_stmt_node);
    const int arg_size      = calc_arg_size(node);

    // prologue
    print_code("push rbp");
    print_code("mov rbp, rsp");
    print_code("sub rsp, %d", (localvar_size + arg_size));

    process_func_declarator(declarator_node);
    process_compound_stmt(node->compound_stmt_node);

    // epilogue
    if (ret_label != NULL) {
        printf("%s:\n", ret_label);
    }
    print_code("mov rsp, rbp");
    print_code("pop rbp");
    print_code("ret");

    free(localvar_list);
    free(ret_label);
    ret_label = NULL;
    current_offset = 8;
}

static int get_int_constant(const InitializerNode* node) {
    return node->assign_expr_node
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
               ->postfix_expr_node
               ->primary_expr_node
               ->constant_node
               ->integer_constant;
}

static Type* process_type_specifier_in_global(const TypeSpecifierNode* node) {
    Type* type = malloc(sizeof(Type));
    type->array_size  = 0;
    type->ptr_count   = 0;
    type->struct_info = NULL;

    switch (node->type_specifier) {
    case TYPE_VOID: { 
        type->base_type = VAR_VOID;
        type->type_size = 0;
        return type;
    }
    case TYPE_CHAR: { 
        type->base_type = VAR_CHAR;
        type->type_size = 1;
        return type;
    }
    case TYPE_SHORT: { 
        type->base_type = VAR_SHORT;
        type->type_size = 2;
        return type;
    }
    case TYPE_INT: {
        type->base_type = VAR_INT;
        type->type_size = 4;
        return type;
    }
    case TYPE_LONG: { 
        type->base_type = VAR_LONG;
        type->type_size = 8;
        return type;
    }
    case TYPE_FLOAT: {
        type->base_type = VAR_FLOAT;
        type->type_size = 4;
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
        if (struct_specifier_node->identifier != NULL) {
            const char* struct_name = struct_specifier_node->identifier;
            const Vector* struct_declaration_nodes = struct_specifier_node->struct_declaration_nodes;
            //
            //  "struct" identifier { {struct-declaration}+ }
            //  
            if (struct_declaration_nodes->size != 0) {
                type->struct_info = malloc(sizeof(StructInfo));
                type->struct_info->field_info_map = create_strptrmap(1024);
                type->struct_info->size           = 0;

                int offset = 0;
                for (int i = 0; i < struct_declaration_nodes->size; ++i) {
                    const StructDeclarationNode* struct_declaration_node  = struct_declaration_nodes->elements[i];
                    const Vector* specifier_qualifier_nodes = struct_declaration_node->specifier_qualifier_nodes;

                    Type* field_type = NULL;
                    for (int j = 0; j < specifier_qualifier_nodes->size; ++j) {
                        const SpecifierQualifierNode* specifier_qualifier_node = specifier_qualifier_nodes->elements[j];
                        const TypeSpecifierNode*      type_specifier_node      = specifier_qualifier_node->type_specifier_node;
                        field_type = process_type_specifier_in_global(type_specifier_node);
                    }

                    const StructDeclaratorListNode* struct_declarator_list_node = struct_declaration_node->struct_declarator_list_node;
                    for (int j = 0; j < struct_declarator_list_node->declarator_nodes->size; ++j) {
                        const DeclaratorNode* node = struct_declarator_list_node->declarator_nodes->elements[j];

                        const PointerNode* pointer_node = node->pointer_node;
                        if (pointer_node != NULL) {
                            field_type->ptr_count = pointer_node->count;
                        } 

                        const char* field_name = node->direct_declarator_node->identifier;
 
                        FieldInfo* field_info = malloc(sizeof(FieldInfo));
                        field_info->type      = field_type;

                        field_info->offset = offset;
                        offset += 8;

                        strptrmap_put(type->struct_info->field_info_map, field_name, field_info);
                    }
                }
                type->struct_info->size = type->struct_info->field_info_map->size * 8;
                strptrmap_put(struct_map, struct_name, type->struct_info);
            }
            //
            // "struct" identifier
            //
            else {
                // @todo
            }
        }
        //
        // "struct" { {struct-declaration}+ }
        //
        else {
            // @todo
        }

        return type;
    }
    default: {
        return NULL;
    }
    }
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
        }

        const DirectDeclaratorNode* direct_declarator_node = declarator_node->direct_declarator_node;
        const ConditionalExprNode*     conditional_expr_node     = direct_declarator_node->conditional_expr_node;
        if (conditional_expr_node == NULL) {
            gv->type->array_size = 0;
        } else {
            gv->type->array_size = get_array_size_from_constant_expr(conditional_expr_node);
        }

        const DirectDeclaratorNode* ident_node = get_identifier_direct_declarator(direct_declarator_node);
        gv->name_len = ident_node->identifier_len;
        gv->name     = malloc(sizeof(char) * gv->name_len);
        strncpy(gv->name, ident_node->identifier, gv->name_len);
        vector_push_back(globalvar_list, gv);

        if (init_declarator_node->initializer_node != NULL) {
            const InitializerNode* initializer_node = init_declarator_node->initializer_node;
            const int int_constant = get_int_constant(initializer_node);

            printf(".data\n");
            printf("%s:\n", gv->name);
            print_code(".long %d", int_constant); 
        }
    }
}

static void process_enum_specifier(const EnumSpecifierNode* node) {
    const EnumeratorListNode* enumerator_list_node = node->enumerator_list_node;
    const Vector* identifiers = enumerator_list_node->identifiers;

    for (int i = 0; i < identifiers->size; ++i) {
        strintmap_put(enum_map, identifiers->elements[i], i + 1);
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
    print_header();
    print_global(node);

    // init
    label_index = 2;
    break_label_stack    = create_stack();
    continue_label_stack = create_stack();
    globalvar_list       = create_vector();
    struct_map           = create_strptrmap(1024);
    enum_map             = create_strintmap(1024);

    for (int i = 0; i < node->external_decl_nodes->size; ++i) {
        process_external_decl(node->external_decl_nodes->elements[i]);
    }
}
