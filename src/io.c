#define _GNU_SOURCE

#include "io.h"

#include <stdio.h>
#include <stdlib.h>

#include "common.h"

StringBuffer* string_buffer_init(void) {
  StringBuffer* buffer = malloc(sizeof(StringBuffer));
  buffer->buffer = NULL;
  buffer->len = 0;
  buffer->input_l = 0;

  return buffer;
}

void string_buffer_destroy(StringBuffer* buffer) {
  free(buffer->buffer);
  free(buffer);
}

void string_buffer_read(StringBuffer* buffer) {
  ssize_t bytes;

  if ((bytes = getline(&(buffer->buffer), &(buffer->len), stdin)) <= 0) {
    DIE("%s\n", "Error reading input");
  }

  buffer->input_l = bytes - 1;
  buffer->buffer[bytes - 1] = '\0';
}

void print_prompt(void) { printf("%s > ", APP_NAME); }
