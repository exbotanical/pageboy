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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Process a meta command
 *
 * @param ib
 * @return MetaCommandResult
 */
MetaCommandResult proc_meta_cmd(InputBuffer *ib) {
	if (strcmp(ib->buffer, ".exit") == 0) {
		exit(EXIT_SUCCESS);
	} else return META_CMD_UNRECOGNIZED_CMD;
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
		stmt->type = STATEMENT_INSERT;

		return PREPARE_SUCCESS;
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
 * @param stmt pointer to a prepared statement
 */
void exec_statement(Statement* stmt) {
	switch (stmt->type) {
		case STATEMENT_INSERT:
			printf("insert\n");
			break;
		case STATEMENT_SELECT:
			printf("select\n");
			break;
	}
}
