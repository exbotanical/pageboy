/**
 * @file vm.c
 * @author goldmund
 * @brief virtual machine - executes prepared statements
 * @version 0.1
 * @date 2021-07-05
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "vm.h"

#include "common.h"
#include "repl.h"
#include "statement.h"
#include "pager.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Process a meta command
 *
 * @param ib
 * @return MetaCommandResult
 */
MetaCommandResult proc_meta_cmd(InputBuffer* ib, Table* table) {
	if (strcmp(ib->buffer, ".exit") == 0) {
		db_close(table);
		exit(EXIT_SUCCESS);
	} else return META_E_UNRECOGNIZED_CMD;
}

/**
 * @brief Prepare an insert statement
 *
 * @param ib
 * @param stmt
 * @return PrepareResult
 */
PrepareResult prepare_insert(InputBuffer* ib, Statement* stmt) {
	stmt->type = STATEMENT_INSERT;

	char* keyword = strtok(ib->buffer, " ");
	char* id_str = strtok(NULL, " ");
	char* uname = strtok(NULL, " ");
	char* email = strtok(NULL, " ");

	if (!id_str || !uname || !email) return PREPARE_E_SYNTAX;

	int id = atoi(id_str);

	if (strlen(uname) > COL_UNAME_SIZE) return PREPARE_E_MAX_CH;

	stmt->row.id = id;

	strcpy(stmt->row.uname, uname);
	strcpy(stmt->row.email, email);

	return PREPARE_SUCCESS;
}

/**
 * @brief Given a statement keyword, create a prepared statement
 *
 * @param ib input buffer
 * @param stmt pointer to a pre-prepared statement
 * @return PrepareResult
 */
PrepareResult prepare_statement(InputBuffer* ib, Statement* stmt) {
	if (strncmp(ib->buffer, "insert", 6) == 0) {
		return prepare_insert(ib, stmt);
	}

	if (strcmp(ib->buffer, "select") == 0) {
		stmt->type = STATEMENT_SELECT;

		return PREPARE_SUCCESS;
	}

	return PREPARE_E_UNRECOGNIZED_STMT;
}

/**
 * @brief Execute a prepared statement
 *
 * @param stmt pointer to a prepared statemen
 * @param table
 * @return ExecuteStatementResult
 */
ExecuteStatementResult exec_statement(Statement* stmt, Table* table) {
	switch (stmt->type) {
		case STATEMENT_INSERT:
			return execute_insert(stmt, table);
		case STATEMENT_SELECT:
			return execute_select(stmt, table);
	}

	return EXEC_SUCCESS; // todo
}
