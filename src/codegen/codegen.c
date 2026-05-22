#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "codegen.h"
#include "tree.h"

static instr_t *ilist = NULL, *ilast = NULL;

static void emit(instr_t *instr) {
  if (ilast) ilast = ilast->next = instr;
  else       ilist = ilast = instr;
}

static instr_t *make_oper(char *assem, temp_t *dst, int ndst, temp_t *src, int nsrc) {
  instr_t *i = malloc(sizeof(instr_t));
  i->kind           = INSTR_OPER;
  i->assem          = assem;
  i->oper.dst       = dst;
  i->oper.dst_num   = ndst;
  i->oper.src       = src;
  i->oper.src_num   = nsrc;
  i->oper.jumps     = NULL;
  i->oper.jumps_num = 0;
  i->next = NULL;
  return i;
}
static instr_t *make_move(char *assem, temp_t dst, temp_t src) {
  instr_t *i  = malloc(sizeof(instr_t));
  i->kind     = INSTR_MOVE;
  i->assem    = assem;
  i->move.dst = dst;
  i->move.src = src;
  i->next = NULL;
  return i;
}

static instr_t *make_label(char *assem, label_t label) {
  instr_t *i = malloc(sizeof(instr_t));
  i->kind = INSTR_LABEL;
  i->assem = assem;
  i->label.label = label;
  i->next = NULL;
  return i;
}

static instr_t *make_jump(char *assem, label_t *jumps, int njumps) {
  instr_t *i = malloc(sizeof(instr_t));
  i->kind           = INSTR_OPER;
  i->assem          = assem;
  i->oper.dst       = NULL;
  i->oper.dst_num   = 0;
  i->oper.src       = NULL;
  i->oper.src_num   = 0;
  i->oper.jumps     = jumps;
  i->oper.jumps_num = njumps;
  i->next = NULL;
  return i;
}

static temp_t munch_exp(tree_expr_t *e) {
  switch (e->kind) {
    case TREE_CONST: {
      temp_t t = temp_new();
      char *assem = malloc(32);
      sprintf(assem, "movq $%d, `d0", e->const_);
      temp_t *dst = malloc(sizeof(temp_t));
      dst[0] = t;
      emit(make_oper(assem, dst, 1, NULL, 0));
      return t;
    }
    case TREE_TEMP: {
      return e->temp;
    }
    case TREE_MEM: {
      temp_t addr = munch_exp(e->mem);
      temp_t t = temp_new();
      temp_t *dst = malloc(sizeof(temp_t)); dst[0] = t;
      temp_t *src = malloc(sizeof(temp_t)); src[0] = addr;
      emit(make_oper("movq (`s0), `d0", dst, 1, src, 1));
      return t;
    }
    case TREE_BINOP: {
      temp_t l = munch_exp(e->binop.e1);
      temp_t r = munch_exp(e->binop.e2);
      temp_t t = temp_new();

      char *assem = "movq `s0, `d0";
      emit(make_move(assem, t, l));
      temp_t *dst = malloc(sizeof(temp_t));
      temp_t *src = malloc(sizeof(temp_t));
      dst[0] = t;
      src[0] = r;
      switch (e->binop.op) {
        case TREE_ADD:
          assem = "addq `s0, `d0";
          break;
        case TREE_SUB:
          assem = "subq `s0, `d0";
          break;
        case TREE_MUL:
          assem = "imulq `s0, `d0";
          break;
        default: assem = "addq `s0, `d0";
      }
      emit(make_oper(assem, dst, 1, src, 1));
      return t;
    }
    case TREE_CALL: {
      temp_t *src = malloc(e->call.num_actuals * sizeof(temp_t));
      temp_t *dst = malloc(sizeof(temp_t));
      for (int i = 0; i < e->call.num_actuals; i++) {
        temp_t t = munch_exp(e->call.actuals[i]);
        emit(make_move("movq `s0, `d0", frame_arg_reg(i), t));
        src[i] = frame_arg_reg(i);
      }
      dst[0] = frame_rv();
      char *assem = malloc(64);
      sprintf(assem, "callq %s", label_name(e->call.name->name));

      emit(make_oper(assem, dst, 1, src, e->call.num_actuals));

      return temp_new();
    }
    default: {
      temp_t t = temp_new();
      return t;
    }
  }
}

