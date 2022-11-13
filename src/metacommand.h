#ifndef METACOMMAND_H
#define METACOMMAND_H

#include "io.h"
#include "pager.h"

typedef enum {
  META_COMMAND_SUCCESS,
  META_COMMAND_UNRECOGNIZED,
} MetaCommandResult;

MetaCommandResult process_meta_command(StringBuffer* ib, Table* table);

#endif /* METACOMMAND_H */
