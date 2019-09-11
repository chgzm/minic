#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"

enum NodeType {
    ND_TRANS_UNIT,
    ND_FUNC_DEF,
    ND_COMPOUND_STMT,
    ND_EXPR,
    ND_RETURN,
    ND_JUMP_STMT,
    ND_STMT
};

enum TypeSpecifier {
    TYPE_INT,
    TYPE_CHAR,
    TYPE_VOID
};

typedef struct NodeVec NodeVec;
struct NodeVec {
    void** nodes;     
    int    size; 
    int    capacity;
};

typedef struct TransUnitNode TransUnitNode;
struct TransUnitNode {
    int      node_type;
    NodeVec* func_def;
};

typedef struct CompoundStmtNode CompoundStmtNode;
struct CompoundStmtNode {
    int      node_type;
    NodeVec* stmt;     
};

typedef struct FuncDefNode FuncDefNode;
struct FuncDefNode {
    int               node_type;
    int               type_specifier;
    char*             identifier;
    CompoundStmtNode* compound_stmt;
};

typedef struct ExprNode ExprNode;
struct ExprNode {
    int node_type;
    int integer_constant;             
};

typedef struct ReturnNode ReturnNode;
struct ReturnNode {
    int       node_type;
    ExprNode* expr;
};

typedef struct JumpStmtNode JumpStmtNode;
struct JumpStmtNode {
    int         node_type;
    ReturnNode* ret;    
};

typedef struct StmtNode StmtNode;
struct StmtNode {
    int           node_type;
    JumpStmtNode* jump_stmt;
};

TransUnitNode* parse(TokenVec* vec);

#endif
