#include "pager.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "btree.h"
#include "common.h"

Table* db_open(const char* filename) {
  Pager* pager = pager_open(filename);
  Table* table = malloc(sizeof(Table));

  table->pager = pager;
  table->root_page_num = 0;

  if (pager->num_pages == 0) {
    // new file - initialize page 0 as leaf node
    void* root_node = get_page(pager, 0);
    leaf_node_init(root_node);
    set_root_node(root_node, true);
  }

  return table;
}

void db_close(Table* table) {
  Pager* pager = table->pager;

  for (uint32_t i = 0; i < pager->num_pages; i++) {
    if (!pager->pages[i]) {
      continue;
    }

    pager_flush(pager, i);
    free(pager->pages[i]);
    pager->pages[i] = NULL;
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

Pager* pager_open(const char* filename) {
  int fd;

  if ((fd = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR)) == -1) {
    DIE("%s\n", "Unable to open file");
  }

  off_t file_len = lseek(fd, 0, SEEK_END);

  Pager* pager = malloc(sizeof(Pager));
  pager->fd = fd;
  pager->file_len = file_len;
  pager->num_pages = (file_len / PAGE_SIZE);

  if (file_len % PAGE_SIZE != 0) {
    DIE("%s\n", "db file is corrupt");
  }

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
    uint32_t num_pages = pager->file_len / PAGE_SIZE;

    // save partial page at end of file
    if (pager->file_len % PAGE_SIZE) {
      num_pages++;
    }

    if (page_num <= num_pages) {
      lseek(pager->fd, page_num * PAGE_SIZE, SEEK_SET);

      if (read(pager->fd, page, PAGE_SIZE) == -1) {
        DIE("Error reading file: %d\n", errno);
      }
    }

    pager->pages[page_num] = page;

    if (page_num >= pager->num_pages) {
      pager->num_pages = page_num + 1;
    }
  }

  return pager->pages[page_num];
}

uint32_t get_unused_page_num(Pager* pager) { return pager->num_pages; }

void pager_flush(Pager* pager, uint32_t page_num) {
  if (!pager->pages[page_num]) {
    DIE("%s\n", "Attempted to flush null page");
  }

  if (lseek(pager->fd, page_num * PAGE_SIZE, SEEK_SET) == -1) {
    DIE("Error seeking: %d\n", errno);
  }

  if (write(pager->fd, pager->pages[page_num], PAGE_SIZE) == -1) {
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

Cursor* cursor_start_init(Table* table) {
  Cursor* cursor = malloc(sizeof(Cursor));
  cursor->table = table;
  cursor->cell_num = 0;
  cursor->page_num = table->root_page_num;

  void* root_node = get_page(table->pager, table->root_page_num);
  uint32_t num_cells = *leaf_node_num_cells(root_node);
  cursor->end = (num_cells == 0);

  return cursor;
}

/**
 * @brief Return the position of the given key.
 * If the key is not extant, return the position
 * where it should be inserted.
 */
Cursor* table_find_by_key(Table* table, uint32_t key) {
  uint32_t root_page_num = table->root_page_num;
  void* root_node = get_page(table->pager, root_page_num);

  if (get_node_type(root_node) == NODE_LEAF) {
    return leaf_node_find(table, root_page_num, key);
  }

  DIE("%s\n", "TODO - search internal node");
}

void* cursor_value(Cursor* cursor) {
  uint32_t page_num = cursor->page_num;
  void* page = get_page(cursor->table->pager, page_num);

  return leaf_node_value(page, cursor->cell_num);
}

void cursor_advance(Cursor* cursor) {
  uint32_t page_num = cursor->page_num;
  void* node = get_page(cursor->table->pager, page_num);

  cursor->cell_num++;
  if (cursor->cell_num >= (*leaf_node_num_cells(node))) {
    cursor->end = true;
  }
}
