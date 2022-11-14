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

uint32_t* internal_node_num_keys(void* node) {
  return node + INTERNAL_NODE_NUM_KEYS_OFFSET;
}

void internal_node_init(void* node) {
  set_node_type(node, NODE_INTERNAL);
  set_root_node(node, false);
  *internal_node_num_keys(node) = 0;
}

uint32_t* internal_node_right_child(void* node) {
  return node + INTERNAL_NODE_RIGHT_CHILD_OFFSET;
}

uint32_t* internal_node_cell(void* node, uint32_t cell_num) {
  return node + INTERNAL_NODE_HEADER_SIZE + cell_num * INTERNAL_NODE_CELL_SIZE;
}

uint32_t* internal_node_child(void* node, uint32_t child_num) {
  uint32_t num_keys = *internal_node_num_keys(node);
  if (child_num > num_keys) {
    DIE("attempt to access child_num %d > num_keys %d\n", child_num, num_keys);
  }

  if (child_num == num_keys) {
    return internal_node_right_child(node);
  }

  return internal_node_cell(node, child_num);
}

uint32_t* internal_node_key(void* node, uint32_t key_num) {
  return internal_node_cell(node, key_num) + INTERNAL_NODE_CHILD_SIZE;
}

uint32_t get_node_max_key(void* node) {
  switch (get_node_type(node)) {
    case NODE_INTERNAL:
      return *internal_node_key(node, *internal_node_num_keys(node) - 1);

    case NODE_LEAF:
      return *leaf_node_key(node, *leaf_node_num_cells(node) - 1);

    default:
      DIE("%s\n", "unknown NodeType");
  }
}

bool is_root_node(void* node) {
  uint8_t value = *((uint8_t*)(node + IS_ROOT_OFFSET));
  return (bool)value;
}

void set_root_node(void* node, bool is_root) {
  uint8_t value = is_root;
  *((uint8_t*)(node + IS_ROOT_OFFSET)) = value;
}

/*
"Let N be the root node. First allocate two nodes, say L and R. Move lower
half of N into L and the upper half into R. Now N is empty. Add 〈L, K,R〉
in N, where K is the max key in L. Page N remains the root. Note that the
depth of the tree has increased by one, but the new tree remains height
balanced without violating any B+-tree property." - SQLite Database System
*/
void set_new_root(Table* table, uint32_t right_child_page_num) {
  // Split the root node, where the old root is
  // copied into a new page and becomes the left child.
  // The address of the right child is passed in.
  void* root = get_page(table->pager, table->root_page_num);
  void* right_child = get_page(table->pager, right_child_page_num);
  uint32_t left_child_page_num = get_unused_page_num(table->pager);
  void* left_child = get_page(table->pager, left_child_page_num);

  // Copy old root to left child
  memcpy(left_child, root, PAGE_SIZE);
  set_root_node(left_child, false);

  // Initialize root page as new internal node w/ 1 key, 2 children
  internal_node_init(root);
  *internal_node_num_keys(root) = 1;
  *internal_node_child(root, 0) = left_child_page_num;
  uint32_t left_child_max_key = get_node_max_key(left_child);
  *internal_node_key(root, 0) = left_child_max_key;
  *internal_node_right_child(root) = right_child_page_num;
}

uint32_t* leaf_node_num_cells(void* node) {
  return node + LEAF_NODE_NUM_CELLS_OFFSET;
}

void* leaf_node_cell(void* node, uint32_t cell_num) {
  return node + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
}

uint32_t* leaf_node_key(void* node, uint32_t cell_num) {
  return leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}

void* leaf_node_value(void* node, uint32_t cell_num) {
  return leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}

void leaf_node_init(void* node) {
  set_node_type(node, NODE_LEAF);
  set_root_node(node, false);
  *leaf_node_num_cells(node) = 0;
}

