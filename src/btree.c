#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "btree.h"

void btree_add(btree_node_t **node, uint32_t key, void *value) {
	if(*node==NULL) {
		*node = btree_new(key, value);
		return;
	}

	btree_insert(*node, btree_new(key, value));
}

void btree_insert(btree_node_t *node, btree_node_t *newnode) {

	while(true) {
		if(newnode->key < node->key) {
			if(node->left == NULL) {
				node->left = newnode;
				return;
			}
			node = node->left;
		} else {
			if(node->right == NULL) {
				node->right = newnode;
				return;
			}
			node = node->right;
		}
	}
}

btree_node_t *btree_get(btree_node_t *node, uint32_t key) {
	while(node!=NULL) {
		if(key == node->key) return node;
		if(key < node->key) {
			node = node->left;
		} else {
			node = node->right;
		}
	}

	return NULL;
}

void btree_free(btree_node_t **node) {
	if(*node == NULL) return;
	if((*node)->left) btree_free(&((*node)->left));
	if((*node)->right) btree_free(&((*node)->right));
	free(*node);
	*node = NULL;
}

btree_node_t *btree_new(uint32_t key, void *value) {
	btree_node_t *node = malloc(sizeof(btree_node_t));
	node->left = NULL;
	node->right = NULL;
	node->key = key;
	node->value = value;
	return node;
}

void btree_iterate(btree_node_t *node, btree_iterator func, void *context) {
	if(node == NULL) return;
	if(node->left) btree_iterate(node->left, func, context);
	if(func(node->key, node->value, context)) return;
	if(node->right) btree_iterate(node->right, func, context);
}

void btree_balance(btree_node_t **node) {
	if(*node == NULL) return;

	// Works by 'shuffling' the root left or right.
	// First, find the number of nodes to shuffle 
	int32_t m = ((int32_t)_btree_count(*node, false) - (int32_t)_btree_count(*node, true)) / 2;

	while(m != 0) {
		btree_node_t *current = *node;
		if(m > 0) {
			*node = current->right;
			current->right = NULL;
			btree_insert(*node, current);
			m--;
		} else {
			*node = current->left;
			current->left = NULL;
			btree_insert(*node, current);
			m++;
		}
	}

	btree_balance(&((*node)->left));
	btree_balance(&((*node)->right));
} 

uint32_t _btree_count(btree_node_t *node, bool left) {
	uint32_t i = 0;
	while(node != NULL) {
		node = left ? node->left : node->right;
		i++;
	}
	return i-1;
}
