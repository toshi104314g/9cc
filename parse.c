
#include "9cc.h"

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

//数字かアンダースコアかどうかを判定する関数
int is_alnum(char c) {
  return ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') ||
         (c == '_');
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
  if (strlen(op) != token->len || memcmp(token->str, op, token->len))
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

// 次のトークンが識別子の場合、トークンを返却し、トークンを1つ読み進める。
// それ以外の場合にはエラーを返す。
Token *expect_ident() {
  if (token->kind != TK_IDENT)
    error_at(token->str,"識別子ではありません");
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

// 変数を名前で検索する。見つからなかった場合はNULLを返す。
LVar *find_lvar(Token *tok) {
  for (LVar *var = functions; var; var = var->next)
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
       return var; 
    }
  for (LVar *var = locals; var; var = var->next)
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
      return var;
    }
//printf("%s(%d) was NOT found\n", tok->str,tok->len);
  return NULL;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

void remember_functions(Token *tok, LVar *lvar, Node* node) {
  if (!lvar) {
    lvar = calloc(1, sizeof(LVar));
    lvar->next = functions;
    lvar->name = tok->str;
    lvar->len = tok->len;
    functions = lvar;
  }
}

void set_offset(Token *tok, LVar *lvar, Node* node) {
  if (lvar) {
    //他の関数で同じ文字を既に使用していたときはポインタを流用
    if (lvar->offset == 0) {
          lvar->offset = locals->offset + 8;
    } else {
        node->offset = lvar->offset;
//printf("%d\n", node->offset);
    }
  } else {
    lvar = calloc(1, sizeof(LVar));
    lvar->next = locals;
    lvar->name = tok->str;
    lvar->len = tok->len;
    if (locals) 
      lvar->offset = locals->offset + 8;
    else
      lvar->offset = 8;
//printf("set offset\n");
    node->offset = lvar->offset;
    locals = lvar;
  }
}

void *set_args(Node* node) {
  expect("("); 
  if (!consume(")")) {
    Node *cur = new_node(ND_FUNC, expr(), NULL);
    node->rhs = cur;
    while (!consume(")")) {
      expect(",");
      cur->rhs = new_node(ND_FUNC, expr(), NULL);
      cur = cur->rhs;
    }
  }
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
        p += 2;
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
        p += 2;
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
        p += 2;
        continue;
      }else {
        cur = new_token(TK_RESERVED, cur, p++);
        cur->len = 1;
        continue;
      }
    }
 
    if (*p == '+' || *p == '-' || *p == '*' || *p == '/'
	|| *p == '(' || *p == ')' || *p == ';' || *p == '{'
        || *p == '}' || *p == ',') {
      cur = new_token(TK_RESERVED, cur, p++);
      cur->len = 1;
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    
    if (!strncmp(p, "return", 6) && !is_alnum(*(p+6))) {
      cur = new_token(TK_RETURN, cur, p);
      cur->len = 6;
      p += 6;
      continue;
    }

    if (!strncmp(p, "if", 2) && !is_alnum(*(p+2))) {
      cur = new_token(TK_IF, cur, p);
      cur->len=2;
      p += 2;
      continue;
    }

    if (!strncmp(p, "else", 4) && !is_alnum(*(p+4))) {
      cur = new_token(TK_ELSE, cur, p);
      cur->len=4;
      p += 4;
      continue;
    }

    if (!strncmp(p, "while", 5) && !is_alnum(*(p+5))) {
      cur = new_token(TK_WHILE, cur, p);
      cur->len=5;
      p += 5;
      continue;
    }

    if (!strncmp(p, "for", 3) && !is_alnum(*(p+3))) {
      cur = new_token(TK_FOR, cur, p);
      cur->len=3;
      p += 3;
      continue;
    }

    if ( isalpha(*p) || *p == '_') {
      cur = new_token(TK_IDENT, cur, p++);
      for ( cur->len = 1;  is_alnum(*p); p++)
        cur->len++; 
      continue;
    }

    error_at(p,"トークナイズできません"); 
  } 
  new_token(TK_EOF, cur, p);
  return head.next;
}

void program() {
  int i = 0;
  while (!at_eof())
    code[i++] = func();
  code[i] = NULL;
} 

Node *func() {
  Node *node = new_node(ND_FUNC, NULL, NULL);

  Token *tok = consume_ident();
  if(tok) {
    LVar *lvar = find_lvar(tok);
//関数の名前を持ったグローバル変数のリストへ
    remember_functions(tok, lvar, node);
    node->name = tok->str;
    node->len = tok->len;
    
    //関数定義の仮引数にx+yや10みたいなのが許されてしまう
    set_args(node);
    expect("{");
    if (!consume("}")) {
      Node *cur = new_node(ND_FUNC, NULL, stmt());
      node->lhs = cur;
      while (!consume("}")) {
        cur->lhs = new_node(ND_FUNC, NULL, stmt());
        cur = cur->lhs;
      }
    } else {
        node->lhs = new_node(ND_FUNC, NULL, new_node(ND_EMPTY, NULL, NULL));
    }
    return node;
  } else if (consume(";")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_EMPTY;
    return node;
  } else {
    error_at(token->str, "関数ではありません");
  }
}  

Node *stmt() {
  Node *node;

  if (consume("return")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();

  } else if (consume("if")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_IF;
    expect("(");
    node->lhs = expr();
    expect(")");
    Node *exec_node = calloc(1, sizeof(Node));
    exec_node->kind = ND_IFEXEC;
    exec_node->lhs = stmt();
    if (consume("else")) 
      exec_node->rhs = stmt();
    node->rhs = exec_node;
    return node;

   } else if (consume("while")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_WHILE;
    expect("(");
    node->lhs = expr();
    expect(")");
    node->rhs = stmt();
    return node;

   } else if (consume("for")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_FOR;
    expect("(");
    Node *for_init_end = calloc(1, sizeof(Node));
    for_init_end->kind = ND_FORFOLLOW;
    if (!consume(";")) {
      for_init_end->lhs = expr();
      expect(";");
    }
    if (!consume(";")) {
      for_init_end->rhs = expr();
      expect(";");
	    }
    node->lhs = for_init_end;
    Node *for_iter_exec = calloc(1, sizeof(Node));
	    for_iter_exec->kind = ND_FORFOLLOW;
	    if (!consume(")")) {
      for_iter_exec->lhs = expr();
      expect(")");
    }
    for_iter_exec->rhs = stmt();
    node->rhs = for_iter_exec; 
    return node;
   
   } else if (consume(";")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_EMPTY;
    return node;

   } else if (consume("{")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    Node *cur = node;
    while(!consume("}")) {
      if (at_eof()) 
        error("}が閉じられていません");
      cur->lhs = stmt();
      cur->rhs = calloc(1, sizeof(Node));
      cur = cur->rhs;
      cur->rhs = NULL;
    } 
    return node;

   } else {
    node = expr();
   }

   if (!consume(";"))
    error_at(token->str, "';'ではないトークンです");
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

  // 次のトークンが識別子の場合（変数or関数）
  Token *tok = consume_ident();
  if (tok) {
    Node *node = calloc(1, sizeof(Node));
  
    LVar *lvar = find_lvar(tok);

    //関数の場合 
    if (!(strlen("(") != token->len || 
        memcmp(token->str, "(", token->len))) {
      node->kind = ND_FUNC;
      node->name = tok->str;
      node->len = tok->len;
      set_args(node);
      return node;
    }

    //変数の場合
    set_offset(tok, lvar, node);
    node->kind = ND_LVAR;
    return node;
  }

  // そうでなければ数値のはず
  return new_node_num(expect_number());
}