void leaf_node_insert(Cursor* cursor, uint32_t key, Row* value) {
  void* node = get_page(cursor->table->pager, cursor->page_num);

  uint32_t num_cells = *leaf_node_num_cells(node);
  if (num_cells >= LEAF_NODE_MAX_CELLS) {
    // node full
    leaf_node_split_and_insert(cursor, key, value);
    return;
  }

  if (cursor->cell_num < num_cells) {
    // allocate room for new cell
    for (uint32_t i = num_cells; i > cursor->cell_num; i--) {
      memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i - 1),
             LEAF_NODE_CELL_SIZE);
    }
  }

  *(leaf_node_num_cells(node)) += 1;
  *(leaf_node_key(node, cursor->cell_num)) = key;
  serialize_row(value, leaf_node_value(node, cursor->cell_num));
}

Cursor* leaf_node_find(Table* table, uint32_t page_num, uint32_t key) {
  void* node = get_page(table->pager, page_num);
  uint32_t num_cells = *leaf_node_num_cells(node);

  Cursor* cursor = malloc(sizeof(Cursor));
  cursor->table = table;
  cursor->page_num = page_num;

  // perform binary search
  uint32_t min_idx = 0;
  uint32_t max_idx_plus_one = num_cells;

  while (max_idx_plus_one != min_idx) {
    uint32_t idx = (min_idx + max_idx_plus_one) / 2;
    uint32_t idx_key = *leaf_node_key(node, idx);

    if (key == idx_key) {
      cursor->cell_num = idx;
      return cursor;
    }

    if (key < idx_key) {
      max_idx_plus_one = idx;
    } else {
      min_idx = idx + 1;
    }
  }

  cursor->cell_num = min_idx;
  return cursor;
}

NodeType get_node_type(void* node) {
  uint8_t value = *((uint8_t*)(node + NODE_TYPE_OFFSET));
  return (NodeType)value;
}

void set_node_type(void* node, NodeType type) {
  uint8_t value = type;
  *((uint8_t*)(node + NODE_TYPE_OFFSET)) = value;
}

void leaf_node_split_and_insert(Cursor* cursor, uint32_t key, Row* value) {
  // Create a new node
  void* old_node = get_page(cursor->table->pager, cursor->page_num);
  uint32_t new_page_num = get_unused_page_num(cursor->table->pager);
  void* new_node = get_page(cursor->table->pager, new_page_num);
  leaf_node_init(new_node);

  // Move half of the cells over.
  // All extant keys plus new key will be divided evenly
  // across the old (left) and new (right) nodes.
  // Begin with the right, moving each key to its new position.
  for (uint32_t i = LEAF_NODE_MAX_CELLS; i >= 0; i--) {
    void* destination_node;
    if (i >= LEAF_NODE_LEFT_SPLIT_COUNT) {
      destination_node = new_node;
    } else {
      destination_node = old_node;
    }

    uint32_t node_idx = i % LEAF_NODE_LEFT_SPLIT_COUNT;
    void* destination = leaf_node_cell(destination_node, node_idx);

    // Insert the new value in one of these two new nodes
    if (i == cursor->cell_num) {
      serialize_row(value, destination);
    } else if (i > cursor->cell_num) {
      memcpy(destination, leaf_node_cell(old_node, i - 1), LEAF_NODE_CELL_SIZE);
    } else {
      memcpy(destination, leaf_node_cell(old_node, i), LEAF_NODE_CELL_SIZE);
    }
  }

  // Update cell counts in each leaf node's header
  *(leaf_node_num_cells(old_node)) = LEAF_NODE_LEFT_SPLIT_COUNT;
  *(leaf_node_num_cells(new_node)) = LEAF_NODE_RIGHT_SPLIT_COUNT;

  // Finally, update the parent or create a new one.
  // If the original node was the root node, it had no parent -
  // thus, we create a new root node i.e. parent.
  if (is_root_node(old_node)) {
    return set_new_root(cursor->table, new_page_num);
  } else {
    DIE("%s\n", "TODO - update parent");
  }
}
