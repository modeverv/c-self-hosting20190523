#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの型を表す値
enum
{
    TK_NUM = 256, // 整数トークン
    TK_EOF,       // 入力の終わりを表すトークン
};

// パーサーを書くときの基本的な戦略は非終端記号をそのまま関数にマップする
enum
{
    ND_NUM = 256, // ノードnum
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

// プロトタイプ
void error_at(char*, char*);
void tokenize();
Node *new_node(int, Node*, Node*);
Node *new_node_num(int);
int consume(int);
Node *expr();
Node *mul();
Node *term();
void gen(Node *);
void error(char*, ...);

// 入力プログラム
char *user_input;

// トークナイズした結果のトークン列はこの配列に保存する
// 100娘以上のトークンは来ないものとする
Token tokens[100];

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// エラー箇所を報告するための関数
void error_at(char *loc, char *msg)
{
    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
    fprintf(stderr, "^ %s\n", msg);
    exit(1);
}

void tokenize()
{
    char *p = user_input;

    int i = 0;
    while (*p)
    {
        // 空白をスキップ
        if (isspace(*p))
        {
            p++;
            continue;
        }
        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')')
        {
            tokens[i].ty = *p;
            tokens[i].input = p;
            i++;
            p++;
            continue;
        }
        if (isdigit(*p))
        {
            tokens[i].ty = TK_NUM;
            tokens[i].input = p;
            tokens[i].val = strtol(p, &p, 10);
            i++;
            continue;
        }
        error_at(p, "トークナイズできません");
    }
    tokens[i].ty = TK_EOF;
    tokens[i].input = p;
}

Node *new_node(int ty, Node *lhs, Node *rhs)
{
    Node *node = malloc(sizeof(Node));
    node->ty = ty;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val)
{
    //↑ポインタを返す関数
    Node *node = malloc(sizeof(Node));
    node->ty = ND_NUM;
    node->val = val;
    return node;
}

int pos = 0;

int consume(int ty)
{
    if (tokens[pos].ty != ty)
    {
        return 0;
    }
    pos++;
    return 1;
}

/*
expr = mul ("+" mul | "-" mul)*
mul  = term ("*" term | "/" term)*
term = num | "(" expr ")"
*/

// EBNF term
//term = num | "(" expr ")"
Node *term()
{
    // 次のトークンが'(' '(' expr ')'のはず)
    if (consume('('))
    { //)
        Node *node = expr();
        if (!consume(')'))
        {
            error_at(tokens[pos].input, "開きカッコにたいして閉じカッコがありません");
        }
        return node;
    }
    // そうでなければ数値
    if (tokens[pos].ty == TK_NUM)
    {
        return new_node_num(tokens[pos++].val);
    }
}
// EBNF mul
// mul  = term ("*" term | "/" term)*
Node *mul()
{
    Node *node = term();
    for (;;)
    {
        if (consume('*'))
        {
            node = new_node('*', node, term());
        }
        else if (consume('/'))
        {
            node = new_node('/', node, term());
        }
        else
        {
            return node;
        }
    }
}

// EBNF expr
// expr = mul ("+" mul | "-" mul)*
Node *expr()
{
    Node *node = mul();
    for (;;)
    {
        if (consume('+'))
        {
            node = new_node('+', node, mul());
        }
        else if (consume('-'))
        {
            node = new_node('-', node, mul());
        }
        else
        {
            return node;
        }
    }
}

void gen(Node *node)
{
    if (node->ty == ND_NUM)
    {
        printf("  push %d\n", node->val);
        return;
    }
    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->ty)
    {
    case '+':
        printf("  add rax, rdi\n");
        break;
    case '-':
        printf("  sub rax, rdi\n");
        break;
    case '*':
        printf("  imul rdi\n");
        break;
    case '/':
        printf("  cqo\n");
        printf("  idiv rdi\n");
        break;
    }
    printf("  push rax\n");
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        error("引数が不正");
        return 1;
    }

    // トークナイズする
    user_input = argv[1];
    tokenize();
    Node *node = expr();

    // アセンブリの前半を出力
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    gen(node);

    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}
