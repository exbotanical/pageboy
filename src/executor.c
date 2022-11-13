#include "executor.h"

#include <stdint.h>
#include <stdio.h>

#include "pager.h"

ExecutionResult execute_insert(Statement* statement, Table* table) {
  if (table->num_rows >= TABLE_MAX_ROWS) {
    return EXECUTE_TABLE_FULL;
  }

  Row* row = &(statement->row);
  serialize_row(row, row_slot(table, table->num_rows));
  table->num_rows++;

  return EXECUTE_SUCCESS;
}

ExecutionResult execute_select(Statement* statement, Table* table) {
  Row row;

  for (uint32_t i = 0; i < table->num_rows; i++) {
    deserialize_row(row_slot(table, i), &row);
    printf("(%d, %s, %s)\n", row.id, row.username, row.email);
  }

  return EXECUTE_SUCCESS;
}

ExecutionResult execute_statement(Statement* statement, Table* table) {
  switch (statement->type) {
    case STATEMENT_INSERT:
      return execute_insert(statement, table);
    case STATEMENT_SELECT:
      return execute_select(statement, table);
  }

  return EXECUTE_SUCCESS;  // todo
}
