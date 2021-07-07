#ifndef VM_H
#define VM_H

#include "repl.h"
#include "pager.h"

/* Enumerations */

/**
 * @brief Meta Command execution result
 */
typedef enum {
	META_SUCCESS, /**< Meta Cmd was successful */
	META_E_UNRECOGNIZED_CMD, /**< Meta Cmd, unrecognized directive */
} MetaCommandResult;

/**
 * @brief Statement preparation result type
 */
typedef enum {
	PREPARE_SUCCESS, /**< Successful preparation */
	PREPARE_E_UNRECOGNIZED_STMT, /**< Unrecognized preparative statement */
	PREPARE_E_SYNTAX, /**< Syntax error */
	PREPARE_E_MAX_CH, /**< Max characters exceeded */
} PrepareResult;

/**
 * @brief Statement type keywords
 */
typedef enum {
	STATEMENT_INSERT,
	STATEMENT_SELECT,
} StatementType;

/**
 * @brief Statement execution return code
 */
typedef enum {
	EXEC_SUCCESS,
	EXEC_E_TABLE_CAP,
} ExecuteStatementResult;

/* Structures */

/**
 * @brief Represents a single statement
 */
typedef struct {
	StatementType type;
	Row row;
} Statement;

/* Functions */

MetaCommandResult proc_meta_cmd(InputBuffer* ib, Table* table);

PrepareResult prepare_statement(InputBuffer* ib, Statement* stmt);

ExecuteStatementResult exec_statement(Statement* stmt, Table* table);

PrepareResult prepare_insert(InputBuffer* ib, Statement* stmt);

#endif
