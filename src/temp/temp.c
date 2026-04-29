#include <stdio.h>
#include <stdlib.h>
#include "temp.h"

typedef struct {
  char **names;
  int    cap;
} ltab_t;

static int temp_count = 0;
static int label_count = 0;
static ltab_t ltab = {NULL, 0};

temp_t  temp_new() {
  return ++temp_count;
}
label_t label_new() {
  return ++label_count;
}

label_t label_named(char *name) {
  if (ltab.cap > label_count) {
    ltab.names[label_count] = name;
    return label_count++;
  }
  int nsize = (ltab.cap == 0 ? 2 : ltab.cap * 2);
  void *temp = realloc(ltab.names,  nsize * sizeof(char *));
  if (!temp) {
    fprintf(stderr, "temp: reallocation failed.\n");
    exit(EXIT_FAILURE);
  }
  ltab.cap = nsize;
  ltab.names = temp;
  ltab.names[label_count] = name;
  return label_count++;
}

char   *label_name(label_t l) {
  return ltab.names[l];
}
