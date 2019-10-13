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
static void printf_indent(int indent, const char* fmt, ...);

void dump_nodes(const TransUnitNode* node) {
    printf("TransUnitNode\n");

    for (int i = 0; i < node->external_decl_nodes->size; ++i) {
        const ExternalDeclNode* external_decl_node = (const ExternalDeclNode*)(node->external_decl_nodes->elements[i]);
        dump_external_decl_node(external_decl_node, 2);
    }
}

static void dump_external_decl_node(const ExternalDeclNode* node, int indent) {
    printf_indent(indent, "ExternalDeclNode\n");

    if (node->func_def_node != NULL) {
        dump_func_def_node(node->func_def_node, indent + 2);
    }

    if (node->declaration_node != NULL) {
        dump_declaration_node(node->declaration_node, indent + 2);
    }
}

static void dump_func_def_node(const FuncDefNode* node, int indent) {
    printf_indent(indent, "FuncDefNode\n");

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
    printf_indent(indent, "DeclSpecifierNode\n");

    if (node->storage_class_specifier_node != NULL) {
        dump_storage_class_specifier_node(node->storage_class_specifier_node, indent + 2);
    }

    if (node->type_specifier_node != NULL) {
        dump_type_specifier_node(node->type_specifier_node, indent + 2);
    }
}

static void dump_storage_class_specifier_node(const StorageClassSpecifierNode* node, int indent) {
    printf_indent(
        indent, 
        "StorageClassSpecifierNode,Specifier=\"%s\"\n",
        decode_storage_class_specifier(node->storage_class_specifier)
    );
}

static void dump_type_specifier_node(const TypeSpecifierNode* node, int indent) {
    printf_indent(
        indent, 
        "TypeSpecifierNode,Type=\"%s\"\n",
        decode_type_specifier(node->type_specifier)
    );

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
    printf_indent(
        indent,
        "StructOrUnionSpecifierNode,StructOrUnion=\"%s\",Identifer=\"%s\"\n",
        decode_struct_or_union(node->struct_or_union),
        node->identifier
    );

    for (int i = 0; i < node->struct_declaration_nodes->size; ++i) {
        dump_struct_declaration_node(node->struct_declaration_nodes->elements[i], indent + 2);
    }
}

static void dump_struct_declaration_node(const StructDeclarationNode* node, int indent) {
    printf_indent(indent, "StructDeclarationNode\n");
    for (int i = 0; i < node->specifier_qualifier_nodes->size; ++i) {
        dump_specifier_qualifier_node(node->specifier_qualifier_nodes->elements[i], indent + 2);
    }

    if (node->struct_declarator_list_node != NULL) {
        dump_struct_declarator_list_node(node->struct_declarator_list_node, indent + 2);
    }
}

static void dump_struct_declarator_list_node(const StructDeclaratorListNode* node, int indent) {
    printf_indent(indent, "StructDeclaratorListNode\n");

    for (int i = 0; i < node->declarator_nodes->size; ++i) {
        dump_declarator_node(node->declarator_nodes->elements[i], indent + 2);
    }
}

static void dump_declarator_node(const DeclaratorNode* node, int indent) {
    printf_indent(indent, "DeclaratorNode\n");

    if (node->pointer_node != NULL) {
        dump_pointer_node(node->pointer_node, indent + 2);
    }

    if (node->direct_declarator_node != NULL) {
        dump_direct_declarator_node(node->direct_declarator_node, indent + 2);
    }
}

static void dump_pointer_node(const PointerNode* node, int indent) {
    printf_indent(
        indent, 
        "PointerNode,Count=%d\n",
        node->count
    );
}

static void dump_type_qualifier_node(const TypeQualifierNode* node, int indent) {
    printf_indent(
        indent, 
        "TypeQualifierNode,TypeQualifierNode=\"%s\"\n",
        decode_type_qualifier(node->type_qualifier)
    );
}

static void dump_specifier_qualifier_node(const SpecifierQualifierNode* node, int indent) {
    printf_indent(indent, "SpecifierQualifierNode\n");

    if (node->type_specifier_node != NULL) {
        dump_type_specifier_node(node->type_specifier_node, indent + 2);
    }

    if (node->type_qualifier_node != NULL) {
        dump_type_qualifier_node(node->type_qualifier_node, indent + 2);
    }
}

