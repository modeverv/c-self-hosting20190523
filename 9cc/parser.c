#include "9cc.h"

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
