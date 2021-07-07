#ifndef STATEMENT_H
#define STATEMENT_H

#include "vm.h"

ExecuteStatementResult execute_insert(Statement* stmt, Table* table);

ExecuteStatementResult execute_select(Statement *stmt, Table *table);

#endif
