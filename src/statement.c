#include "statement.h"

#include "serialize.h"

#include <stdint.h>
#include <stdio.h>

/**
 * @brief Execute an INSERT statement
 *
 * @param stmt
 * @param table
 */
ExecuteStatementResult execute_insert(Statement* stmt, Table* table) {
	if (table->rows_n >= TABLE_MAX_ROWS) {
		return EXEC_E_TABLE_CAP;
	}

	Row* row = &(stmt->row);
	serialize_row(row, row_slot(table, table->rows_n));
	table->rows_n++;

	return EXEC_SUCCESS;
}

ExecuteStatementResult execute_select(Statement* stmt, Table* table) {
	Row row;

	for (uint32_t i = 0; i < table->rows_n; i++) {
		deserialize_row(row_slot(table, i), &row);
		printf("(%d, %s, %s)\n", row.id, row.uname, row.email);
	}

	return EXEC_SUCCESS;
}
