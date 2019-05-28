#include "9cc.h"

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
