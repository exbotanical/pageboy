#include "metacommand.h"

#include <stdio.h>
#include <string.h>

MetaCommandResult process_meta_command(StringBuffer* buffer, Table* table) {
  if (strcmp(buffer->buffer, ".exit") == 0) {
    db_close(table);
    exit(EXIT_SUCCESS);
  }

  if (strcmp(buffer->buffer, ".btree") == 0) {
    printf("TODO\n");
    return META_COMMAND_SUCCESS;
  }

  if (strcmp(buffer->buffer, ".settings") == 0) {
    printf("TODO\n");
    return META_COMMAND_SUCCESS;
  }

  return META_COMMAND_UNRECOGNIZED;
}