static void dump_direct_declarator_node(const DirectDeclaratorNode* node, int indent) {
    printf_indent(
        indent, 
        "DirectDeclaratorNode,Identifier=\"%s\"\n",
        node->identifier
    );

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
        printf_indent(indent, "identifier[%d]=%s\n", i, node->identifier_list->elements[i]);
    }
}

static void dump_conditional_expr_node(const ConditionalExprNode* node, int indent) {
    printf_indent(indent, "ConditionalExprNode\n");

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
    printf_indent(indent, "LogicalOrExprNode\n");

    if (node->logical_and_expr_node != NULL) {
        dump_logical_and_expr_node(node->logical_and_expr_node, indent + 2);
    }

    if (node->logical_or_expr_node != NULL) {
        dump_logical_or_expr_node(node->logical_or_expr_node, indent + 2);
    }
}

static void dump_logical_and_expr_node(const LogicalAndExprNode* node, int indent) {
    printf_indent(indent, "LogicalAndExprNode\n");

    if (node->inclusive_or_expr_node != NULL) {
        dump_inclusive_or_expr_node(node->inclusive_or_expr_node, indent + 2);
    }

    if (node->logical_and_expr_node != NULL) {
        dump_logical_and_expr_node(node->logical_and_expr_node, indent + 2);
    }
}

static void dump_inclusive_or_expr_node(const InclusiveOrExprNode* node, int indent) {
    printf_indent(indent, "InclusiveOrExprNode\n");

    if (node->exclusive_or_expr_node != NULL) {
        dump_exclusive_or_expr_node(node->exclusive_or_expr_node, indent + 2);
    }

    if (node->inclusive_or_expr_node != NULL) {
        dump_inclusive_or_expr_node(node->inclusive_or_expr_node, indent + 2);
    }
}

static void dump_exclusive_or_expr_node(const ExclusiveOrExprNode* node, int indent) {
    printf_indent(indent, "ExclusiveOrExprNode\n");

    if (node->and_expr_node != NULL) {
        dump_and_expr_node(node->and_expr_node, indent + 2);
    }

    if (node->exclusive_or_expr_node != NULL) {
        dump_exclusive_or_expr_node(node->exclusive_or_expr_node, indent + 2);
    }

}

static void dump_and_expr_node(const AndExprNode* node, int indent) {
    printf_indent(indent, "AndExprNode\n");

    if (node->equality_expr_node != NULL) {
        dump_equality_expr_node(node->equality_expr_node, indent + 2);
    }

    if (node->and_expr_node != NULL) {
        dump_and_expr_node(node->and_expr_node, indent + 2);
    }
}

static void dump_equality_expr_node(const EqualityExprNode* node, int indent) {
    printf_indent(
        indent, 
        "EqualityExprNode,CmpType=\"%s\"\n",
        decode_comparison_operator_type(node->cmp_type)
    );

    if (node->relational_expr_node != NULL) {
        dump_relational_expr_node(node->relational_expr_node, indent + 2);
    }

    if (node->equality_expr_node != NULL) {
        dump_equality_expr_node(node->equality_expr_node, indent + 2);
    }
}

static void dump_relational_expr_node(const RelationalExprNode* node, int indent) {
    printf_indent(
        indent,
        "RelationalExprNode,CmpType=\"%s\"\n",
        decode_comparison_operator_type(node->cmp_type)
    );

    if (node->shift_expr_node != NULL) {
        dump_shift_expr_node(node->shift_expr_node, indent + 2);
    }

    if (node->relational_expr_node != NULL) {
        dump_relational_expr_node(node->relational_expr_node, indent + 2);
    }
}

static void dump_shift_expr_node(const ShiftExprNode* node, int indent) {
    printf_indent(indent, "ShiftExprNode\n");

    if (node->additive_expr_node != NULL) {
        dump_additive_expr_node(node->additive_expr_node, indent + 2);
    }

    if (node->shift_expr_node != NULL) {
        dump_shift_expr_node(node->shift_expr_node, indent + 2);
    }
}

