#include "debug.h"

#include <time.h>
#include <stdarg.h>

//
// debug
//

void _debug_print_tmsp() {
    struct timespec ts; clock_gettime(CLOCK_REALTIME, &ts);

    struct tm tm;
    localtime_r(&(ts.tv_sec), &tm);

    char buf[30];
    snprintf(
        buf
      , 30
      , "%4d/%02d/%02d %02d:%02d:%02d"
      , tm.tm_year + 1900 , tm.tm_mon + 1
      , tm.tm_mday
      , tm.tm_hour
      , tm.tm_min
      , tm.tm_sec
    );

    fprintf(stdout, "[%s ", buf);
}

void _debug(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
}

//
// token
//

void dump_tokens(const TokenVec* vec) {
    for (int i = 0; i < vec->size; ++i) {
        fprintf(stdout, "index=%d,token=\"%s\"\n", i, decode_token_type(vec->tokens[i]->type));
    }
}

//
// parser
//

void dump_nodes(const TransUnitNode* node); static void dump_external_decl_node(const ExternalDeclNode* node, int indent);
static void dump_func_def_node(const FuncDefNode* node, int indent);
static void dump_decl_specifier_node(const DeclSpecifierNode* node, int indent);
static void dump_storage_class_specifier_node(const StorageClassSpecifierNode* node, int indent);
static void dump_type_specifier_node(const TypeSpecifierNode* node, int indent);
static void dump_struct_or_union_specifier_node(const StructOrUnionSpecifierNode* node, int indent);
static void dump_struct_declaration_node(const StructDeclarationNode* node, int indent);
static void dump_struct_declarator_list_node(const StructDeclaratorListNode* node, int indent);
static void dump_declarator_node(const DeclaratorNode* node, int indent);
static void dump_pointer_node(const PointerNode* node, int indent);
static void dump_type_qualifier_node(const TypeQualifierNode* node, int indent);
static void dump_specifier_qualifier_node(const SpecifierQualifierNode* node, int indent);
static void dump_direct_declarator_node(const DirectDeclaratorNode* node, int indent);
static void dump_conditional_expr_node(const ConditionalExprNode* node, int indent);
static void dump_logical_or_expr_node(const LogicalOrExprNode* node, int indent);
static void dump_logical_and_expr_node(const LogicalAndExprNode* node, int indent);
static void dump_inclusive_or_expr_node(const InclusiveOrExprNode* node, int indent);
static void dump_exclusive_or_expr_node(const ExclusiveOrExprNode* node, int indent);
static void dump_and_expr_node(const AndExprNode* node, int indent);
static void dump_equality_expr_node(const EqualityExprNode* node, int indent);
static void dump_relational_expr_node(const RelationalExprNode* node, int indent);
static void dump_shift_expr_node(const ShiftExprNode* node, int indent);
static void dump_additive_expr_node(const AdditiveExprNode* node, int indent);
static void dump_multiplicative_expr_node(const MultiPlicativeExprNode* node, int indent);
static void dump_cast_expr_node(const CastExprNode* node, int indent);
static void dump_unary_expr_node(const UnaryExprNode* node, int indent);
static void dump_postfix_expr_node(const PostfixExprNode* node, int indent);
static void dump_primary_expr_node(const PrimaryExprNode* node, int indent);
static void dump_constant_node(const ConstantNode* node, int indent);
static void dump_expr_node(const ExprNode* node, int indent);
static void dump_assign_expr_node(const AssignExprNode* node, int indent);
static void dump_type_name_node(const TypeNameNode* node, int indent);
static void dump_param_type_list_node(const ParamTypeListNode* node, int indent);
static void dump_param_list_node(const ParamListNode* node, int indent);
static void dump_param_declaration_node(const ParamDeclarationNode* node, int indent);
static void dump_abstract_declarator_node(const AbstractDeclaratorNode* node, int indent);
static void dump_direct_abstract_declarator_node(const DirectAbstractDeclaratorNode* node, int indent);
static void dump_enum_specifier_node(const EnumSpecifierNode* node, int indent);
static void dump_enumerator_list_node(const EnumeratorListNode* node, int indent);
static void dump_enumerator_node(const EnumeratorNode* node, int indent);
static void dump_typedef_name_node(const TypedefNameNode* node, int indent);
static void dump_declaration_node(const DeclarationNode* node, int indent);
static void dump_init_declarator_node(const InitDeclaratorNode* node, int indent);
static void dump_initializer_node(const InitializerNode* node, int indent);
static void dump_initializer_list_node(const InitializerListNode* node, int indent);
static void dump_compound_stmt_node(const CompoundStmtNode* node, int indent);
static void dump_stmt_node(const StmtNode* node, int indent);
static void dump_labeled_stmt_node(const LabeledStmtNode* node, int indent);
static void dump_expr_stmt_node(const ExprStmtNode* node, int indent);
static void dump_selection_stmt_node(const SelectionStmtNode* node, int indent);
static void dump_itr_stmt_node(const ItrStmtNode* node, int indent);
static void dump_jump_stmt_node(const JumpStmtNode* node, int indent);
static void print_name(const char* s, int indent);
static void printf_ident(int indent, const char* fmt, ...);

