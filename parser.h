#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"

enum NodeType {
    ND_TRANS_UNIT,
    ND_EXTERNAL_DECL,
    ND_FUNC_DEF,
    ND_EXPR,
    ND_RETURN, 
    ND_COMPOUND_STMT,
    ND_STMT,
    ND_LABELED_STMT,
    ND_EXPR_STMT,
    ND_SELECT_STMT,
    ND_ITER_STMT,
    ND_JUMP_STMT,
    ND_ASSIGN_EXPR,
    ND_CONDITIONAL_EXPR,
    ND_LOGICAL_OR_EXPR,
    ND_LOGICAL_AND_EXPR,
    ND_INCLUSIVE_OR_EXPR,
    ND_EXCLUSIVE_OR_EXPR,
    ND_AND_EXPR,
    ND_EQUALITY_EXPR,
    ND_RELATIONAL_EXPR,
    ND_SHIFT_EXPR,
    ND_ADDITIVE_EXPR,
    ND_MULTIPLICATIVE_EXPR,
    ND_CAST_EXPR,
    ND_UNARY_EXPR,
    ND_POSTFIX_EXPR,
    ND_PRIMARY_EXPR,
    ND_CONSTANT
};
 
enum TypeSpecifier {
    TYPE_INT,
    TYPE_CHAR,
    TYPE_VOID
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

typedef struct NodeVec NodeVec;
typedef struct TransUnitNode TransUnitNode;
typedef struct ExternalDeclNode ExternalDeclNode;
typedef struct FuncDefNode FuncDefNode;
typedef struct DeclNode DeclNode;
typedef struct ConstantNode ConstantNode;
typedef struct PrimaryExprNode PrimaryExprNode;
typedef struct PostfixExprNode PostfixExprNode;
typedef struct UnaryExprNode UnaryExprNode;
typedef struct CastExprNode CastExprNode;
typedef struct MultiPlicativeExprNode MultiPlicativeExprNode;
typedef struct AdditiveExprNode AdditiveExprNode;
typedef struct ShiftExprNode ShiftExprNode;
typedef struct RelationalExprNode RelationalExprNode;
typedef struct EqualityExprNode EqualityExprNode;
typedef struct AndExprNode AndExprNode;
typedef struct ExclusiveOrExprNode ExclusiveOrExprNode;
typedef struct InclusiveOrExprNode InclusiveOrExprNode;
typedef struct LogicalAndExprNode LogicalAndExprNode;
typedef struct LogicalOrExprNode LogicalOrExprNode;
typedef struct ConditionalExprNode ConditionalExprNode;
typedef struct AssignExprNode AssignExprNode;
typedef struct ExprNode ExprNode;
typedef struct ReturnNode ReturnNode;
typedef struct CompoundStmtNode CompoundStmtNode;
typedef struct StmtNode StmtNode;
typedef struct LabeledStmtNode LabeledStmtNode;
typedef struct ExprStmtNode ExprStmtNode;
typedef struct SelectStmtNode SelectStmtNode;
typedef struct IterStmtNode IterStmtNode;
typedef struct JumpStmtNode JumpStmtNode;
typedef struct NodeVec NodeVec;

struct NodeVec {
    void** nodes;     
    int    size; 
    int    capacity;
};

struct TransUnitNode {
    int      node_type;
    NodeVec* external_decl_nodes;
};

struct ExternalDeclNode {
    int          node_type;
    FuncDefNode* func_def_node;
    DeclNode*    decl_node;
};

struct FuncDefNode {
    int               node_type;
    int               type_specifier;
    char*             identifier;
    CompoundStmtNode* compound_stmt_node;
};

struct DeclNode {
    int node_type;
};

struct ConstantNode {
    int   node_type;
    int   const_type;
    int   integer_constant;
    int   enumeration_constant;
    char* character_constant;  
    float floating_constant;
}; 

struct PrimaryExprNode {
    int           node_type;
    ConstantNode* constant_node;
    ExprNode*     expr_node;
    char*         string;
    char*         identifier;
};

struct PostfixExprNode {
    int              node_type;
    PrimaryExprNode* primary_expr_node;
    PostfixExprNode* postfix_expr_node;
    ExprNode*        expr_node;
    AssignExprNode*  assignment_expr_node;
    char*            identifier;
};

struct UnaryExprNode {
    int              node_type;
    PostfixExprNode* postfix_expr_node;
    UnaryExprNode*   unary_expr_node;
    CastExprNode*    cast_expr_node;
    int              type_name;
}; 

struct CastExprNode {
    int            node_type;
    UnaryExprNode* unary_expr_node;
    CastExprNode*  cast_expr_node;
    int            type_name;
};  

struct MultiPlicativeExprNode {
    int                     node_type;
    int                     operator_type;
    CastExprNode*           cast_expr_node;
    MultiPlicativeExprNode* multiplicative_expr_node;
};

struct AdditiveExprNode {
    int                     node_type;
    int                     operator_type;
    MultiPlicativeExprNode* multiplicative_expr_node;
    AdditiveExprNode*       additive_expr_node;

};

struct ShiftExprNode {
    int               node_type;
    AdditiveExprNode* additive_expr_node;
    ShiftExprNode*    shift_expr_node;
};

struct RelationalExprNode {
    int                 node_type;
    ShiftExprNode*      shift_expr_node;
    RelationalExprNode* relational_expr_node;
};

struct EqualityExprNode {
    int                 node_type;
    RelationalExprNode* relational_expr_node;
    EqualityExprNode*   equality_expr_node;
};

struct AndExprNode {
    int               node_type;
    EqualityExprNode* equality_expr_node;
    AndExprNode*      and_expr_node;
};

struct ExclusiveOrExprNode {
    int                  node_type;
    AndExprNode*         and_expr_node;
    ExclusiveOrExprNode* exclusive_or_expr_node;
};

struct InclusiveOrExprNode {
    int                  node_type;
    ExclusiveOrExprNode* exclusive_or_expr_node;
    InclusiveOrExprNode* inclusive_or_expr_node;
};

struct LogicalAndExprNode {
    int                  node_type;
    InclusiveOrExprNode* inclusive_or_expr_node;
    LogicalAndExprNode*  logical_and_expr_node;
};

struct LogicalOrExprNode {
    int                 node_type;
    LogicalAndExprNode* logical_and_expr_node;
    LogicalOrExprNode*  logical_or_expr_node;
};

struct ConditionalExprNode {
    int                  node_type;
    LogicalOrExprNode*   logical_or_expr_node;
    LogicalAndExprNode*  logical_and_expr_node;
    ConditionalExprNode* conditional_expr_node;
};

struct AssignExprNode {
    int                  node_type;
    ConditionalExprNode* conditional_expr_node;
    UnaryExprNode*       unary_expr_node;
    AssignExprNode*      assign_expr_node;
    int                  assign_operator;
};

struct ExprNode {
    int             node_type;
    AssignExprNode* assign_expr_node;
    ExprNode*       expr_node;
};

struct ReturnNode {
    int       node_type;
    ExprNode* expr;
};

struct CompoundStmtNode {
    int      node_type;
    NodeVec* stmt;     
};

struct StmtNode {
    int           node_type;
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
    int         node_type;
    ReturnNode* ret;    
};


TransUnitNode* parse(TokenVec* vec);

//
// debug
//

const char* decode_node_type(int node_type);

#endif
