
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類
typedef enum {
  TK_RESERVED, // 記号
  TK_IDENT,    // 識別子
  TK_NUM,      // 整数トークン
  TK_RETURN,   // return
  TK_IF,       // if
  TK_ELSE,     // else
  TK_WHILE,    // while
  TK_FOR,      // for
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  char *str;      // トークン文字列
  int len;	  // トークンの長さ
};

// 現在着目しているトークン
extern Token *token;

// エラー箇所を報告する
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

// 抽象構文木のノードの種類
typedef enum {
  ND_LE, // <=
  ND_LT,  // <
  //ND_GE, // >=
  //ND_GT,  // >
  ND_EQ,  // ==
  ND_NE, // !=
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_NUM, // 整数
  ND_ASSIGN, // =
  ND_LVAR, // ローカル変数
  ND_IF, // IF
  ND_IFEXEC, // ifの実行文
  ND_WHILE, // while
  ND_FOR, // for
  ND_FORFOLLOW, // for文内の条件式や実行文
  ND_EMPTY, // 空文
  ND_RETURN, // return
  ND_BLOCK, // ブロック{ ... }
  ND_FUNC, // 関数
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
  NodeKind kind; // ノードの型
  Node *lhs;     // 左辺
  Node *rhs;     // 右辺
  int val;       // kindがND_NUMの場合のみ使う
  int offset;    // kindがND_LVARの場合のみ使う
  char *name;    // kindがND_FUNCの場合のみ使う
  int len;       // kindがND_FUNCの場合のみ使う
};

typedef struct LVar LVar;

// ローカル変数の型
struct LVar {
  LVar *next; // 次の変数かNULL
  char *name; // 変数の名前
  int len;    // 名前の長さ
  int offset; // RBPからのオフセット
};

Token *tokenize(char *p);

void program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

void gen(Node *node);
    
extern Node *code[100];

// ローカル変数
LVar *locals;

