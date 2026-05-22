#ifndef CODEGEN_H
#define CODEGEN_H
#include "temp.h"
#include "frame.h"
#include "canon.h"

typedef enum {
  INSTR_OPER,
  INSTR_MOVE,
  INSTR_LABEL,
} instr_kind_t;

typedef struct instr_t {
  instr_kind_t    kind;
  char           *assem;
  union {
    struct {
      temp_t  *dst;   int dst_num;
      temp_t  *src;   int src_num;
      label_t *jumps; int jumps_num;
    } oper;
    struct {
      temp_t dst;
      temp_t src;
    } move;
    struct {
      label_t label;
    } label;
  };
  struct instr_t *next;
} instr_t;

instr_t *codegen(frame_t *frame, stmt_list_t *stmts);
void     print_instrs(instr_t *instrs);

#endif
