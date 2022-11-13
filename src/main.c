#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "executor.h"
#include "io.h"
#include "metacommand.h"
#include "preparator.h"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    DIE("%s\n", "Must provide a database filename");
  }

  char* filename = argv[1];
  Table* table = db_open(filename);

  StringBuffer* buffer = string_buffer_init();

  while (1) {
    print_prompt();
    string_buffer_read(buffer);

    if (buffer->buffer[0] == '.') {
      switch (process_meta_command(buffer, table)) {
        case META_COMMAND_SUCCESS:
          continue;
        case META_COMMAND_UNRECOGNIZED:
          fprintf(stderr, "Unrecognized command '%s'\n", buffer->buffer);
          continue;
      }
    }

    Statement statement;

    switch (prepare_statement(buffer, &statement)) {
      case PREPARE_SUCCESS:
        break;
      case PREPARE_SYNTAX_ERROR:
        fprintf(stderr, "%s\n", "Syntax error. Could not parse statement");
        continue;
      case PREPARE_UNRECOGNIZED_STATEMENT:
        fprintf(stderr, "Unrecognized keyword at start of '%s'\n",
                buffer->buffer);
        continue;
      case PREPARE_INPUT_TOO_LONG:
        fprintf(stderr, "%s\n", "Provided input was too long");
        continue;
      case PREPARE_NEGATIVE_ID:
        fprintf(stderr, "%s\n", "Provided negative id");
        continue;
      default:
        fprintf(stderr, "%s\n",
                "[main::PrepareStatement] An error occurred (TODO:)");
    }

    switch (execute_statement(&statement, table)) {
      case EXECUTE_SUCCESS:
        fprintf(stdout, "%s\n", "Executed statement");

        break;

      case EXECUTE_TABLE_FULL:
        fprintf(stderr, "%s\n", "Table memory full");

        break;
    }
  }

  return EXIT_SUCCESS;
}
