#ifndef PARSER_H
#define PARSER_H 

#include "tokenizer.h"
#include "util.h"

enum TypeSpecifier {
    TYPE_NONE,
    TYPE_VOID,
    TYPE_CHAR,
    TYPE_INT,
    TYPE_DOUBLE,
    TYPE_STRUCT,
    TYPE_ENUM,
    TYPE_TYPEDEFNAME
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

enum ComparisonOperatorType {
    CMP_NONE,  // None
    CMP_LT,    // <
    CMP_GT,    // >
    CMP_EQ,    // ==
    CMP_NE,    // !=
    CMP_LE,    // <=
    CMP_GE,    // >=
};

enum ConstType {
    CONST_INT,
    CONST_BYTE,
    CONST_STR,
    CONST_FLOAT,
};

enum JumpType {
    JMP_CONTINUE,
    JMP_BREAK,
    JMP_RETURN,
};

enum UnaryType {
    UN_NONE,         // none
    UN_INC,          // ++
    UN_DEC,          // --
    UN_OP,           // unary-operator
    UN_SIZEOF_IDENT, // sizeof ( identifier ) 
    UN_SIZEOF_TYPE,  // sizeof ( type-name )
};

enum SizeofType {
    SIZEOFTYPE_CHAR,
    SIZEOFTYPE_INT,
    SIZEOFTYPE_DOUBLE,
    SIZEOFTYPE_IDENT,
};

enum PostfixType {
    PS_PRIMARY, // primary-expreesion
    PS_LSQUARE, // postfix-expression [ <expression> ]
    PS_LPAREN,  // postfix-expression ( {assignment-expression}* )
    PS_DOT,     // postfix-expression . identifier
    PS_ARROW,   // postfix-expression -> identifier
    PS_INC,     // postfix-expression ++
    PS_DEC,     // postfix-expression --
};

enum SelectionStmtType {
    SELECT_IF,
    SELECT_IF_ELSE,
    SELECT_SWITCH,
};

enum IterationType {
    ITR_WHILE,
    ITR_FOR,
};

enum LabeledStmtType {
    LABELED_CASE,
    LABELED_DEFAULT,
};

typedef struct TransUnitNode TransUnitNode;
typedef struct ExternalDeclNode ExternalDeclNode;
typedef struct FuncDefNode FuncDefNode;
typedef struct DeclSpecifierNode DeclSpecifierNode;
typedef struct TypeSpecifierNode TypeSpecifierNode;
typedef struct StructSpecifierNode StructSpecifierNode;
typedef struct StructDeclarationNode StructDeclarationNode;
typedef struct StructDeclaratorListNode StructDeclaratorListNode;
typedef struct SpecifierQualifierNode SpecifierQualifierNode;
typedef struct DeclaratorNode DeclaratorNode;
typedef struct PointerNode PointerNode;
typedef struct DirectDeclaratorNode DirectDeclaratorNode;
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
typedef struct TypeNameNode TypeNameNode;
typedef struct ParamTypeListNode ParamTypeListNode;
typedef struct ParamListNode ParamListNode;
typedef struct ParamDeclarationNode ParamDeclarationNode;
typedef struct AbstractDeclaratorNode AbstractDeclaratorNode;
typedef struct DirectAbstractDeclaratorNode DirectAbstractDeclaratorNode;
typedef struct EnumSpecifierNode EnumSpecifierNode;
typedef struct EnumeratorListNode EnumeratorListNode;
typedef struct DeclarationNode DeclarationNode;
typedef struct InitDeclaratorNode InitDeclaratorNode;
typedef struct InitializerNode InitializerNode;
typedef struct InitializerListNode InitializerListNode;
typedef struct CompoundStmtNode CompoundStmtNode;
typedef struct BlockItemNode BlockItemNode;
typedef struct StmtNode StmtNode;
typedef struct LabeledStmtNode LabeledStmtNode;
typedef struct ExprStmtNode ExprStmtNode;
typedef struct SelectionStmtNode SelectionStmtNode;
typedef struct ItrStmtNode ItrStmtNode;
typedef struct JumpStmtNode JumpStmtNode;

struct TransUnitNode {
    Vector* external_decl_nodes;
};

struct ExternalDeclNode {
    FuncDefNode*       func_def_node;
    DeclarationNode*   declaration_node;
    EnumSpecifierNode* enum_specifier_node;
};

struct FuncDefNode {
    Vector*           decl_specifier_nodes;
    DeclaratorNode*   declarator_node;
    CompoundStmtNode* compound_stmt_node;
};

struct DeclSpecifierNode {
    TypeSpecifierNode* type_specifier_node; 
    bool               is_const;
    bool               is_static;    
};

struct TypeSpecifierNode {
    int                  type_specifier;
    StructSpecifierNode* struct_specifier_node;
    char*                struct_name;
};

struct StructSpecifierNode {
    int        identifier_len;
    char*      identifier;
    Vector*    struct_declaration_nodes; 
};

struct StructDeclarationNode {
    Vector*                   specifier_qualifier_nodes;
    StructDeclaratorListNode* struct_declarator_list_node;    
};

struct SpecifierQualifierNode {
    TypeSpecifierNode* type_specifier_node;
    bool               is_const;
};

struct StructDeclaratorListNode {
    Vector* declarator_nodes;
};

struct DeclaratorNode {
    PointerNode*          pointer_node;
    DirectDeclaratorNode* direct_declarator_node;     
};

struct PointerNode {
    int count;
};

struct DirectDeclaratorNode {
    char*                 identifier;
    int                   identifier_len;
    DeclaratorNode*       declarator_node;
    DirectDeclaratorNode* direct_declarator_node;  
    ConditionalExprNode*  conditional_expr_node;
    ParamTypeListNode*    param_type_list_node;
    Vector*               identifier_list;
};

struct ConditionalExprNode {
    LogicalOrExprNode*   logical_or_expr_node;
    ExprNode*            expr_node;
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
    int                 cmp_type;
    RelationalExprNode* relational_expr_node;
    EqualityExprNode*   equality_expr_node;
};

struct RelationalExprNode {
    int                 cmp_type;
    ShiftExprNode*      shift_expr_node;
    RelationalExprNode* relational_expr_node;
};

struct ShiftExprNode {
    AdditiveExprNode* additive_expr_node;
    ShiftExprNode*    shift_expr_node;
};

struct AdditiveExprNode {
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
    TypeNameNode*  type_name_node;
};  

struct UnaryExprNode {
    PostfixExprNode* postfix_expr_node;
    UnaryExprNode*   unary_expr_node;
    CastExprNode*    cast_expr_node;
    TypeNameNode*    type_name_node;
    int              type;
    int              op_type;
    int              sizeof_name_len;
    char*            sizeof_name;
}; 

struct PostfixExprNode {
    PrimaryExprNode* primary_expr_node;
    PostfixExprNode* postfix_expr_node;
    ExprNode*        expr_node;
    Vector*          assign_expr_nodes;
    char*            identifier;
    int              identifier_len;
    int              postfix_expr_type; 
};

struct PrimaryExprNode {
    ConstantNode* constant_node;
    ExprNode*     expr_node;
    char*         string;
    char*         identifier;
    int           identifier_len;
    int           value_type;
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
    SpecifierQualifierNode* specifier_qualifier_node;
    bool                    is_pointer; 
};

struct ParamTypeListNode {
    bool           has_ellipsis;
    ParamListNode* param_list_node; 
};

struct ParamListNode {
    ParamDeclarationNode* param_declaration_node;
    ParamListNode*        param_list_node;  
};

struct ParamDeclarationNode {
    Vector*                 decl_spec_nodes;
    DeclaratorNode*         declarator_node;
};

struct EnumSpecifierNode {
    char*               identifier;
    EnumeratorListNode* enumerator_list_node;
};

struct EnumeratorListNode {
    Vector* identifiers;
};

struct DeclarationNode {
    Vector* decl_specifier_nodes;
    Vector* init_declarator_nodes;
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
    Vector* initializer_nodes;        
};

struct CompoundStmtNode {
    Vector* block_item_nodes;
};

struct BlockItemNode {
    DeclarationNode* declaration_node;
    StmtNode*        stmt_node;
};

struct StmtNode {
    LabeledStmtNode*   labeled_stmt_node;
    ExprStmtNode*      expr_stmt_node; 
    CompoundStmtNode*  compound_stmt_node;
    SelectionStmtNode* selection_stmt_node;
    ItrStmtNode*       itr_stmt_node;
    JumpStmtNode*      jump_stmt_node;
};

struct LabeledStmtNode {
    int                  labeled_stmt_type;
    ConditionalExprNode* conditional_expr_node;
    StmtNode*            stmt_node;
};

struct ExprStmtNode {
    ExprNode* expr_node;  
};

struct SelectionStmtNode {
    int       selection_type;
    ExprNode* expr_node;
    StmtNode* stmt_node[2];
};

struct ItrStmtNode {
    int        itr_type;
    StmtNode*  stmt_node;
    ExprNode*  expr_node[3];
    Vector*    declaration_nodes;
};

struct JumpStmtNode {
    int       jump_type;
    char*     identifier;
    ExprNode* expr_node;
};

//
// parse
//

TransUnitNode* parse(const Vector* vec);

//
// debug
//

const char* decode_type_specifier(int type_specifier);
const char* decode_type_qualifier(int type_qualifier);
const char* decode_storage_class_specifier(int storage_class_specifier);
const char* decode_struct_or_union(int struct_or_union);
const char* decode_operator_type(int operator_type);
const char* decode_comparison_operator_type(int type);
const char* decode_const_type(int type);
const char* decode_jump_type(int type);
const char* decode_unary_type(int type);
const char* decode_postfix_type(int type);
const char* decode_param_list_type(int type);
const char* decode_selection_stmt_type(int type);
const char* decode_itr_type(int type);
const char* decode_labeled_stmt_type(int type);

#endif
