#include "preparator.h"

#include <string.h>

#include "common.h"

PrepareResult prepare_insert(StringBuffer* buffer, Statement* statement) {
  statement->type = STATEMENT_INSERT;

  char* keyword = strtok(buffer->buffer, " ");

  char* id_str = strtok(NULL, " ");
  if (id_str == NULL) {
    return PREPARE_SYNTAX_ERROR;
  }

  char* username = strtok(NULL, " ");
  if (username == NULL) {
    return PREPARE_SYNTAX_ERROR;
  }

  if (strlen(username) > COLUMN_USERNAME_SIZE) {
    return PREPARE_INPUT_TOO_LONG;
  }

  char* email = strtok(NULL, " ");
  if (email == NULL) {
    return PREPARE_SYNTAX_ERROR;
  }

  if (strlen(email) > COLUMN_EMAIL_SIZE) {
    return PREPARE_INPUT_TOO_LONG;
  }

  int id = atoi(id_str);
  if (id < 0) {
    return PREPARE_NEGATIVE_ID;
  }

  statement->row.id = id;
  strcpy(statement->row.username, username);
  strcpy(statement->row.email, email);

  return PREPARE_SUCCESS;
}

PrepareResult prepare_statement(StringBuffer* buffer, Statement* statement) {
  if (strncmp(buffer->buffer, "insert", 6) == 0) {
    return prepare_insert(buffer, statement);
  }

  if (strcmp(buffer->buffer, "select") == 0) {
    statement->type = STATEMENT_SELECT;

    return PREPARE_SUCCESS;
  }

  return PREPARE_UNRECOGNIZED_STATEMENT;
}
