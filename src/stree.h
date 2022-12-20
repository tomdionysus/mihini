#ifndef STREE
#define STREE

#include <stdint.h>
#include <stdbool.h>

#define STREE_MAX_STRING_LENGTH 65536;

typedef struct stree_node_t {
	struct stree_node_t *left;
	struct stree_node_t *right;
	struct stree_node_t *forward;
	char c;
	char *key;
	void *value;
} stree_node_t;

typedef bool (*stree_iterator)(char *key, void *value, void *context);

void stree_set(stree_node_t *node, char *str, void *value);
stree_node_t *stree_get(stree_node_t *node, char *str);
bool stree_exists(stree_node_t *node, char *str);

stree_node_t *stree_add(stree_node_t *node, char c);
void stree_insert(stree_node_t *node, stree_node_t *newnode);
stree_node_t *stree_new(char c);
stree_node_t *stree_find(stree_node_t *node, char c);
void stree_free(stree_node_t **node);

void stree_iterate(stree_node_t *node, stree_iterator func, void *context);

void stree_balance(stree_node_t **node);
uint32_t _stree_count(stree_node_t *node, bool left);

#endif