void dump_nodes(const TransUnitNode* node) {
    printf("TransUnitNode\n");

    for (int i = 0; i < node->external_decl_nodes->size; ++i) {
        const ExternalDeclNode* external_decl_node = (const ExternalDeclNode*)(node->external_decl_nodes->elements[i]);
        dump_external_decl_node(external_decl_node, 2);
    }
}

static void dump_external_decl_node(const ExternalDeclNode* node, int indent) {
    print_name("ExternalDeclNode", indent);

    if (node->func_def_node != NULL) { 
        dump_func_def_node(node->func_def_node, indent + 2);
    }

    if (node->declaration_node != NULL) {
        dump_declaration_node(node->declaration_node, indent + 2);
    }
}

static void dump_func_def_node(const FuncDefNode* node, int indent) {
    print_name("FuncDefNode", indent);

    for (int i = 0; i < node->decl_specifier_nodes->size; ++i) {
        const DeclSpecifierNode* n = (const DeclSpecifierNode*)(node->decl_specifier_nodes->elements[i]);
        dump_decl_specifier_node(n, indent + 2);
    }

    if (node->declarator_node != NULL) {
        dump_declarator_node(node->declarator_node, indent + 2);
    }

    if (node->compound_stmt_node != NULL) {
        dump_compound_stmt_node(node->compound_stmt_node, indent + 2);
    }
}

static void dump_decl_specifier_node(const DeclSpecifierNode* node, int indent) {
    print_name("DeclSpecifierNode", indent);

    if (node->storage_class_specifier_node != NULL) {
        dump_storage_class_specifier_node(node->storage_class_specifier_node, indent + 2);
    }

    if (node->type_specifier_node != NULL) {
        dump_type_specifier_node(node->type_specifier_node, indent + 2);
    }
}

static void dump_storage_class_specifier_node(const StorageClassSpecifierNode* node, int indent) {
    print_name("StorageClassSpecifierNode", indent);
    printf_ident(indent, "specifier=%s\n", decode_storage_class_specifier(node->storage_class_specifier));
}

static void dump_type_specifier_node(const TypeSpecifierNode* node, int indent) {
    print_name("TypeSpecifierNode", indent);
    printf_ident(indent, "Type=%s\n", decode_type_specifier(node->type_specifier));
    
    if (node->struct_or_union_specifier_node != NULL) {
        dump_struct_or_union_specifier_node(node->struct_or_union_specifier_node, indent + 2);
    }
    
    if (node->enum_specifier_node != NULL) {
        dump_enum_specifier_node(node->enum_specifier_node, indent + 2);
    }

    if (node->typedef_name_node != NULL) {
        dump_typedef_name_node(node->typedef_name_node, indent + 2);
    }
}

