
#include "9cc.h"

Node *code[100];

//ローカル変数
LVar *locals;

//関数名のリスト
LVar *functions;

// 現在着目しているトークン
Token *token;

// 入力プログラム
char *user_input;

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません");
    return 1;
  }

  // トークナイズしてパースする
  user_input = argv[1];
  token = tokenize(user_input);
  program(); 

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");

  // プロローグ
  
    // 先頭の式から順にコード生成
    for (int i = 0; code[i]; i++) {
      gen(code[i]);
 
      // 式の評価結果としてスタックに一つの値が残っている
      // はずなので、スタックが溢れないようにポップしておく
      printf("  pop rax\n");
    }
  
    // エピローグ
    // 最後の式の結果がRAXに残っているのでそれが返り値になる
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
}

