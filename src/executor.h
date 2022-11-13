#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "pager.h"
#include "statement.h"

typedef enum {
  EXECUTE_SUCCESS,
  EXECUTE_TABLE_FULL,
} ExecutionResult;

ExecutionResult execute_insert(Statement* statement, Table* table);

ExecutionResult execute_select(Statement* statement, Table* table);

ExecutionResult execute_statement(Statement* statement, Table* table);

#endif /* EXECUTOR_H */
