
#include "9cc.h"

int label;

//変数のoffsetからメモリアドレスを計算してアドレスをスタックに積む
void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void gen(Node *node) {
switch (node->kind) {
  case ND_EMPTY:
    //ノードを評価した場合は何かしらの値が必要
    //0にすると比較演算子を通ってしまうので1とする
    printf("  push 1\n");
    return;
  case ND_BLOCK:
    for (Node *cur = node; cur->rhs ; cur=cur->rhs)
      gen(cur->lhs);
    return; 
  case ND_RETURN:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  case ND_IF:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    if ((node->rhs)->rhs) {
      printf("  je .Lelse%d\n", ++label);
      gen((node->rhs)->lhs);
      printf("  jmp .Lend%d\n", label);
      printf(".Lelse%d:\n", label);
      gen((node->rhs)->rhs);
      printf(".Lend%d:\n", label);
    } else {
      printf("  je .Lend%d\n", ++label);
      gen((node->rhs)->lhs);
      printf(".Lend%d:\n", label);
    } 
    return;
  case ND_WHILE:
    printf(".Lbegin%d:\n", ++label);
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%d\n", label);
    gen(node->rhs);
    printf("  jmp .Lbegin%d\n", label);
    printf("  jmp .Lend%d\n", label);
    printf(".Lend%d:\n", label);
    return;
  case ND_FOR:
    if (node->lhs->lhs)
      gen(node->lhs->lhs);
    printf(".Lbegin%d:\n", ++label);
    if (node->lhs->rhs)
      gen(node->lhs->rhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%d\n", label);
    gen(node->rhs->rhs);
    if (node->rhs->lhs)
     gen(node->rhs->lhs);
    printf("  jmp .Lbegin%d\n", label);
    printf(".Lend%d:\n", label); 
  case ND_NUM:
    printf("  push %d\n", node->val);
    return;
  //右辺値として計算
  case ND_LVAR:
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  case ND_ASSIGN:
    gen_lval(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  sub  rsp, 8\n");
    printf("  push rdi\n");
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  }

  if (node->kind == ND_EQ) {
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
  }

  if (node->kind == ND_LT) {
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
  }

   if (node->kind == ND_LE) {
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
  }

   if (node->kind == ND_NE) {
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
  }


 printf("  push rax\n");
}