static void munch_stm(tree_stmt_t *s) {
  switch (s->kind) {
    case TREE_LABEL: {
      emit(make_label(label_name(s->label), s->label));
      return;
    }
    case TREE_MOVE: {
      if (s->move.d->kind == TREE_MEM) {
        temp_t addr = munch_exp(s->move.d->mem);
        temp_t src  = munch_exp(s->move.s);
        temp_t *srcs = malloc(2 * sizeof(temp_t));
        srcs[0] = src; srcs[1] = addr;
        emit(make_oper("movq `s0, (`s1)", NULL, 0, srcs, 2));
        return;
      }
      temp_t src = munch_exp(s->move.s);
      emit(make_move("movq `s0, `d0", s->move.d->temp, src));
      return;
    }
    case TREE_EXP: {
      munch_exp(s->exp);
      return;
    }
    case TREE_JUMP: {
      label_t *jump_ = malloc(sizeof(label_t));
      jump_[0] = s->jump_.dests[0];
      emit(make_jump("jmp `j0", jump_, 1));
      return;
    }
    case TREE_CJUMP: {
      temp_t l = munch_exp(s->cjump.e1);
      temp_t r = munch_exp(s->cjump.e2);
      temp_t *src = malloc(2 * sizeof(temp_t));
      src[0] = l; src[1] = r;
      emit(make_oper("cmpq `s1, `s0", NULL, 0, src, 2));

      const char *jmp;
      switch (s->cjump.op) {
        case TREE_EQ: jmp = "je `j0";  break;
        case TREE_NE: jmp = "jne `j0"; break;
        case TREE_LT: jmp = "jl `j0";  break;
        case TREE_LE: jmp = "jle `j0"; break;
        case TREE_GT: jmp = "jg `j0";  break;
        case TREE_GE: jmp = "jge `j0"; break;
        default:      jmp = "je `j0";  break;
      }
      label_t *jumps = malloc(sizeof(label_t));
      jumps[0] = s->cjump.true_;
      emit(make_jump((char *)jmp, jumps, 1));
      return;
    }
    default: return;
  }
}

instr_t *codegen(frame_t *frame, stmt_list_t *stmts) {
  ilist = ilast = NULL;
  for (stmt_list_t *sl = stmts; sl; sl = sl->next)
    munch_stm(sl->stmt);
  return ilist;
}

void print_instrs(instr_t *instrs) {
  for (instr_t *i = instrs; i; i = i->next) {
    if (i->kind == INSTR_LABEL) {
      printf("%s:\n", i->assem);
      continue;
    }
    /* substitute `d, `s, `j placeholders */
    printf("  ");
    for (const char *p = i->assem; *p; p++) {
      if (*p == '`' && *(p+1)) {
        char kind = *(p+1);
        int  idx  = *(p+2) - '0';
        if (kind == 'd' && i->kind == INSTR_MOVE)
          printf("t%d", i->move.dst);
        else if (kind == 's' && i->kind == INSTR_MOVE)
          printf("t%d", i->move.src);
        else if (kind == 'd' && idx < i->oper.dst_num)
          printf("t%d", i->oper.dst[idx]);
        else if (kind == 's' && idx < i->oper.src_num)
          printf("t%d", i->oper.src[idx]);
        else if (kind == 'j' && idx < i->oper.jumps_num)
          printf("%s", label_name(i->oper.jumps[idx]));
        p += 2;
      } else {
        putchar(*p);
      }
    }
    printf("\n");
  }
}