static void dump_struct_or_union_specifier_node(const StructOrUnionSpecifierNode* node, int indent) {
    print_name("StructOrUnionSpecifierNode", indent);
    printf_ident(indent, "StructOrUnion=%s\n", decode_struct_or_union(node->struct_or_union));
    printf_ident(indent, "Identifier=%s\n",    node->identifier);
    for (int i = 0; i < node->struct_declaration_nodes->size; ++i) {
        dump_struct_declaration_node(node->struct_declaration_nodes->elements[i], indent + 2);
    }
}

static void dump_struct_declaration_node(const StructDeclarationNode* node, int indent) {
    print_name("StructDeclarationNode", indent);
    for (int i = 0; i < node->specifier_qualifier_nodes->size; ++i) {
        dump_specifier_qualifier_node(node->specifier_qualifier_nodes->elements[i], indent + 2);
    }

    if (node->struct_declarator_list_node != NULL) {
        dump_struct_declarator_list_node(node->struct_declarator_list_node, indent + 2);
    }
}

static void dump_struct_declarator_list_node(const StructDeclaratorListNode* node, int indent) {
    print_name("StructDeclaratorListNode", indent);
}

static void dump_declarator_node(const DeclaratorNode* node, int indent) {
    print_name("DeclaratorNode", indent);

    if (node->pointer_node != NULL) {
        dump_pointer_node(node->pointer_node, indent + 2);
    }

    if (node->direct_declarator_node != NULL) {
        dump_direct_declarator_node(node->direct_declarator_node, indent + 2);
    } 
}

static void dump_pointer_node(const PointerNode* node, int indent) {
    print_name("PointerNode", indent);
    printf_ident(indent, "count=%d\n", node->count); 
}

static void dump_type_qualifier_node(const TypeQualifierNode* node, int indent) {
    print_name("TypeQualifierNode", indent);
    printf_ident(indent, "TypeQualfier=%s\n", decode_type_qualifier(node->type_qualifier));
}

static void dump_specifier_qualifier_node(const SpecifierQualifierNode* node, int indent) {
    print_name("SpecifierQualifierNode", indent);

    if (node->type_specifier_node != NULL) {
        dump_type_specifier_node(node->type_specifier_node, indent + 2);
    }

    if (node->type_qualifier_node != NULL) {
        dump_type_qualifier_node(node->type_qualifier_node, indent + 2);
    }
}

static void dump_direct_declarator_node(const DirectDeclaratorNode* node, int indent) {
    print_name("DirectDeclaratorNode", indent);
    printf_ident(indent, "Identifier=%s\n", node->identifier);

    if (node->declarator_node != NULL) {
        dump_declarator_node(node->declarator_node, indent + 2);
    }

    if (node->direct_declarator_node != NULL) {
        dump_direct_declarator_node(node->direct_declarator_node, indent + 2);
    }

    if (node->conditional_expr_node != NULL) {
        dump_conditional_expr_node(node->conditional_expr_node, indent + 2);
    }

    if (node->param_type_list_node != NULL) {
        dump_param_type_list_node(node->param_type_list_node, indent + 2);
    }

    for (int i = 0; i < node->identifier_list->size; ++i) { 
        printf_ident(indent, "identifier[%d]=%s\n", i, node->identifier_list->elements[i]);
    }
}

static void dump_conditional_expr_node(const ConditionalExprNode* node, int indent) {
    print_name("ConditionalExprNode", indent);

    if (node->logical_or_expr_node != NULL) {
        dump_logical_or_expr_node(node->logical_or_expr_node, indent + 2);
    }

    if (node->expr_node != NULL) {
        dump_expr_node(node->expr_node, indent + 2);
    }

    if (node->conditional_expr_node != NULL) {
        dump_conditional_expr_node(node->conditional_expr_node, indent + 2);
    }
}

