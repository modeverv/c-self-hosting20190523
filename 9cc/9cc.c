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

Vector *new_vector()
{
    Vector *vec = malloc(sizeof(Vector));
    vec->data = malloc(sizeof(void *) * 16);
    vec->capacity = 16;
    vec->len = 0;
    return vec;
}
void vec_push(Vector *vec, void *elem)
{
    if (vec->capacity == vec->len)
    {
        vec->capacity *= 2;
        vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
    }
    vec->data[vec->len++] = elem;
}

// プロトタイプ
void error_at(char *, char *);
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
// test
int expect(int, int, int);
void runtest();

// 入力プログラム
char *user_input;

// Tokenはここに入る
Vector *vec;

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
        Token *t = malloc(sizeof(*t));
        // 空白をスキップ
        if (isspace(*p))
        {
            p++;
            continue;
        }

        // == != <= >=
        if (strncmp(p, "==", 2) == 0)
        {
            t->ty = TK_EQ;
            t->input = p;
            vec_push(vec, (void *)t);

            p++;
            p++;
            continue;
        }

        if (strncmp(p, "!=", 2) == 0)
        {
            t->ty = TK_NE;
            t->input = p;
            vec_push(vec, (void *)t);

            p++;
            p++;
            continue;
        }

        if (strncmp(p, "<=", 2) == 0)
        {
            t->ty = TK_LE;
            t->input = p;
            vec_push(vec, (void *)t);

            p++;
            p++;
            continue;
        }

        if (strncmp(p, ">=", 2) == 0)
        {
            t->ty = TK_GE;
            t->input = p;
            vec_push(vec, (void *)t);

            p++;
            p++;
            continue;
        }

        if (*p == '>' || *p == '<')
        {
            t->ty = *p;
            t->input = p;
            vec_push(vec, (void *)t);

            p++;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')')
        {
            t->ty = *p;
            t->input = p;
            vec_push(vec, (void *)t);

            p++;
            continue;
        }
        if (isdigit(*p))
        {
            int val = strtol(p, &p, 10);
            t->ty = TK_NUM;
            t->input = p;
            t->val = val;
            vec_push(vec, (void *)t);

            continue;
        }
        error_at(p, "トークナイズできません");
    }
    Token *t = malloc(sizeof(*t));
    t->ty = TK_EOF;
    t->input = p;
    vec_push(vec, (void *)t);
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

unsigned long pos = 0;

int consume(int ty)
{

    if (((Token *)vec->data[pos])->ty != ty)
    {
        return 0;
    }
    pos++;
    return 1;
}

/*
expr       = equality
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = ("+" | "-")? term
term       = num | "(" expr ")"
*/

Node *expr()
{
    return equality();
}

Node *equality()
{
    Node *node = relational();
    if (consume(TK_EQ))
    {
        return new_node(ND_EQ, node, relational());
    }
    else if (consume(TK_NE))
    {
        return new_node(ND_NE, node, relational());
    }
    else
    {
        return node;
    }
}

Node *relational()
{
    Node *node = add();
    if (consume('<'))
    {
        return new_node('<', node, add());
    }
    if (consume(TK_LE))
    {
        return new_node(ND_LE, node, add());
    }
    if (consume('>'))
    {
        return new_node('<', add(), node);
    }
    if (consume(TK_GE))
    {
        return new_node(ND_LE, add(), node);
    }
    return node;
}

Node *add()
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

Node *mul()
{
    Node *node = unary();
    if (consume('*'))
    {
        node = new_node('*', node, unary());
    }
    else if (consume('/'))
    {
        node = new_node('/', node, unary());
    }
    else
    {
        return node;
    }
}

Node *unary()
{
    if (consume('+'))
    {
        return term();
    }
    if (consume('-'))
    {
        return new_node('-', new_node_num(0), term());
    }
    else
    {
        return term();
    }
}

Node *term()
{
    // 次のトークンが'(' '(' expr ')'のはず)
    if (consume('('))
    {
        Node *node = expr();
        if (!consume(')'))
        {
            error_at(((Token *)vec->data[pos])->input, "開きカッコにたいして閉じカッコがありません");
        }
        return node;
    }
    // そうでなければ数値
    if (((Token *)vec->data[pos])->ty == TK_NUM)
    {
        return new_node_num(((Token *)vec->data[pos++])->val);
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
    case ND_EQ:
        printf("  cmp rax, rdi\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_NE:
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movzb rax, al\n");
        break;
    case '<':
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_LE:
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
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

    if (strcmp(argv[1], "-test") == 0)
    {
        runtest();
        return 0;
    }

    vec = new_vector();

    // トークナイズする
    user_input = argv[1];
    //user_input = "5+10";
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

int expect(int line, int expected, int actual)
{
    if (expected == actual)
        return 0;
    fprintf(stderr, "%d: %d expected, but got %d\n",
            line, expected, actual);
    exit(1);
}

void runtest()
{
    Vector *vec = new_vector();
    expect(__LINE__, 0, vec->len);

    for (int i = 0; i < 100; i++)
        vec_push(vec, (void *)i);

    expect(__LINE__, 100, vec->len);
    expect(__LINE__, 0, (long)vec->data[0]);
    expect(__LINE__, 50, (long)vec->data[50]);
    expect(__LINE__, 99, (long)vec->data[99]);

    printf("OK\n");
    return;
}
