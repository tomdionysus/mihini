#ifndef BTREE
#define BTREE

#include <stdint.h>
#include <stdbool.h>

typedef struct btree_node_t {
	struct btree_node_t *left;
	struct btree_node_t *right;
	uint32_t key;
	void *value;
} btree_node_t;

typedef bool (*btree_iterator)(uint32_t key, void *value, void *context);

void btree_add(btree_node_t **node, uint32_t key, void *value);
void btree_insert(btree_node_t *root, btree_node_t *node);
btree_node_t *btree_get(btree_node_t *node, uint32_t key);
void btree_free(btree_node_t **node);
btree_node_t *btree_new(uint32_t key, void *value);
void btree_iterate(btree_node_t *node, btree_iterator func, void *context);
void btree_balance(btree_node_t **node);
uint32_t _btree_count(btree_node_t *node, bool left);

#endif