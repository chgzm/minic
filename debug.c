#include "debug.h"

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

void dump_nodes(const TransUnitNode* node);
#if 0
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
static void dump_SelectionStmtNode(const SelectionStmtNode* node, int indent);
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

static void dump_SelectonStmtNode(const SelectionStmtNode* node, int indent) {
    print_name("SelectionStmtNode", indent);
}

static void dump_IterStmtNode(const IterStmtNode* node, int indent) {
    print_name("IterStmtNode", indent);
}

static void dump_JumpStmtNode(const JumpStmtNode* node, int indent) {
    print_name("JumpStmtNode", indent);
}

void print_name(const char* s, int indent) {
    for (int i = 0; i < indent; ++i) {
        putchar(' ');
    }
    printf("%s\n", s);
}

#endif