static void dump_additive_expr_node(const AdditiveExprNode* node, int indent) {
    printf_indent(
        indent, 
        "AdditiveExprNode,OperatorType=\"%s\"\n", 
        decode_operator_type(node->operator_type)
    );

    if (node->multiplicative_expr_node != NULL) {
        dump_multiplicative_expr_node(node->multiplicative_expr_node, indent + 2);
    }

    if (node->additive_expr_node != NULL) {
        dump_additive_expr_node(node->additive_expr_node, indent + 2);
    }
}

static void dump_multiplicative_expr_node(const MultiPlicativeExprNode* node, int indent) {
    printf_indent(
        indent,
        "MultiPlicativeExprNode,OperatorType=\"%s\"\n",
        decode_operator_type(node->operator_type)
    );

    if (node->cast_expr_node != NULL) {
        dump_cast_expr_node(node->cast_expr_node, indent + 2);
    }

    if (node->multiplicative_expr_node != NULL) {
        dump_multiplicative_expr_node(node->multiplicative_expr_node, indent + 2);
    }
}

static void dump_cast_expr_node(const CastExprNode* node, int indent) {
    printf_indent(indent, "CastExprNode\n");

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
    printf_indent(
        indent, 
        "UnaryExprNode,UnaryType=\"%s\",OperatorType=\"%s\"\n", 
        decode_unary_type(node->type),
        decode_operator_type(node->op_type)
    );

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
    printf_indent(
        indent, 
        "PostfixExprNode,PostfixExprType=\"%s\",Identifier=\"%s\"\n",
        decode_postfix_type(node->postfix_expr_type),
        node->identifier
    );

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
    printf_indent(
        indent, 
        "PrimaryExprNode,Identifier=\"%s\"\n",
        node->identifier
    );

    if (node->constant_node != NULL) {
        dump_constant_node(node->constant_node, indent + 2);
    }

    if (node->expr_node != NULL) {
        dump_expr_node(node->expr_node, indent + 2);
    }
}

static void dump_constant_node(const ConstantNode* node, int indent) {
    printf_indent(
        indent, 
        "ConstantNode,ConstType=\"%s\"",
        decode_const_type(node->const_type)
    );

    switch (node->const_type) {
    case CONST_INT: {
        printf("IntegerConstant=%d\n", node->integer_constant);
        break;
    }
    case CONST_STR: {
        printf("CharacterConstant=\"%s\"\n", node->character_constant);
        break;
    }
    case CONST_FLOAT: {
        printf("FloatingConstant=%f\n", node->floating_constant);
        break;
    }
    default: {
        printf("\n");
        break;
    }
    }
}

static void dump_expr_node(const ExprNode* node, int indent) {
    printf_indent(indent, "ExprNode\n");

    if (node->assign_expr_node != NULL) {
        dump_assign_expr_node(node->assign_expr_node, indent + 2);
    }

    if (node->expr_node != NULL) {
        dump_expr_node(node->expr_node, indent + 2);
    }
}

static void dump_assign_expr_node(const AssignExprNode* node, int indent) {
    printf_indent(
        indent, 
        "AssignExprNode,Operator=\"%s\"\n",
        decode_operator_type(node->assign_operator)
    );

    if (node->conditional_expr_node != NULL) {
        dump_conditional_expr_node(node->conditional_expr_node, indent + 2);
    }

    if (node->unary_expr_node != NULL) {
        dump_unary_expr_node(node->unary_expr_node, indent + 2);
        dump_assign_expr_node(node->assign_expr_node, indent + 2);
    }
}

static void dump_type_name_node(const TypeNameNode* node, int indent) {
    printf_indent(indent, "TypeNameNode");
}

static void dump_param_type_list_node(const ParamTypeListNode* node, int indent) {
    printf_indent(
        indent,
        "ParamTypeListNode,ParamType=\"%s\"\n",
        decode_param_list_type(node->type)
    );

    if (node->param_list_node != NULL) {
        dump_param_list_node(node->param_list_node, indent + 2);
    }
}

static void dump_param_list_node(const ParamListNode* node, int indent) {
    printf_indent(indent, "ParamListNode\n");

    if (node->param_declaration_node != NULL) {
        dump_param_declaration_node(node->param_declaration_node, indent + 2);
    }

    if (node->param_list_node != NULL) {
        dump_param_list_node(node->param_list_node, indent + 2);
    }
}

