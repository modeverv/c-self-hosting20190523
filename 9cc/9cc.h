#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの型を表す値
enum
{
    TK_NUM = 256, // 整数トークン
    TK_IDENT,
    TK_RETURN,
    TK_EQ,
    TK_NE,
    TK_LE,
    TK_GE,
    TK_EOF, // 入力の終わりを表すトークン
};

// パーサーを書くときの基本的な戦略は非終端記号をそのまま関数にマップする
enum
{
    ND_NUM = 256, // ノードnum
    ND_IDENT,
    ND_RETURN,
    ND_EQ,
    ND_NE,
    ND_LE,
    ND_GE,
};

// トークンの型
typedef struct
{
    int ty;      // トークンの型
    int val;     // tyがTK_NUMの場合、その数値
    char *name; // tyがTK_IDENTの場合、その名前
    char *input; // トークン文字列(エラーメッセージ用途)

} Token;

typedef struct Node
{
    int ty;           //演算子かND_NUM
    struct Node *lhs; // 左辺
    struct Node *rhs; // 右辺
    int val;          // tyがND_NUMの場合のみ使う
    char name; // tyがND_IDENTの場合使う
} Node;

typedef struct
{
    void **data;
    int capacity;
    int len;
} Vector;

typedef struct {
    Vector *keys;
    Vector *vals;
} Map;

// プロトタイプ
void
tokenize();
Vector *new_vector();
void vec_push(Vector *vec, void *elem);
Node *new_node(int, Node *, Node *);
Node *new_node_num(int);
Node *new_node_ident(char*);
int consume(int);
Node *expr();
Node *mul();
Node *unary();
Node *term();
Node *equality();
Node *relational();
Node *add();
void program();
Node *stmt();
Node *assign();
void gen_lval(Node *);
void gen(Node *);
void error(char *, ...);
void error_at(char *, char *);
// test
int expect(int, int, int);
void runtest();
Map *new_map();
void map_put(Map *map, char *key, void *val);
void *map_get(Map *map, char *key);


// 入力プログラム
char *user_input;

// Tokenはここに入る
Vector *vec;

Node *code[1000];

