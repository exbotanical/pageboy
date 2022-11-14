#ifndef BTREE_H
#define BTREE_H

#include <stdint.h>

#include "pager.h"

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

static const uint32_t LEAF_NODE_RIGHT_SPLIT_COUNT =
    (LEAF_NODE_MAX_CELLS + 1) / 2;
static const uint32_t LEAF_NODE_LEFT_SPLIT_COUNT =
    (LEAF_NODE_MAX_CELLS + 1) - LEAF_NODE_RIGHT_SPLIT_COUNT;

static const uint32_t INTERNAL_NODE_NUM_KEYS_SIZE = sizeof(uint32_t);
static const uint32_t INTERNAL_NODE_NUM_KEYS_OFFSET = COMMON_NODE_HEADER_SIZE;
static const uint32_t INTERNAL_NODE_RIGHT_CHILD_SIZE = sizeof(uint32_t);
static const uint32_t INTERNAL_NODE_RIGHT_CHILD_OFFSET =
    INTERNAL_NODE_NUM_KEYS_OFFSET + INTERNAL_NODE_NUM_KEYS_SIZE;
static const uint32_t INTERNAL_NODE_HEADER_SIZE =
    COMMON_NODE_HEADER_SIZE + INTERNAL_NODE_NUM_KEYS_SIZE +
    INTERNAL_NODE_RIGHT_CHILD_SIZE;

static const uint32_t INTERNAL_NODE_KEY_SIZE = sizeof(uint32_t);
static const uint32_t INTERNAL_NODE_CHILD_SIZE = sizeof(uint32_t);
static const uint32_t INTERNAL_NODE_CELL_SIZE =
    INTERNAL_NODE_CHILD_SIZE + INTERNAL_NODE_KEY_SIZE;

void leaf_node_init(void* node);

void leaf_node_insert(Cursor* cursor, uint32_t key, Row* value);

uint32_t* leaf_node_num_cells(void* node);

uint32_t* leaf_node_key(void* node, uint32_t cell_num);

void* leaf_node_value(void* node, uint32_t cell_num);

Cursor* leaf_node_find(Table* table, uint32_t page_num, uint32_t key);

NodeType get_node_type(void* node);

void set_node_type(void* node, NodeType type);

void set_root_node(void* node, bool is_root);

void leaf_node_split_and_insert(Cursor* cursor, uint32_t key, Row* value);

#endif /* BTREE_H */
