#ifndef PAGER_H
#define PAGER_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "common.h"

#define sizeof_attr(Struct, Attr) sizeof(((Struct*)0)->Attr)

#define TABLE_MAX_PAGES 100

/**
 * @brief Node type identifier, where a node corresponds to one page.
 * Internal nodes point to their children by storing the page number in which
 * the child is stored.
 */
typedef enum {
  NODE_INTERNAL,
  NODE_LEAF,
} NodeType;

typedef struct {
  int fd;
  uint32_t file_len;
  uint32_t num_pages;
  void* pages[TABLE_MAX_PAGES];
} Pager;

typedef struct {
  uint32_t root_page_num;
  Pager* pager;
} Table;

typedef struct {
  uint32_t id;
  char username[COLUMN_USERNAME_SIZE + 1];
  char email[COLUMN_EMAIL_SIZE + 1];
} Row;

typedef struct {
  Table* table;
  uint32_t page_num;
  uint32_t cell_num;
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

static const uint32_t NODE_TYPE_SIZE = sizeof(uint32_t);  // uint8?
static const uint32_t NODE_TYPE_OFFSET = 0;
static const uint32_t IS_ROOT_SIZE = sizeof(uint32_t);  // uint8?
static const uint32_t IS_ROOT_OFFSET = NODE_TYPE_SIZE;
static const uint32_t PARENT_POINTER_SIZE = sizeof(uint32_t);
static const uint32_t PARENT_POINTER_OFFSET = IS_ROOT_OFFSET + IS_ROOT_SIZE;
static const uint8_t COMMON_NODE_HEADER_SIZE =
    NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE;

static const uint32_t LEAF_NODE_NUM_CELLS_SIZE = sizeof(uint32_t);
static const uint32_t LEAF_NODE_NUM_CELLS_OFFSET = COMMON_NODE_HEADER_SIZE;
static const uint32_t LEAF_NODE_HEADER_SIZE =
    COMMON_NODE_HEADER_SIZE + LEAF_NODE_NUM_CELLS_SIZE;

static const uint32_t LEAF_NODE_KEY_SIZE = sizeof(uint32_t);
static const uint32_t LEAF_NODE_KEY_OFFSET = 0;
static const uint32_t LEAF_NODE_VALUE_SIZE = ROW_SIZE;
static const uint32_t LEAF_NODE_VALUE_OFFSET =
    LEAF_NODE_KEY_OFFSET + LEAF_NODE_KEY_SIZE;
static const uint32_t LEAF_NODE_CELL_SIZE =
    LEAF_NODE_KEY_SIZE + LEAF_NODE_VALUE_SIZE;
static const uint32_t LEAF_NODE_SPACE_FOR_CELLS =
    PAGE_SIZE + LEAF_NODE_HEADER_SIZE;
static const uint32_t LEAF_NODE_MAX_CELLS =
    LEAF_NODE_SPACE_FOR_CELLS / LEAF_NODE_CELL_SIZE;

Table* db_open(const char* filename);

void db_close(Table* table);

Pager* pager_open(const char* filename);

void pager_flush(Pager* pager, uint32_t page_num);

void* get_page(Pager* pager, uint32_t page_num);

void serialize_row(Row* src, void* dest);

void deserialize_row(void* src, Row* dest);

void* cursor_value(Cursor* cursor);

Cursor* cursor_start_init(Table* table);

Cursor* cursor_end_init(Table* table);

void cursor_advance(Cursor* cursor);

void leaf_node_insert(Cursor* cursor, uint32_t key, Row* value);

uint32_t* leaf_node_num_cells(void* node);

void* leaf_node_value(void* node, uint32_t cell_num);

#endif /* PAGER_H */