static void dump_logical_or_expr_node(const LogicalOrExprNode* node, int indent) {
    print_name("LogicalOrExprNode", indent);

    if (node->logical_and_expr_node != NULL) {
        dump_logical_and_expr_node(node->logical_and_expr_node, indent + 2);
    }

    if (node->logical_or_expr_node != NULL) {
        dump_logical_or_expr_node(node->logical_or_expr_node, indent + 2);
    }
}

static void dump_logical_and_expr_node(const LogicalAndExprNode* node, int indent) {
    print_name("LogicalAndExprNode", indent);

    if (node->inclusive_or_expr_node != NULL) {
        dump_inclusive_or_expr_node(node->inclusive_or_expr_node, indent + 2);
    }

    if (node->logical_and_expr_node != NULL) {
        dump_logical_and_expr_node(node->logical_and_expr_node, indent + 2);
    }
}

static void dump_inclusive_or_expr_node(const InclusiveOrExprNode* node, int indent) {
    print_name("InclusiveOrExprNode", indent);

    if (node->exclusive_or_expr_node != NULL) {
        dump_exclusive_or_expr_node(node->exclusive_or_expr_node, indent + 2);
    }

    if (node->inclusive_or_expr_node != NULL) {
        dump_inclusive_or_expr_node(node->inclusive_or_expr_node, indent + 2);
    }
}

static void dump_exclusive_or_expr_node(const ExclusiveOrExprNode* node, int indent) {
    print_name("ExclusiveOrExprNode", indent);

    if (node->and_expr_node != NULL) {
        dump_and_expr_node(node->and_expr_node, indent + 2);
    }

    if (node->exclusive_or_expr_node != NULL) {
        dump_exclusive_or_expr_node(node->exclusive_or_expr_node, indent + 2);
    }

}

static void dump_and_expr_node(const AndExprNode* node, int indent) {
    print_name("AndExprNode", indent);

    if (node->equality_expr_node != NULL) {
        dump_equality_expr_node(node->equality_expr_node, indent + 2);
    }

    if (node->and_expr_node != NULL) {
        dump_and_expr_node(node->and_expr_node, indent + 2);
    }
}

static void dump_equality_expr_node(const EqualityExprNode* node, int indent) {
    print_name("EqualityExprNode", indent);
    printf_ident(indent, "CmpType=%s\n", decode_comparison_operator_type(node->cmp_type));

    if (node->relational_expr_node != NULL) {
        dump_relational_expr_node(node->relational_expr_node, indent + 2);
    }

    if (node->equality_expr_node != NULL) {
        dump_equality_expr_node(node->equality_expr_node, indent + 2);
    }
}

static void dump_relational_expr_node(const RelationalExprNode* node, int indent) {
    print_name("RelationalExprNode", indent);
    printf_ident(indent, "CmpType=%s\n", decode_comparison_operator_type(node->cmp_type));

    if (node->shift_expr_node != NULL) {
        dump_shift_expr_node(node->shift_expr_node, indent + 2);
    }

    if (node->relational_expr_node != NULL) {
        dump_relational_expr_node(node->relational_expr_node, indent + 2);
    } 
}

static void dump_shift_expr_node(const ShiftExprNode* node, int indent) {
    print_name("ShiftExprNode", indent);

    if (node->additive_expr_node != NULL) {
        dump_additive_expr_node(node->additive_expr_node, indent + 2);
    }

    if (node->shift_expr_node != NULL) {
        dump_shift_expr_node(node->shift_expr_node, indent + 2);
    }
}

static void dump_additive_expr_node(const AdditiveExprNode* node, int indent) {
    print_name("AdditiveExprNode", indent);
    printf_ident(indent, "OperatorType=%s\n", decode_operator_type(node->operator_type));

    if (node->multiplicative_expr_node != NULL) {
        dump_multiplicative_expr_node(node->multiplicative_expr_node, indent + 2);
    }

    if (node->additive_expr_node != NULL) {
        dump_additive_expr_node(node->additive_expr_node, indent + 2);
    }
}

