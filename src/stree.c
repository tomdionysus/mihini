#include "stree.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void stree_set(stree_node_t *node, char *str, void *value)
{
	uint32_t i = 0;

	while(true) {
		node = stree_add(node, str[i++]);
		if(str[i]==0) break;
		if(node->forward == NULL) {
			// Shortcut
			while(true) {
				node->forward = stree_new(str[i++]);
				node = node->forward;
				if(str[i]==0) break;
			}
			break;
		}
		node = node->forward;
	}
	
	uint32_t len = strlen(str);
	node->key = malloc(len+1);
	strcpy(node->key, str);
	node->key[len+1] = 0;
	node->value = value;
}

stree_node_t *stree_get(stree_node_t *node, char *str)
{
	uint32_t i = 0;

	while(true) {
		node = stree_find(node, str[i++]);
		if(node==NULL) return NULL;
		if(str[i]==0) return node;
		if(node->forward==NULL) return NULL;
		node = node->forward;
	}
}

bool stree_exists(stree_node_t *node, char *str) {
	node = stree_get(node, str);
	if(node==NULL) return false;
	if(node->value == 0) return false;
	return true;
}

void stree_free(stree_node_t **node)
{
	while((*node)->left) { 
		stree_free(&((*node)->left));
		(*node)->left = NULL;
	}
	while((*node)->right) { 
		stree_free(&((*node)->right));
		(*node)->right = NULL;
	}
	while((*node)->forward) { 
		stree_free(&((*node)->forward));
		(*node)->forward = NULL;
	}
	if((*node)->key!=NULL) free((*node)->key);
	free(*node);
	*node = NULL;
}

stree_node_t* stree_add(stree_node_t *node, char c)
{
	while(true) {
		if(node->c == c) return node;
		if(node==NULL) return stree_new(c);

		if(c < node->c) {
			if(node->left == NULL) return (node->left = stree_new(c));
			node = node->left;
		} else {
			if(node->right == NULL) return (node->right = stree_new(c));
			node = node->right;
		}
	}
}

void stree_insert(stree_node_t *node, stree_node_t *newnode) {

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

stree_node_t* stree_new(char c)
{
	stree_node_t *node = malloc(sizeof(stree_node_t));
	node->c = c;
	return node;
}

stree_node_t* stree_find(stree_node_t *node, char c)
{
	while(node!=NULL) {
		if(node->c == c) return node;
		if(c < node->c) {
			node = node->left;
		} else {
			node = node->right;
		}
	}

	return NULL;
}

void stree_iterate(stree_node_t *node, stree_iterator func, void *context) {
	if(node->left != NULL) stree_iterate(node->left, func, context);
	if(node->value != NULL && func(node->key, node->value, context)) return;
	if(node->right != NULL) stree_iterate(node->right, func, context);
	if(node->forward != NULL) stree_iterate(node->forward, func, context);
}

void stree_balance(stree_node_t **node) {
	if(*node == NULL) return;

	// Works by 'shuffling' the root left or right.
	// First, find the number of nodes to shuffle 
	int32_t m = ((int32_t)_stree_count(*node, false) - (int32_t)_stree_count(*node, true)) / 2;

	while(m != 0) {
		stree_node_t *current = *node;
		if(m > 0) {
			*node = current->right;
			current->right = NULL;
			stree_insert(*node, current);
			m--;
		} else {
			*node = current->left;
			current->left = NULL;
			stree_insert(*node, current);
			m++;
		}
	}

	stree_balance(&((*node)->left));
	stree_balance(&((*node)->right));
	stree_balance(&((*node)->forward));
} 

uint32_t _stree_count(stree_node_t *node, bool left) {
	uint32_t i = 0;
	while(node != NULL) {
		node = left ? node->left : node->right;
		i++;
	}
	return i-1;
}