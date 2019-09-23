#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"
#include "util.h"

enum TypeSpecifier {
    TYPE_NONE,
    TYPE_VOID,
    TYPE_CHAR,
    TYPE_SHORT,
    TYPE_INT,
    TYPE_LONG,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_SIGNED,
    TYPE_UNSIGNED,
};

enum TypeQualifier {
    TQ_CONST,
    TQ_VOLATILE,
};

enum StorageClassSpecifier {
    SC_AUTO,
    SC_REGISTER,
    SC_STATIC,
    SC_EXTERN,
    SC_TYPEDEF,
};

enum StructOrUnion {
    SU_STRUCT,
    SU_UNION,
};

enum OperatorType {
    OP_NONE,   // None
    OP_ASSIGN, // =
    OP_MUL_EQ, // *= 
    OP_DIV_EQ, // /=
    OP_MOD_EQ, // %=
    OP_ADD_EQ, // +=
    OP_SUB_EQ, // -=
    OP_AND_EQ, // &=
    OP_XOR_EQ, // ^=
    OP_OR_EQ,  // |=
    OP_AND,    // &
    OP_ADD,    // +
    OP_SUB,    // -
    OP_MUL,    // *
    OP_DIV,    // /
    OP_MOD,    // %
    OP_TILDE,  // ~
    OP_EXCLA,  // !
};

enum ConstType {
    CONST_INT,
    CONST_STR,
    CONST_FLOAT,
};

enum JumpType {
    JMP_GOTO,
    JMP_CONTINUE,
    JMP_BREAK,
    JMP_RETURN,
};

typedef struct TransUnitNode TransUnitNode;
typedef struct ExternalDeclNode ExternalDeclNode;
typedef struct FuncDefNode FuncDefNode;
typedef struct DeclSpecifierNode DeclSpecifierNode;
typedef struct StorageClassSpecifierNode StorageClassSpecifierNode;
typedef struct TypeSpecifierNode TypeSpecifierNode;
typedef struct StructOrUnionSpecifierNode StructOrUnionSpecifierNode;
typedef struct StructOrUnionNode StructOrUnionNode;
typedef struct StructDeclNode StructDeclNode;
typedef struct DeclaratorNode DeclaratorNode;
typedef struct PointerNode PointerNode;
typedef struct TypeQualifierNode TypeQualifierNode;
typedef struct DirectDeclaratorNode DirectDeclaratorNode;
typedef struct ConstantExprNode ConstantExprNode;
typedef struct ConditionalExprNode ConditionalExprNode;
typedef struct LogicalOrExprNode LogicalOrExprNode;
typedef struct LogicalAndExprNode LogicalAndExprNode;
typedef struct InclusiveOrExprNode InclusiveOrExprNode;
typedef struct ExclusiveOrExprNode ExclusiveOrExprNode;
typedef struct AndExprNode AndExprNode;
typedef struct EqualityExprNode EqualityExprNode;
typedef struct RelationalExprNode RelationalExprNode;
typedef struct ShiftExprNode ShiftExprNode;
typedef struct AdditiveExprNode AdditiveExprNode;
typedef struct MultiPlicativeExprNode MultiPlicativeExprNode;
typedef struct CastExprNode CastExprNode;
typedef struct UnaryExprNode UnaryExprNode;
typedef struct PostfixExprNode PostfixExprNode;
typedef struct PrimaryExprNode PrimaryExprNode;
typedef struct ConstantNode ConstantNode;
typedef struct ExprNode ExprNode;
typedef struct AssignExprNode AssignExprNode;
// AssignOperator 
// UnaryOperator
typedef struct TypeNameNode TypeNameNode;
typedef struct ParamTypeListNode ParamTypeListNode;
typedef struct ParamListNode ParamListNode;
typedef struct ParamDeclarationNode ParamDeclarationNode;
typedef struct AbstractDeclaratorNode AbstractDeclaratorNode;
typedef struct DirectAbstractDeclaratorNode DirectAbstractDeclaratorNode;
typedef struct EnumSpecifierNode EnumSpecifierNode;
typedef struct EnumeratorListNode EnumeratorListNode;
typedef struct EnumeratorNode EnumeratorNode;
typedef struct TypedefNameNode TypedefNameNode;
typedef struct DeclarationNode DeclarationNode;
typedef struct InitDeclaratorNode InitDeclaratorNode;
typedef struct InitializerNode InitializerNode;
typedef struct InitializerListNode InitializerListNode;
typedef struct CompoundStmtNode CompoundStmtNode;
typedef struct StmtNode StmtNode;
typedef struct LabeledStmtNode LabeledStmtNode;
typedef struct ExprStmtNode ExprStmtNode;
typedef struct SelectStmtNode SelectStmtNode;
typedef struct IterStmtNode IterStmtNode;
typedef struct JumpStmtNode JumpStmtNode;

struct TransUnitNode {
    PtrVector* external_decl_nodes;
};

struct ExternalDeclNode {
    FuncDefNode*     func_def_node;
    DeclarationNode* declaration_node;
};

struct FuncDefNode {
    int               type_specifier;
    char*             identifier;
    CompoundStmtNode* compound_stmt_node;
};

struct DeclSpecifierNode {
    StorageClassSpecifierNode* storage_class_specifier_node; 
    TypeSpecifierNode*         type_specifier_node; 
    TypeQualifierNode*         type_qualifier_node; 
};

struct StorageClassSpecifierNode {
    int storage_class_specifier;
};

