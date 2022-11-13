#ifndef PAGER_H
#define PAGER_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "common.h"

#define sizeof_attr(Struct, Attr) sizeof(((Struct*)0)->Attr)

#define TABLE_MAX_PAGES 100

typedef struct {
  int fd;
  uint32_t file_s;
  void* pages[TABLE_MAX_PAGES];
} Pager;

typedef struct {
  uint32_t num_rows;
  Pager* pager;
} Table;

typedef struct {
  uint32_t id;
  char username[COLUMN_USERNAME_SIZE + 1];
  char email[COLUMN_EMAIL_SIZE + 1];
} Row;

typedef struct {
  Table* table;
  uint32_t row_num;
  bool end;  // where end is 1 position past the last element
} Cursor;

static const uint32_t ID_SIZE = sizeof_attr(Row, id);
static const uint32_t USERNAME_SIZE = sizeof_attr(Row, username);
static const uint32_t EMAIL_SIZE = sizeof_attr(Row, email);

static const uint32_t ID_OFFSET = 0;
static const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
static const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
static const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

static const uint32_t PAGE_SIZE = 4096;
static const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
static const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

Table* db_open(const char* filename);

void db_close(Table* table);

Pager* pager_open(const char* filename);

void pager_flush(Pager* pager, uint32_t page_num, uint32_t size);

void* get_page(Pager* pager, uint32_t page_num);

void serialize_row(Row* src, void* dest);

void deserialize_row(void* src, Row* dest);

void* cursor_value(Cursor* cursor);

Cursor* cursor_start_init(Table* table);

Cursor* cursor_end_init(Table* table);

void cursor_advance(Cursor* cursor);

#endif /* PAGER_H */
