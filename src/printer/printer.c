#include <stdio.h>
#include "printer.h"
#include "ast.h"

static void print_dec(dec_t *dec, int indent);
static void print_ty(ty_t *ty, int indent);

static void print_indent(int indent) {
  for (int i = 0; i < indent; i++) printf("  ");
}

static const char *binop_str(binop_t op) {
  switch (op) {
    case OP_ADD: return "+";
    case OP_SUB: return "-";
    case OP_MUL: return "*";
    case OP_DIV: return "/";
    case OP_EQ:  return "=";
    case OP_NEQ: return "<>";
    case OP_LT:  return "<";
    case OP_LE:  return "<=";
    case OP_GT:  return ">";
    case OP_GE:  return ">=";
    case OP_AND: return "&";
    case OP_OR:  return "|";
    default:     return "?";
  }
}

void print_expr(expr_t *expr, int indent) {
  if (!expr) {
    print_indent(indent);
    printf("(null)\n");
    return;
  }

  switch (expr->kind) {
    case EXPR_INT:
      print_indent(indent);
      printf("INT %d\n", expr->int_val);
      break;

    case EXPR_STRING:
      print_indent(indent);
      printf("STRING \"%s\"\n", expr->str_val);
      break;

    case EXPR_NIL:
      print_indent(indent);
      printf("NIL\n");
      break;

    case EXPR_ID:
      print_indent(indent);
      printf("ID %s\n", expr->id);
      break;

    case EXPR_INDEX:
      print_indent(indent);
      printf("INDEX\n");
      print_expr(expr->index_.array, indent + 1);
      print_expr(expr->index_.index, indent + 1);
      break;

    case EXPR_FIELD:
      print_indent(indent);
      printf("FIELD .%s\n", expr->field_.field);
      print_expr(expr->field_.record, indent + 1);
      break;

    case EXPR_BINOP:
      print_indent(indent);
      printf("BINOP %s\n", binop_str(expr->binop.op));
      print_expr(expr->binop.left, indent + 1);
      print_expr(expr->binop.right, indent + 1);
      break;

    case EXPR_ASSIGN:
      print_indent(indent);
      printf("ASSIGN %s :=\n", expr->assign.var);
      print_expr(expr->assign.rhs, indent + 1);
      break;

    case EXPR_IF:
      print_indent(indent);
      printf("IF\n");
      print_indent(indent + 1);
      printf("COND\n");
      print_expr(expr->if_.cond, indent + 2);
      print_indent(indent + 1);
      printf("THEN\n");
      print_expr(expr->if_.then, indent + 2);
      if (expr->if_.else_) {
        print_indent(indent + 1);
        printf("ELSE\n");
        print_expr(expr->if_.else_, indent + 2);
      }
      break;

    case EXPR_WHILE:
      print_indent(indent);
      printf("WHILE\n");
      print_indent(indent + 1);
      printf("COND\n");
      print_expr(expr->while_.cond, indent + 2);
      print_indent(indent + 1);
      printf("BODY\n");
      print_expr(expr->while_.body, indent + 2);
      break;

    case EXPR_FOR:
      print_indent(indent);
      printf("FOR %s\n", expr->for_.var);
      print_indent(indent + 1);
      printf("FROM\n");
      print_expr(expr->for_.init, indent + 2);
      print_indent(indent + 1);
      printf("TO\n");
      print_expr(expr->for_.to, indent + 2);
      print_indent(indent + 1);
      printf("DO\n");
      print_expr(expr->for_.body, indent + 2);
      break;

    case EXPR_LET:
      print_indent(indent);
      printf("LET\n");
      print_indent(indent + 1);
      printf("DECS\n");
      for (dec_list_t *dl = expr->let.dec_list; dl; dl = dl->next)
        print_dec(dl->dec, indent + 2);
      print_indent(indent + 1);
      printf("IN\n");
      for (expr_list_t *el = expr->let.body; el; el = el->next)
        print_expr(el->expr, indent + 2);
      break;

    case EXPR_CALL:
      print_indent(indent);
      printf("CALL %s\n", expr->call.id);
      for (expr_list_t *el = expr->call.arg_list; el; el = el->next)
        print_expr(el->expr, indent + 1);
      break;

    case EXPR_SEQ:
      print_indent(indent);
      printf("SEQ\n");
      for (expr_list_t *el = expr->seq; el; el = el->next)
        print_expr(el->expr, indent + 1);
      break;

    case EXPR_RECORD:
      print_indent(indent);
      printf("RECORD %s\n", expr->record.type_name);
      for (field_list_t *fl = expr->record.fields; fl; fl = fl->next) {
        print_indent(indent + 1);
        printf("%s =\n", fl->name);
        print_expr(fl->val, indent + 2);
      }
      break;

    case EXPR_ARRAY:
      print_indent(indent);
      printf("ARRAY %s\n", expr->array.type_name);
      print_indent(indent + 1);
      printf("SIZE\n");
      print_expr(expr->array.size, indent + 2);
      print_indent(indent + 1);
      printf("INIT\n");
      print_expr(expr->array.init, indent + 2);
      break;
  }
}

static void print_ty(ty_t *ty, int indent) {
  if (!ty) return;
  print_indent(indent);
  switch (ty->kind) {
    case TY_NAME:
      printf("NAME %s\n", ty->alias);
      break;
    case TY_ARRAY:
      printf("ARRAY OF %s\n", ty->array_of);
      break;
    case TY_RECORD:
      printf("RECORD\n");
      for (param_list_t *pl = ty->fields; pl; pl = pl->next) {
        print_indent(indent + 1);
        printf("%s : %s\n", pl->param->name, pl->param->type_name);
      }
      break;
  }
}

static void print_dec(dec_t *dec, int indent) {
  if (!dec) return;
  switch (dec->kind) {
    case DEC_VAR:
      print_indent(indent);
      if (dec->var.type_name)
        printf("VAR %s : %s :=\n", dec->var.id, dec->var.type_name);
      else
        printf("VAR %s :=\n", dec->var.id);
      print_expr(dec->var.init, indent + 1);
      break;

    case DEC_FUNC:
      print_indent(indent);
      printf("FUNC %s(", dec->func.id);
      for (param_list_t *pl = dec->func.args; pl; pl = pl->next) {
        printf("%s : %s", pl->param->name, pl->param->type_name);
        if (pl->next) printf(", ");
      }
      printf(")");
      if (dec->func.type_name)
        printf(" : %s", dec->func.type_name);
      printf("\n");
      print_expr(dec->func.body, indent + 1);
      break;

    case DEC_TYPE:
      print_indent(indent);
      printf("TYPE %s =\n", dec->type.name);
      print_ty(dec->type.ty, indent + 1);
      break;
  }
}