static void dump_multiplicative_expr_node(const MultiPlicativeExprNode* node, int indent) {
    print_name("MultiPlicativeExprNode", indent);
    printf_ident(indent, "OperatorType=%s\n", decode_operator_type(node->operator_type));

    if (node->cast_expr_node != NULL) {
        dump_cast_expr_node(node->cast_expr_node, indent + 2);
    }

    if (node->multiplicative_expr_node != NULL) {
        dump_multiplicative_expr_node(node->multiplicative_expr_node, indent + 2);
    }
}
    
static void dump_cast_expr_node(const CastExprNode* node, int indent) {
    print_name("CastExprNode", indent);

    if (node->unary_expr_node != NULL) {
        dump_unary_expr_node(node->unary_expr_node, indent + 2);
    }

    if (node->cast_expr_node != NULL) {
        dump_cast_expr_node(node->cast_expr_node, indent + 2);
    }

    if (node->type_name_node != NULL) {
        dump_type_name_node(node->type_name_node, indent + 2);
    }
}

static void dump_unary_expr_node(const UnaryExprNode* node, int indent) {
    print_name("UnaryExprNode", indent);
    printf_ident(indent, "UnaryType=%s\n", decode_unary_type(node->type));
    printf_ident(indent, "OpType=%s\n", decode_operator_type(node->op_type));
    
    if (node->postfix_expr_node != NULL) {
        dump_postfix_expr_node(node->postfix_expr_node, indent + 2);
    }

    if (node->unary_expr_node != NULL) {
        dump_unary_expr_node(node->unary_expr_node, indent + 2);
    }

    if (node->cast_expr_node != NULL) {
        dump_cast_expr_node(node->cast_expr_node, indent + 2);
    }
}

static void dump_postfix_expr_node(const PostfixExprNode* node, int indent) {
    print_name("PostfixExprNode", indent);
    printf_ident(indent, "PostfixExprType=%s\n", decode_postfix_type(node->postfix_expr_type));
    printf_ident(indent, "Identifier=%s\n", node->identifier);
    
    if (node->primary_expr_node != NULL) {
        dump_primary_expr_node(node->primary_expr_node, indent + 2);
    }

    if (node->postfix_expr_node != NULL) {
        dump_postfix_expr_node(node->postfix_expr_node, indent + 2);
    }

    if (node->expr_node != NULL) {
        dump_expr_node(node->expr_node, indent + 2);
    }

    for (int i = 0; i < node->assign_expr_nodes->size; ++i) {
        dump_assign_expr_node(node->assign_expr_nodes->elements[i], indent + 2);
    }
}

static void dump_primary_expr_node(const PrimaryExprNode* node, int indent) {
    print_name("PrimaryExprNode", indent);
    printf_ident(indent, "Identifier=%s\n", node->identifier);

    if (node->constant_node != NULL) {
        dump_constant_node(node->constant_node, indent + 2);
    } 

    if (node->expr_node != NULL) {
        dump_expr_node(node->expr_node, indent + 2);
    }
}

static void dump_constant_node(const ConstantNode* node, int indent) {
    print_name("ConstantNode", indent);
    printf_ident(indent, "ConstType=%s\n", decode_const_type(node->const_type));

    switch (node->const_type) {
    case CONST_INT: {
        printf_ident(indent, "IntegerConstant=%d\n", node->integer_constant);
        break;
    }
    case CONST_STR: {
        printf_ident(indent, "CharacterConstant=%d\n", node->character_constant);
        break;
    }
    case CONST_FLOAT: {
        printf_ident(indent, "FloatingConstant=%d\n", node->floating_constant);
        break;
    }
    default: {
        break;
    }
    }
}

static void dump_expr_node(const ExprNode* node, int indent) {
    print_name("ExprNode", indent);

    if (node->assign_expr_node != NULL) {
        dump_assign_expr_node(node->assign_expr_node, indent + 2);
    } 

    if (node->expr_node != NULL) {
        dump_expr_node(node->expr_node, indent + 2);
    } 
}

