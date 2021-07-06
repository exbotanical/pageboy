#ifndef VM_H
#define VM_H

#include "repl.h"

/* Enumerations */

/**
 * @brief Meta Command execution result
 */
typedef enum {
	META_CMD_SUCCESS, /**< Meta Cmd was successful */
	META_CMD_UNRECOGNIZED_CMD, /**< Meta Cmd, unrecognized directive */
} MetaCommandResult;

/**
 * @brief Statement preparation result type
 */
typedef enum {
	PREPARE_SUCCESS, /**< Successful preparation */
	PREPARE_E_UNRECOGNIZED_STMT, /**< Unrecognized preparative statement */
	PREPARE_E_SYNTAX, /**< Syntax error */
} PrepareResult;

/**
 * @brief Statement type keywords
 */
typedef enum {
	STATEMENT_INSERT,
	STATEMENT_SELECT,
} StatementType;

/* Structures */

/**
 * @brief Represents a single statement
 */
typedef struct {
	StatementType type;
} Statement;

/* Functions */

MetaCommandResult proc_meta_cmd(InputBuffer *ib);

PrepareResult prepare_statement(InputBuffer *ib, Statement *stmt);

void exec_statement(Statement *stmt);

#endif
