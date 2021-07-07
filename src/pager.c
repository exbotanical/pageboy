#include "pager.h"

#include "common.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

/**
 * @brief Initialize and allocate a table object; in turn initializes a Pager
 *
 * @return Table*
 */
Table* db_open(const char* filename) {
	Pager *pager = pager_open(filename);
	uint32_t rows_n = pager->file_s / ROW_SIZE;

	Table* table = malloc(sizeof(Table));

	table->pager = pager;
	table->rows_n = rows_n;

	return table;
}

/**
 * @brief Flush page cache to disk, close db file, and free Pager, Table memory
 *
 * @param table
 */
void db_close(Table* table) {
	Pager* pager = table->pager;
	uint32_t full_pages_n = table->rows_n / ROWS_PER_PAGE;

	for (uint32_t i = 0; i < full_pages_n; i++) {
		if (!pager->pages[i]) {
			continue;
		}

		pager_flush(pager, i, PAGE_SIZE);
		free(pager->pages[i]);
		pager->pages[i] = NULL;
	}

	uint32_t additional_rows_n = table->rows_n % ROWS_PER_PAGE;
	if (additional_rows_n > 0) {
		uint32_t page_n = full_pages_n;

		if (pager->pages[page_n]) {
			pager_flush(pager, page_n, additional_rows_n * ROW_SIZE);
			free(pager->pages[page_n]);
			pager->pages[page_n] = NULL;
		}
	}

	if (close(pager->file_descriptor) == -1) {
		DIE("%s\n", "Error closing database file");
	}

	for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
		void* page = pager->pages[i];

		if (page) {
			free(page);
			pager->pages[i] = NULL;
		}
	}

	free(pager);
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

	void* page = get_page(table->pager, page_n);

	uint32_t row_offset = row_n % ROWS_PER_PAGE;
	uint32_t byte_offset = row_offset * ROW_SIZE;

	return page + byte_offset;
}

/**
 * @brief Initialize Pager and page cache to NULLs
 *
 * @param filename
 * @return Pager*
 */
Pager* pager_open(const char* filename) {
	int stream;

	if ((stream = open(
		filename,
		// read / write mode
		O_RDWR |
		// create if not extant
		O_CREAT,
		// user write perm
		S_IWUSR |
		// user read perm
		S_IRUSR
	)) == -1) {
		DIE("%s\n", "Unable to open file");
	}

	off_t file_s = lseek(stream, 0, SEEK_END);

	Pager* pager = malloc(sizeof(Pager));
	pager->file_descriptor = stream;
	pager->file_s = file_s;

	for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
		pager->pages[i] = NULL;
	}

	return pager;
}

/**
 * @brief Get the page at the given offset.
 *
 * If the requested page is beyond the file bounds
 * we allocate memory and return it, as it should be empty. The page will
 * be written when flushing the cache to disk
 *
 * @param pager
 * @param page_n
 * @return void*
 */
void* get_page(Pager* pager, uint32_t page_n) {
	if (page_n > TABLE_MAX_PAGES) {
		DIE(
			"Attempted to fetch page number beyond range: %d > %d\n",
			page_n,
			TABLE_MAX_PAGES
		);
	}

	if (!pager->pages[page_n]) {
		// cache miss; alloc and load from file
		void* page = malloc(PAGE_SIZE);
		uint32_t page_s = pager->file_s / PAGE_SIZE;

		// save partial page at end of file
		if (pager->file_s % PAGE_SIZE) {
			page_s += 1;
		}

		if (page_n <= page_s) {
			lseek(pager->file_descriptor, page_n * PAGE_SIZE, SEEK_SET);

			ssize_t bytes_r = read(pager->file_descriptor, page, PAGE_SIZE);

			if (bytes_r == -1) {
				DIE("Error reading file: %d\n", errno);
			}
		}

		pager->pages[page_n] = page;
	}

	return pager->pages[page_n];
}

void pager_flush(Pager* pager, uint32_t page_n, uint32_t size) {
	if (!pager->pages[page_n]) {
		DIE("%s\n", "Attempted to flush null page");
	}

	if (lseek(pager->file_descriptor, page_n * PAGE_SIZE, SEEK_SET) == -1) {
		DIE("Error seeking: %d\n", errno);
	}

	if (write(pager->file_descriptor, pager->pages[page_n], size) == -1) {
		DIE("Error writing: %d\n", errno);
	}
}
