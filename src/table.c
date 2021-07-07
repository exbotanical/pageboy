#include "table.h"

/**
 * @brief Initialize and allocate memory for a new table
 *
 * @return Table*
 */
Table* table_alloc(void) {
  Table* table = malloc(sizeof(Table));
	table->rows_n = 0;

	for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
		table->pages[i] = NULL;
	}

	return table;
}

/**
 * @brief Deallocate memory utilized by given table
 *
 * @param table
 */
void table_free(Table* table) {
	for (int i = 0; table->pages[i]; i++) {
		free(table->pages[i]);
	}

	free(table);
}

/**
 * @brief Read / Write in memory for a given row
 *
 * @param table
 * @param row_n
 * @return void*
 */
void* row_slot(Table* table, uint32_t row_n) {
	uint32_t page_n = row_n / ROWS_PER_PAGE;
	void* page = table->pages[page_n];

	if (!page) {
		// allocate memory only when we attempt to access the page
		page = table->pages[page_n] = malloc(PAGE_SIZE);
	}

	uint32_t row_offset = row_n % ROWS_PER_PAGE;
	uint32_t byte_offset = row_offset * ROW_SIZE;

	return page + byte_offset;
}
