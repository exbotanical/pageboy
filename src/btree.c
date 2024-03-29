#include "btree.h"

#include <string.h>

#include "common.h"

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
  return (void*)internal_node_cell(node, key_num) + INTERNAL_NODE_CHILD_SIZE;
}

void internal_node_update_key(void* node, uint32_t old_key, uint32_t new_key) {
  uint32_t old_child_idx = internal_node_find_child(node, old_key);

  *internal_node_key(node, old_child_idx) = new_key;
}

/**
 * @brief Return the  f the child which should contain the given key.
 */
Cursor* internal_node_find_child(void* node, uint32_t key) {
  uint32_t num_keys = *internal_node_num_keys(node);

  // Binary search
  uint32_t min_idx = 0;
  uint32_t max_idx = num_keys;

  while (min_idx != max_idx) {
    uint32_t idx = (min_idx + max_idx) / 2;
    uint32_t key_to_right = *internal_node_key(node, idx);

    if (key_to_right >= key) {
      max_idx = idx;
    } else {
      min_idx = idx + 1;
    }
  }

  return min_idx;
}

Cursor* internal_node_find(Table* table, uint32_t page_num, uint32_t key) {
  void* node = get_page(table->pager, page_num);

  uint32_t child_idx = internal_node_find_child(node, key);
  uint32_t child_num = *internal_node_child(node, child_idx);
  void* child = get_page(table->pager, child_num);

  switch (get_node_type(child)) {
    case NODE_LEAF:
      return leaf_node_find(table, child_num, key);
    case NODE_INTERNAL:
      return internal_node_find(table, child_num, key);
  }

  DIE("%s\n", "todo");
}

void internal_node_insert(Table* table, uint32_t parent_page_num,
                          uint32_t child_page_num) {
  // Add a new child / key pair to the parent that corresponds to child
  void* parent = get_page(table->pager, parent_page_num);
  void* child = get_page(table->pager, child_page_num);

  uint32_t child_max_key = get_node_max_key(child);
  uint32_t idx = internal_node_find_child(parent, child_max_key);
  uint32_t original_num_keys = *internal_node_num_keys(parent);

  *internal_node_num_keys(parent) = original_num_keys + 1;

  if (original_num_keys >= INTERNAL_NODE_MAX_CELLS) {
    DIE("%s\n", "TODO");
  }

  uint32_t right_child_page_num = *internal_node_right_child(parent);
  void* right_child = get_page(table->pager, right_child_page_num);

  if (child_max_key > get_node_max_key(right_child)) {
    // Replace right child
    *internal_node_child(parent, original_num_keys) = right_child_page_num;
    *internal_node_key(parent, original_num_keys) =
        get_node_max_key(right_child);
    *internal_node_right_child(parent) = child_page_num;
  } else {
    // Allocate space for new cell
    for (uint32_t i = original_num_keys; i > idx; i--) {
      void* destination = internal_node_cell(parent, i);
      void* source = internal_node_cell(parent, i - 1);
      memcpy(destination, source, INTERNAL_NODE_CELL_SIZE);
    }

    *internal_node_child(parent, idx) = child_page_num;
    *internal_node_key(parent, idx) = child_max_key;
  }
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

  *get_parent_node(left_child) = table->root_page_num;
  *get_parent_node(right_child) = table->root_page_num;
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
  *leaf_node_next_leaf(node) = 0;  // where 0 represents no sibling
}

uint32_t* leaf_node_next_leaf(void* node) {
  return node + LEAF_NODE_NEXT_LEAF_OFFSET;
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

uint32_t* get_parent_node(void* node) { return node + PARENT_POINTER_OFFSET; }

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
  uint32_t old_max = get_node_max_key(old_node);

  uint32_t new_page_num = get_unused_page_num(cursor->table->pager);
  void* new_node = get_page(cursor->table->pager, new_page_num);
  leaf_node_init(new_node);
  *get_parent_node(new_node) = *get_parent_node(old_node);

  // After splitting, update the sibling pointers
  // where old sibling becomes new leaf,
  // new leaf's sibling is old leaf's sibling
  *leaf_node_next_leaf(new_node) = *leaf_node_next_leaf(old_node);
  *leaf_node_next_leaf(old_node) = new_page_num;

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
      serialize_row(value, leaf_node_value(destination_node, node_idx));
      *leaf_node_key(destination_node, node_idx) = key;
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
    // Update first key in the parent to be the max.
    // Add new child pointer / key pair, where the pointer
    // points to the new child node and the new key is that child's max.
    uint32_t parent_page_num = *get_parent_node(old_node);
    uint32_t new_max = get_node_max_key(old_node);
    void* parent = get_page(cursor->table->pager, parent_page_num);

    internal_node_update_key(parent, old_max, new_max);
    internal_node_insert(cursor->table, parent_page_num, new_page_num);

    return;
  }
}
