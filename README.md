# minic
- Mini C compiler written in C.
- The purpose of this project is to learn C language and compiler, so far from complete implementation.

# Build
```
make
```

# Usage
```
Usage: minic [OPTION] file

OPTION:
   -d, --debug    output debug-log.
```

# Test
```
make test
```

# Syntax
```
translation-unit: 
    {external-declaration}*

external-declaration:
    function-definition
    declaration
    "typedef" "struct" identifier typedef-name
    enum-specifier ';'

function-definition: 
    {declaration-specifier}* declarator compound-statement

declaration-specifier:
    type-specifier
    type-qualifier
    "static"
    
type-specifier:
    "void"
    "char"
    "int"
    "double"
    struct-specifier
    typedef-name

struct-specifier:
    "struct" {identifier}? '{' {struct-declaration}+ '}'
    "struct" identifier

struct-declaration:
    {specifier-qualifier}* struct-declarator-list ';'

specifier-qualifier:
    type-specifier
    type-qualifier

struct-declarator-list:
    declarator {',' declarator}*

declarator:
    {pointer}? direct-declarator

pointer:
    {'*'}+

type-qualifier:
    const
    volatile

direct-declarator:
    identifier
    '(' declarator ')'
    direct-declarator '[' {conditional-expression}? ']'
    direct-declarator '(' parameter-type-list ')'
    direct-declarator '(' {identifier}* ')'

conditional-expression:
    logical-or-expression
    logical-or-expression '?' expression ':' conditional-expression

logical-or-expression:
    logical-and-expression
    logical-or-expression "||" logical-and-expression

logical-and-expression:
    inclusive-or-expression
    logical-and-expression "&&" inclusive-or-expression

inclusive-or-expression: 
    exclusive-or-expression
    inclusive-or-expression '|' exclusive-or-expression

exclusive-or-expression: 
    and-expression
    exclusive-or-expression '^' and-expression

and-expression:
    equality-expression
    and-expression '&' equality-expression

equality-expression: 
    relational-expression
    equality-expression "==" relational-expression
    equality-expression "!=" relational-expression

relational-expression: 
    shift-expression
    relational-expression '<'  shift-expression
    relational-expression '>'  shift-expression
    relational-expression "<=" shift-expression
    relational-expression ">=" shift-expression

shift-expression: 
    additive-expression
    shift-expression "<<" additive-expression
    shift-expression ">>" additive-expression

additive-expression:
    multiplicative-expression
    additive-expression '+' multiplicative-expression
    additive-expression '-' multiplicative-expression

multiplicative-expression: 
    cast-expression
    multiplicative-expression '*' cast-expression
    multiplicative-expression '/' cast-expression
    multiplicative-expression '%' cast-expression

cast-expression:
    unary-expression
    '(' type-name ')' cast-expression

unary-expression: 
    postfix-expression
    ++ unary-expression
    -- unary-expression
    unary-operator cast-expression
    sizeof unary-expression
    sizeof type-name

postfix-expression: 
    primary-expression
    postfix-expression '[ expression ']'
    postfix-expression '(' {assignment-expression}* ')'
    postfix-expression '.'  identifier
    postfix-expression "->" identifier
    postfix-expression "++"
    postfix-expression "--"

primary-expression: 
    identifier
    constant
    string
    '(' expression ')'

constant: 
    integer-constant
    character-constant
    floating-constant
    enumeration-constant

expression:
    assignment-expression
    expression ',' assignment-expression

assignment-expression: 
    conditional-expression
    unary-expression assignment-operator assignment-expression

assignment-operator:
    "=" 
    "*="
    "/="
    "%="
    "+="
    "-="
    "&="
    "^="
    "|="

unary-operator: 
    '&'
    '*'
    '+'
    '-'
    '~'
    '!'

type-name: 
    {specifier-qualifier}+ {abstract-declarator}?

parameter-type-list: 
    parameter-list {, ...}?

parameter-list:
    parameter-declaration {, parameter-declaration}*

parameter-declaration: 
    {declaration-specifier}+ declarator
    {declaration-specifier}+ abstract-declarator
    {declaration-specifier}+

abstract-declarator:
    pointer
    pointer direct-abstract-declarator
    direct-abstract-declarator

direct-abstract-declarator:
    '(' abstract-declarator ')'
    {direct-abstract-declarator}? '[' {conditional-expression}? ']'
    {direct-abstract-declarator}? '(' {parameter-type-list}? ')'

enum-specifier: 
    enum {identifier}? '{' enumerator-list '}'

enumerator-list:
    identifier {, identifier}*

typedef-name:
    identifier

declaration:
    {declaration-specifier}+ {init-declarator {','}?}* ';'

init-declarator:
    declarator {'=' initializer}?

initializer:
    assignment-expression
    '{' initializer-list '}'
    '{' initializer-list ',' '}'

initializer-list:
    initializer {, initializer}*

compound-statement: 
    '{' {declaration}* {statement}* '}'

statement: 
    labeled-statement
    expression-statement
    compound-statement
    selection-statement
    iteration-statement
    jump-statement

labeled-statement: 
    "case" conditional-expression ':' statement
    "default" ':' statement

expression-statement: 
    {expression}? ';'

selection-statement:
    "if" '(' expression ')' statement
    "if" '(' expression ')' statement "else" statement
    "switch" '(' expression ')' statement

iteration-statement: 
    "while" '(' expression ')' statement
    "for" '(' {expression}? ';' {expression}? ';' {expression}? ')' statement
    "for" '(' {declaration}* ';' {expression}? ';' {expression}? ')' statement

jump-statement: 
    "continue" ';'
    "break" ';'
    "return" {expression}? ';'
```

# Reference
- [N1570](http://port70.net/~nsz/c/c11/n1570.html)
- [9cc](https://github.com/rui314/9cc)
- [低レイヤを知りたい人のためのCコンパイラ作成入門](https://www.sigbus.info/compilerbook)
