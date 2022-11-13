#ifndef PREPARATOR_H
#define PREPARATOR_H

#include "io.h"
#include "statement.h"

typedef enum {
  PREPARE_SUCCESS,
  PREPARE_UNRECOGNIZED_STATEMENT,
  PREPARE_SYNTAX_ERROR,
  PREPARE_INPUT_TOO_LONG,
  PREPARE_NEGATIVE_ID,
} PrepareResult;

PrepareResult prepare_statement(StringBuffer* ib, Statement* statement);

PrepareResult prepare_insert(StringBuffer* ib, Statement* statement);

#endif /* PREPARATOR_H */