struct TypeSpecifierNode {
    int                type_specifier;
    StructOrUnionNode* struct_or_union_specifier_node;
    EnumSpecifierNode* enum_specifier_node;
    TypedefNameNode*   typedef_name_node;
};

struct StructOrUnionSpecifierNode {
    int struct_or_union;
};

struct StructOrUnionNode {
};

struct StructDeclNode {
};

struct DeclaratorNode {
    PointerNode*          pointer_node;
    DirectDeclaratorNode* direct_declarator_node;     
};

struct PointerNode {
};

struct TypeQualifierNode {
    int type_qualifier;
};

struct DirectDeclaratorNode {
    char*                 identifier;
    int                   identifier_len;
    DeclaratorNode*       declarator_node;
    DirectDeclaratorNode* direct_declarator_node;  
    ConstantExprNode*     constant_expr_node;
    ParamTypeListNode*    param_type_list_node;
    PtrVector*            identifier_list;
};

struct ConstantExprNode {
};

struct ConditionalExprNode {
    LogicalOrExprNode*   logical_or_expr_node;
    LogicalAndExprNode*  logical_and_expr_node;
    ConditionalExprNode* conditional_expr_node;
};

struct LogicalOrExprNode {
    LogicalAndExprNode* logical_and_expr_node;
    LogicalOrExprNode*  logical_or_expr_node;
};

struct LogicalAndExprNode {
    InclusiveOrExprNode* inclusive_or_expr_node;
    LogicalAndExprNode*  logical_and_expr_node;
};

struct InclusiveOrExprNode {
    ExclusiveOrExprNode* exclusive_or_expr_node;
    InclusiveOrExprNode* inclusive_or_expr_node;
};

struct ExclusiveOrExprNode {
    AndExprNode*         and_expr_node;
    ExclusiveOrExprNode* exclusive_or_expr_node;
};

struct AndExprNode {
    EqualityExprNode* equality_expr_node;
    AndExprNode*      and_expr_node;
};

struct EqualityExprNode {
    RelationalExprNode* relational_expr_node;
    EqualityExprNode*   equality_expr_node;
};

struct RelationalExprNode {
    ShiftExprNode*      shift_expr_node;
    RelationalExprNode* relational_expr_node;
};

struct ShiftExprNode {
    AdditiveExprNode* additive_expr_node;
    ShiftExprNode*    shift_expr_node;
};

struct AdditiveExprNode {
    int                     node_type;
    int                     operator_type;
    MultiPlicativeExprNode* multiplicative_expr_node;
    AdditiveExprNode*       additive_expr_node;
};

struct MultiPlicativeExprNode {
    int                     operator_type;
    CastExprNode*           cast_expr_node;
    MultiPlicativeExprNode* multiplicative_expr_node;
};

struct CastExprNode {
    UnaryExprNode* unary_expr_node;
    CastExprNode*  cast_expr_node;
    int            type_name;
};  

struct UnaryExprNode {
    PostfixExprNode* postfix_expr_node;
    UnaryExprNode*   unary_expr_node;
    CastExprNode*    cast_expr_node;
    int              type_name;
}; 

struct PostfixExprNode {
    PrimaryExprNode* primary_expr_node;
    PostfixExprNode* postfix_expr_node;
    ExprNode*        expr_node;
    AssignExprNode*  assignment_expr_node;
    char*            identifier;
    int              identifier_len;
};

struct PrimaryExprNode {
    ConstantNode* constant_node;
    ExprNode*     expr_node;
    char*         string;
    char*         identifier;
    int           identifier_len;
};

struct ConstantNode {
    int   const_type;
    int   integer_constant;
    int   enumeration_constant;
    char* character_constant;  
    float floating_constant;
}; 

struct ExprNode {
    AssignExprNode* assign_expr_node;
    ExprNode*       expr_node;
};

struct AssignExprNode {
    ConditionalExprNode* conditional_expr_node;
    UnaryExprNode*       unary_expr_node;
    AssignExprNode*      assign_expr_node;
    int                  assign_operator;
};

struct TypeNameNode {
};

struct ParamTypeListNode {
};

struct ParamListNode {
};

struct ParamDeclarationNode {
};

struct AbstractDeclaratorNode {
};

struct DirectAbstractDeclaratorNode {
};

struct EnumSpecifierNode {
};

struct EnumeratorListNode {
};

struct EnumeratorNode {
};

struct TypedefNameNode {
};

struct DeclarationNode {
    PtrVector* decl_specifier_nodes;
    PtrVector* init_declarator_nodes;
};

struct InitDeclaratorNode {
    DeclaratorNode*  declarator_node;
    InitializerNode* initializer_node;
};

struct InitializerNode {
    AssignExprNode*      assign_expr_node;
    InitializerListNode* initializer_list_node;
};

struct InitializerListNode {
};

struct CompoundStmtNode {
    PtrVector* declaration_nodes;
    PtrVector* stmt_nodes;
};

struct StmtNode {
    ExprStmtNode* expr_stmt_node; 
    JumpStmtNode* jump_stmt_node;
};

struct LabeledStmtNode {
};

struct ExprStmtNode {
    ExprNode* expr_node;  
};

struct SelectStmtNode {
};

struct IterStmtNode {
};

struct JumpStmtNode {
    int       jump_type;
    char*     identifier;
    ExprNode* expr_node;
};

//
// parse
//

TransUnitNode* parse(TokenVec* vec);

#endif