static void dump_assign_expr_node(const AssignExprNode* node, int indent) {
    print_name("AssignExprNode", indent);

    if (node->conditional_expr_node != NULL) {
        dump_conditional_expr_node(node->conditional_expr_node, indent + 2);
    }

    if (node->unary_expr_node != NULL) {
        dump_unary_expr_node(node->unary_expr_node, indent + 2);
        print_name(fmt("Assignment-Operator=%d", node->assign_operator), indent + 2);
        dump_assign_expr_node(node->assign_expr_node, indent + 2);
    }
}

static void dump_type_name_node(const TypeNameNode* node, int indent) {
    print_name("TypeNameNode", indent);
}

static void dump_param_type_list_node(const ParamTypeListNode* node, int indent) {
    print_name("ParamTypeListNode", indent);
    printf_ident(indent, "%s\n", decode_param_list_type(node->type)); 

    if (node->param_list_node != NULL) {
        dump_param_list_node(node->param_list_node, indent + 2);
    }
}

static void dump_param_list_node(const ParamListNode* node, int indent) {
    print_name("ParamListNode", indent);

    if (node->param_declaration_node != NULL) {
        dump_param_declaration_node(node->param_declaration_node, indent + 2);
    }

    if (node->param_list_node != NULL) {
        dump_param_list_node(node->param_list_node, indent + 2);
    }
}

static void dump_param_declaration_node(const ParamDeclarationNode* node, int indent) {
    print_name("ParamDeclarationNode", indent);

    for (int i = 0; i < node->decl_spec_nodes->size; ++i) {
        dump_decl_specifier_node(node->decl_spec_nodes->elements[i], indent + 2);
    }

    if (node->declarator_node != NULL) {
        dump_declarator_node(node->declarator_node, indent + 2);
    }

    if (node->abstract_declarator_node != NULL) {
        dump_abstract_declarator_node(node->abstract_declarator_node, indent + 2);
    }
}

static void dump_abstract_declarator_node(const AbstractDeclaratorNode* node, int indent) {
    print_name("AbstractDeclaratorNode", indent);
}

static void dump_DirectAbstractDeclarator_node(const DirectAbstractDeclaratorNode* node, int indent) {
    print_name("DirectAbstractDeclaratorNode", indent);
}

static void dump_enum_specifier_node(const EnumSpecifierNode* node, int indent) {
    print_name("EnumSpecifierNode", indent);
}

static void dump_enumerator_list_node(const EnumeratorListNode* node, int indent) {
    print_name("EnumeratorListNode", indent);
}

static void dump_enumerator_node(const EnumeratorNode* node, int indent) {
    print_name("EnumeratorNode", indent);
}

static void dump_typedef_name_node(const TypedefNameNode* node, int indent) {
    print_name("TypedefNameNode", indent);
}

static void dump_declaration_node(const DeclarationNode* node, int indent) {
    print_name("DeclarationNode", indent);
    for (int i = 0; i < node->decl_specifier_nodes->size; ++i) {
        dump_decl_specifier_node(node->decl_specifier_nodes->elements[i], indent + 2);
    }

    for (int i = 0; i < node->init_declarator_nodes->size; ++i) {
        dump_init_declarator_node(node->init_declarator_nodes->elements[i], indent + 2);
    }
}

static void dump_init_declarator_node(const InitDeclaratorNode* node, int indent) {
    print_name("InitDeclaratorNode", indent);

    if (node->declarator_node != NULL) {
        dump_declarator_node(node->declarator_node, indent + 2);
    }

    if (node->initializer_node != NULL) {
        dump_initializer_node(node->initializer_node, indent + 2);
    }
}

static void dump_initializer_node(const InitializerNode* node, int indent) {
    print_name("InitializerNode", indent);

    if (node->assign_expr_node != NULL) {
        dump_assign_expr_node(node->assign_expr_node, indent + 2);
    }

    if (node->initializer_list_node != NULL) {
        dump_initializer_list_node(node->initializer_list_node, indent + 2);
    }
}

static void dump_initializer_list_node(const InitializerListNode* node, int indent) {
    print_name("InitializerListNode", indent);
}

