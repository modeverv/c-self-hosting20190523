#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの型を表す値
enum
{
    TK_NUM = 256, // 整数トークン
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
    char *input; // トークン文字列(エラーメッセージ用途)
} Token;

typedef struct Node
{
    int ty;           //演算子かND_NUM
    struct Node *lhs; // 左辺
    struct Node *rhs; // 右辺
    int val;          // tyがND_NUMの場合のみ使う
} Node;

typedef struct
{
    void **data;
    int capacity;
    int len;
} Vector;

// プロトタイプ
void tokenize();
Vector *new_vector();
void vec_push(Vector *vec, void *elem);
Node *new_node(int, Node *, Node *);
Node *new_node_num(int);
int consume(int);
Node *expr();
Node *mul();
Node *unary();
Node *term();
Node *equality();
Node *relational();
Node *add();
void gen(Node *);
void error(char *, ...);
void error_at(char *, char *);
// test
int expect(int, int, int);
void runtest();

// 入力プログラム
char *user_input;

// Tokenはここに入る
Vector *vec;

