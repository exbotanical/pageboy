#include "metacommand.h"

#include <string.h>

MetaCommandResult process_meta_command(StringBuffer* buffer, Table* table) {
  if (strcmp(buffer->buffer, ".exit") == 0) {
    db_close(table);
    exit(EXIT_SUCCESS);
  } else
    return META_COMMAND_UNRECOGNIZED;
}