static void dump_compound_stmt_node(const CompoundStmtNode* node, int indent) {
    print_name("CompoundStmtNode", indent);
    for (int i = 0; i < node->declaration_nodes->size; ++i) {
        const DeclarationNode* declaration_node = (const DeclarationNode*)(node->declaration_nodes->elements[i]);
        dump_declaration_node(declaration_node, indent + 2);
    }

    for (int i = 0; i < node->stmt_nodes->size; ++i) {
        const StmtNode* stmt_node = (const StmtNode*)(node->stmt_nodes->elements[i]);
        dump_stmt_node(stmt_node, indent + 2);
    }
}

static void dump_stmt_node(const StmtNode* node, int indent) {
    print_name("StmtNode", indent);

    if (node->expr_stmt_node != NULL) {
        dump_expr_stmt_node(node->expr_stmt_node, indent + 2);    
    }

    if (node->compound_stmt_node != NULL) {
        dump_compound_stmt_node(node->compound_stmt_node, indent + 2);
    }

    if (node->selection_stmt_node != NULL) {
        dump_selection_stmt_node(node->selection_stmt_node, indent + 2);
    }

    if (node->itr_stmt_node != NULL) {
        dump_itr_stmt_node(node->itr_stmt_node, indent + 2);
    }

    if (node->jump_stmt_node != NULL) {
        dump_jump_stmt_node(node->jump_stmt_node, indent + 2);
    }
}

static void dump_labeled_stmt_node(const LabeledStmtNode* node, int indent) {
    print_name("LabeledStmtNode", indent);
}

static void dump_expr_stmt_node(const ExprStmtNode* node, int indent) {
    print_name("ExprStmtNode", indent);

    if (node->expr_node != NULL) {
        dump_expr_node(node->expr_node, indent + 2);
    }
}

static void dump_selection_stmt_node(const SelectionStmtNode* node, int indent) {
    print_name("SelectionStmtNode", indent);
    printf_ident(indent, "SelectionType=%s\n", decode_selection_stmt_type(node->selection_type));

    if (node->expr_node != NULL) {
        dump_expr_node(node->expr_node, indent + 2);
    }

    if (node->stmt_node[0] != NULL) {
        dump_stmt_node(node->stmt_node[0], indent + 2);
    }

    if (node->stmt_node[1] != NULL) {
        dump_stmt_node(node->stmt_node[1], indent + 2);
    }
}

static void dump_itr_stmt_node(const ItrStmtNode* node, int indent) {
    print_name("ItrStmtNode", indent);
    printf_ident(indent, "ItrType=%s", decode_itr_type(node->itr_type));
    
    for (int i = 0; i < node->declaration_nodes->size; ++i) {
        dump_declaration_node(node->declaration_nodes->elements[i], indent + 2);
    }

    if (node->expr_node[0] != NULL) {
        dump_expr_node(node->expr_node[0], indent + 2);
    }

    if (node->expr_node[1] != NULL) {
        dump_expr_node(node->expr_node[0], indent + 2);
    }

    if (node->expr_node[2] != NULL) {
        dump_expr_node(node->expr_node[0], indent + 2);
    }

    if (node->stmt_node != NULL) {
        dump_stmt_node(node->stmt_node, indent + 2);
    }
}

static void dump_jump_stmt_node(const JumpStmtNode* node, int indent) {
    print_name("JumpStmtNode", indent);
    printf_ident(indent, "JumpType=%s\n", decode_jump_type(node->jump_type));
    printf_ident(indent, "Identifier=%s\n", node->identifier);

    if (node->expr_node != NULL) {
        dump_expr_node(node->expr_node, indent + 2);
    }
}

static void print_name(const char* s, int indent) {
    for (int i = 0; i < indent; ++i) {
        putchar(' ');
    }
    printf("%s\n", s);
}

static void printf_ident(int indent, const char* fmt, ...) {
    for (int i = 0; i < indent; ++i) {
        putchar(' ');
    }

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
}