static void dump_param_declaration_node(const ParamDeclarationNode* node, int indent) {
    printf_indent(indent, "ParamDeclarationNode\n");

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
    printf_indent(indent, "AbstractDeclaratorNode\n");
}

static void dump_DirectAbstractDeclarator_node(const DirectAbstractDeclaratorNode* node, int indent) {
    printf_indent(indent, "DirectAbstractDeclaratorNode\n");
}

static void dump_enum_specifier_node(const EnumSpecifierNode* node, int indent) {
    printf_indent(indent, "EnumSpecifierNode\n");
}

static void dump_enumerator_list_node(const EnumeratorListNode* node, int indent) {
    printf_indent(indent, "EnumeratorListNode\n");
}

static void dump_enumerator_node(const EnumeratorNode* node, int indent) {
    printf_indent(indent, "EnumeratorNode\n");
}

static void dump_typedef_name_node(const TypedefNameNode* node, int indent) {
    printf_indent(indent, "TypedefNameNode\n");
}

static void dump_declaration_node(const DeclarationNode* node, int indent) {
    printf_indent(indent, "DeclarationNode\n");
    for (int i = 0; i < node->decl_specifier_nodes->size; ++i) {
        dump_decl_specifier_node(node->decl_specifier_nodes->elements[i], indent + 2);
    }

    for (int i = 0; i < node->init_declarator_nodes->size; ++i) {
        dump_init_declarator_node(node->init_declarator_nodes->elements[i], indent + 2);
    }
}

static void dump_init_declarator_node(const InitDeclaratorNode* node, int indent) {
    printf_indent(indent, "InitDeclaratorNode\n");

    if (node->declarator_node != NULL) {
        dump_declarator_node(node->declarator_node, indent + 2);
    }

    if (node->initializer_node != NULL) {
        dump_initializer_node(node->initializer_node, indent + 2);
    }
}

static void dump_initializer_node(const InitializerNode* node, int indent) {
    printf_indent(indent, "InitializerNode\n");

    if (node->assign_expr_node != NULL) {
        dump_assign_expr_node(node->assign_expr_node, indent + 2);
    }

    if (node->initializer_list_node != NULL) {
        dump_initializer_list_node(node->initializer_list_node, indent + 2);
    }
}

static void dump_initializer_list_node(const InitializerListNode* node, int indent) {
    printf_indent(indent, "InitializerListNode\n");
}

static void dump_compound_stmt_node(const CompoundStmtNode* node, int indent) {
    printf_indent(indent, "CompoundStmtNode\n");
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
    printf_indent(indent, "StmtNode\n");

    if (node->labeled_stmt_node != NULL) {
        dump_labeled_stmt_node(node->labeled_stmt_node, indent + 2);
    }

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
    printf_indent(
        indent,
        "LabeledStmtNode,LabeledStmtType=\"%s\"\n",
        decode_labeled_stmt_type(node->labeled_stmt_type)
    );

    if (node->conditional_expr_node != NULL) {
        dump_conditional_expr_node(node->conditional_expr_node, indent + 2 );
    }

    if (node->stmt_node != NULL) {
        dump_stmt_node(node->stmt_node, indent + 2);
    }
}

static void dump_expr_stmt_node(const ExprStmtNode* node, int indent) {
    printf_indent(indent, "ExprStmtNode\n");

    if (node->expr_node != NULL) {
        dump_expr_node(node->expr_node, indent + 2);
    }
}

static void dump_selection_stmt_node(const SelectionStmtNode* node, int indent) {
    printf_indent(
        indent, 
        "SelectionStmtNode,SelectionType=\"%s\"\n",
        decode_selection_stmt_type(node->selection_type)
    );

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
    printf_indent(
        indent, 
        "ItrStmtNode,ItrType=\"%s\"", 
        decode_itr_type(node->itr_type)
    );

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
    printf_indent(
        indent, 
        "JumpStmtNode,JumpType=\"%s\",Identifier=\"%s\"\n",
        decode_jump_type(node->jump_type),
        node->identifier
    );

    if (node->expr_node != NULL) {
        dump_expr_node(node->expr_node, indent + 2);
    }
}

static void printf_indent(int indent, const char* fmt, ...) {
    for (int i = 0; i < indent; ++i) {
        putchar(' ');
    }

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
}
