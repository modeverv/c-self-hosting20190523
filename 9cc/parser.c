#include "9cc.h"

int is_alnum(char c) {
  return ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') ||
         (c == '_');
}

void tokenize()
{
    char *p = user_input;

    while (*p)
    {
        Token *t = malloc(sizeof(*t));
        // 空白をスキップ
        if (isspace(*p))
        {
            p++;
            continue;
        }

        if (*p == ';')
        {
            t->ty = ';';
            t->input = p;
            vec_push(vec, (void *)t);
            p++;
            continue;
        }

        if (strncmp(p, "return", 6)==0 && !is_alnum(p[6]))
        {
            t->ty = TK_RETURN;
            t->input = p;
            vec_push(vec, (void *)t);
            p += 6;
            continue;
        }

        if ('a' <= *p && *p <= 'z')
        {
            t->ty = TK_IDENT;
            t->input = p;
            t->val = *p - 'a';
            size_t num = 1;
            char *pp = p;
            p++;
            while ('a' <= *p && *p <= 'z')
            {
                num++;
                p++;
            }
            char buf[num];
            strncpy(buf, pp, num);
            buf[num] = '\0';
            t->name = strdup(buf);
            vec_push(vec, (void *)t);
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

        if (*p == '=')
        {
            t->ty = '=';
            t->input = p;
            vec_push(vec, (void *)t);
            p++;
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

Node *new_node_ident(char *val)
{
    //error("%c",val);
    Node *node = malloc(sizeof(Node));
    node->ty = ND_IDENT;
    //printf("new_node_ident %s\n", val);
    node->name = val;
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
- program    = stmt*
- stmt       = expr ";"
expr       = assign
- assign     = equality ("=" assign)?
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = ("+" | "-")? term
term       = num | ident | "(" expr ")"
*/

Node *assign()
{
    Node *node = equality();
    if (consume('='))
        node = new_node('=', node, assign());
    return node;
}

Node *expr()
{
    return assign();
}

Node *stmt()
{
    Node *node;
    if(consume(TK_RETURN))
    {
        node = malloc(sizeof(Node));
        node->ty = ND_RETURN;
        node->lhs = expr();
    }else {
        node = expr();
    }

    if (!consume(';'))
        error_at(((Token *)vec->data[pos])->input, "';'ではないトークンです");
    return node;
}

void program()
{
    int i = 0;
    while (((Token *)vec->data[pos])->ty != TK_EOF)
        code[i++] = stmt();
    code[i] = NULL;
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
    return node;
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
    if  (((Token *)vec->data[pos])->ty == TK_IDENT) 
    {
        return new_node_ident(((Token *)vec->data[pos++])->name);
    }
    if (((Token *)vec->data[pos])->ty == TK_NUM)
    {
        return new_node_num(((Token *)vec->data[pos++])->val);
    }
    if  (((Token *)vec->data[pos])->ty == TK_IDENT) 
    {
        return new_node_ident(((Token *)vec->data[pos++])->name);
    }
}
