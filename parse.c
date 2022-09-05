
#include "9cc.h"

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

// 次のトークンが識別子の場合、トークンを返却し、トークンを1つ読み進める。
// それ以外の場合にはNULLポインタを返す。
Token *consume_ident() {
  if (token->kind != TK_IDENT)
    return NULL; 
  Token *ident_token = token;
  token = token->next;
  return ident_token; 
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
    error_at(token->str,"'%c'ではありません", op);
  token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str,"数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // 空白文字をスキップ
    if (isspace(*p) || *p == '\n' || *p == '\t') {
      p++;
      continue;
    }

    if (*p == '<' || *p == '>') {
      if (*(p+1) == '=') {
        cur = new_token(TK_RESERVED, cur, p);
        cur->len = 2;
        p=p+2;
        continue;
      }else {
        cur = new_token(TK_RESERVED, cur, p++);
        cur->len = 1;
        continue;
      }
    }

//
    if (*p == '=') {
      if (*(p+1) == '=') {
        cur = new_token(TK_RESERVED, cur, p);
        cur->len = 2;
        p=p+2;
        continue;
      }else {
        cur = new_token(TK_RESERVED, cur, p++);
        cur->len = 1;
        continue;
      }
    }

//!が通ってしまう
    if (*p == '!') {
      if (*(p+1) == '=') {
        cur = new_token(TK_RESERVED, cur, p);
        cur->len = 2;
        p=p+2;
        continue;
      }else {
        cur = new_token(TK_RESERVED, cur, p++);
        cur->len = 1;
        continue;
      }
    }
 
    if (*p == '+' || *p == '-' || *p == '*' || *p == '/'
	|| *p == '(' || *p == ')' || *p == ';') {
      cur = new_token(TK_RESERVED, cur, p++);
      cur->len = 1;
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    if ('a' <= *p && *p <= 'z') {
      cur = new_token(TK_IDENT, cur, p++);
      cur->len = 1;
      continue;
    }

    error_at(token->str,"トークナイズできません");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

void program() {
  int i = 0;
  while (!at_eof())
    code[i++] = stmt();
  code[i] = NULL;
} 

Node *stmt() {
  Node *node = expr();

  expect(";");
  return node;
}

Node *expr() {
  return assign();
}

Node *assign() {
  Node *node = equality();

  if (consume("="))
    node = new_node(ND_ASSIGN, node, assign());
  return node;
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_node(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_node(ND_NE, node, relational());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_node(ND_LT, node, add());
    else if (consume("<="))
      node = new_node(ND_LE, node, add());
    //>の場合は両辺入れ替えて<の処理をする
    else if (consume(">"))
      node = new_node(ND_LT, add(), node);
    //>=の場合は両辺入れ替えて<=の処理をする
    else if (consume(">="))
      node = new_node(ND_LE, add(), node);
    else
      return node;
  }
}


Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}


Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}


Node *unary() {
  if (consume("+"))
    return primary();
  if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), primary());
  return primary();
}

Node *primary() {
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  // 次のトークンがローカル変数(識別子)の場合
  Token *tok = consume_ident();
  if (tok) {
    //new_node_lvar()の実行
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    node->offset = (tok->str[0] - 'a' + 1) * 8;
    return node;
  }

  // そうでなければ数値のはず
  return new_node_num(expect_number());
}


