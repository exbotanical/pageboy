#include "pager.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common.h"

Table* db_open(const char* filename) {
  Pager* pager = pager_open(filename);
  Table* table = malloc(sizeof(Table));

  table->pager = pager;
  table->num_rows = pager->file_s / ROW_SIZE;

  return table;
}

void db_close(Table* table) {
  Pager* pager = table->pager;
  uint32_t num_full_pages = table->num_rows / ROWS_PER_PAGE;

  for (uint32_t i = 0; i < num_full_pages; i++) {
    if (!pager->pages[i]) {
      continue;
    }

    pager_flush(pager, i, PAGE_SIZE);
    free(pager->pages[i]);
    pager->pages[i] = NULL;
  }

  uint32_t num_additional_rows = table->num_rows % ROWS_PER_PAGE;
  if (num_additional_rows > 0) {
    uint32_t page_num = num_full_pages;

    if (pager->pages[page_num]) {
      pager_flush(pager, page_num, num_additional_rows * ROW_SIZE);
      free(pager->pages[page_num]);
      pager->pages[page_num] = NULL;
    }
  }

  if (close(pager->fd) == -1) {
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

void* row_slot(Table* table, uint32_t row_num) {
  uint32_t page_num = row_num / ROWS_PER_PAGE;

  void* page = get_page(table->pager, page_num);

  uint32_t row_offset = row_num % ROWS_PER_PAGE;
  uint32_t byte_offset = row_offset * ROW_SIZE;

  return page + byte_offset;
}

Pager* pager_open(const char* filename) {
  int fd;

  if ((fd = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR)) == -1) {
    DIE("%s\n", "Unable to open file");
  }

  off_t file_s = lseek(fd, 0, SEEK_END);

  Pager* pager = malloc(sizeof(Pager));
  pager->fd = fd;
  pager->file_s = file_s;

  for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
    pager->pages[i] = NULL;
  }

  return pager;
}

void* get_page(Pager* pager, uint32_t page_num) {
  if (page_num > TABLE_MAX_PAGES) {
    DIE("Attempted to fetch page number beyond range: %d > %d\n", page_num,
        TABLE_MAX_PAGES);
  }

  if (!pager->pages[page_num]) {
    // cache miss; alloc and load from file
    void* page = malloc(PAGE_SIZE);
    uint32_t num_pages = pager->file_s / PAGE_SIZE;

    // save partial page at end of file
    if (pager->file_s % PAGE_SIZE) {
      num_pages++;
    }

    if (page_num <= num_pages) {
      lseek(pager->fd, page_num * PAGE_SIZE, SEEK_SET);

      if (read(pager->fd, page, PAGE_SIZE) == -1) {
        DIE("Error reading file: %d\n", errno);
      }
    }

    pager->pages[page_num] = page;
  }

  return pager->pages[page_num];
}

void pager_flush(Pager* pager, uint32_t page_num, uint32_t size) {
  if (!pager->pages[page_num]) {
    DIE("%s\n", "Attempted to flush null page");
  }

  if (lseek(pager->fd, page_num * PAGE_SIZE, SEEK_SET) == -1) {
    DIE("Error seeking: %d\n", errno);
  }

  if (write(pager->fd, pager->pages[page_num], size) == -1) {
    DIE("Error writing: %d\n", errno);
  }
}

void serialize_row(Row* src, void* dest) {
  memcpy(dest + ID_OFFSET, &(src->id), ID_SIZE);
  strncpy(dest + USERNAME_OFFSET, src->username, USERNAME_SIZE);
  strncpy(dest + EMAIL_OFFSET, src->email, EMAIL_SIZE);
}

void deserialize_row(void* src, Row* dest) {
  memcpy(&(dest->id), src + ID_OFFSET, ID_SIZE);
  memcpy(&(dest->username), src + USERNAME_OFFSET, USERNAME_SIZE);
  memcpy(&(dest->email), src + EMAIL_OFFSET, EMAIL_SIZE);
}
