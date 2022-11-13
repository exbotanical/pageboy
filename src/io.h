#ifndef REPL_H
#define REPL_H

#include <unistd.h>

typedef struct {
  char *buffer;
  size_t len;
  ssize_t input_l;
} StringBuffer;

StringBuffer *string_buffer_init(void);

void string_buffer_destroy(StringBuffer *ib);

void string_buffer_read(StringBuffer *ib);

void print_prompt(void);

#endif
