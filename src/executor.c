#include "executor.h"

#include <stdint.h>
#include <stdio.h>

#include "pager.h"

ExecutionResult execute_insert(Statement* statement, Table* table) {
  void* node = get_page(table->pager, table->root_page_num);

  if ((*leaf_node_num_cells(node) >= LEAF_NODE_MAX_CELLS)) {
    return EXECUTE_TABLE_FULL;
  }

  Row* row = &(statement->row);
  Cursor* cursor = cursor_end_init(table);

  leaf_node_insert(cursor, row->id, row);

  free(cursor);
  return EXECUTE_SUCCESS;
}

ExecutionResult execute_select(Statement* statement, Table* table) {
  Row row;
  Cursor* cursor = cursor_start_init(table);
  while (!(cursor->end)) {
    deserialize_row(cursor_value(cursor), &row);
    printf("(%d, %s, %s)\n", row.id, row.username, row.email);
    cursor_advance(cursor);
  }

  free(cursor);
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
