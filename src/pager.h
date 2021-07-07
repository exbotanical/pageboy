#ifndef TABLE_H
#define TABLE_H

#include <stdlib.h>
#include <stdint.h>

#define sizeof_attr(Struct, Attr) sizeof(((Struct*)0)->Attr)

#define TABLE_MAX_PAGES 100

#define COL_UNAME_SIZE 32
#define COL_EM_SIZE 255

/**
 * @brief Interface for handling pages; the Table object
 * makes requests for pages via the Pager
 */
typedef struct {
	int file_descriptor;
	uint32_t file_s;
	void* pages[TABLE_MAX_PAGES];
} Pager;

typedef struct {
	uint32_t rows_n;
	Pager* pager;
} Table;

typedef struct {
	uint32_t id;
	char uname[COL_UNAME_SIZE - 1];
	char email[COL_EM_SIZE - 1];
} Row;

static const uint32_t ID_SIZE = sizeof_attr(Row, id);
static const uint32_t UNAME_SIZE = sizeof_attr(Row, uname);
static const uint32_t EMAIL_SIZE = sizeof_attr(Row, email);

static const uint32_t ID_OFFSET = 0;
static const uint32_t UNAME_OFFSET = ID_OFFSET + ID_SIZE;
static const uint32_t EMAIL_OFFSET = UNAME_OFFSET + UNAME_SIZE;
static const uint32_t ROW_SIZE = ID_SIZE + UNAME_SIZE + EMAIL_SIZE;

static const uint32_t PAGE_SIZE = 4096;
static const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
static const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

Table* db_open(const char* filename);

void db_close(Table* table);

void* row_slot(Table* table, uint32_t row_n);

Pager* pager_open(const char* filename);

void* get_page(Pager* pager, uint32_t page_n);

void pager_flush(Pager* pager, uint32_t page_n, uint32_t size);

#endif
