#ifndef STATEMENT_H
#define STATEMENT_H

#include "pager.h"

typedef enum {
  STATEMENT_INSERT,
  STATEMENT_SELECT,
} StatementType;

typedef struct {
  StatementType type;
  Row row;
} Statement;

#endif
