#ifndef TEMP_H
#define TEMP_H

typedef int temp_t;
typedef int label_t;

temp_t  temp_new();
label_t label_new();
label_t label_named(char *name);
char   *label_name(label_t l);

#endif
