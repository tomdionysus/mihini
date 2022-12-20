#include <stdlib.h>
#include <memory.h>
#include <stddef.h>

#include "dll.h"

void dll_push(dll_t *list, void *value) {
	// If list is empty
	if(list->first == NULL) {
		list->first = dll_node_new(value);
		list->last = list->first;
		list->count++;
		return;
	}

	// Add at end of list, update previous last and list last
	dll_node_t *prev = list->last;
	list->last = dll_node_new(value);
	list->last->prev = prev;
	prev->next = list->last;
	list->count++;
}

void *dll_pop(dll_t *list) {
	// If List is empty
	if(list->first == NULL) return NULL;

	// Remove from end of list
	dll_node_t *last = list->last;
	list->last = last->prev;
	if(last->prev!=NULL) last->prev->next = NULL;
	if(list->first == last) list->first = NULL;
	list->count--;

	// Free Node
	void *value = last->value;
	free(last);
	return value;
}

void dll_unshift(dll_t *list, void *value) {
	// If list is empty
	if(list->first == NULL) {
		list->first = dll_node_new(value);
		list->last = list->first;
		list->count++;
		return;
	}

	dll_node_t *prev = list->first;
	list->first = dll_node_new(value);
	list->first->next = prev;
	if(prev!=NULL) prev->prev = list->first;
	list->count++;
}

void *dll_shift(dll_t *list) {
	// If List is empty
	if(list->first == NULL) return NULL;

	// Remove from start of list
	dll_node_t *first = list->first;
	list->first = first->next;
	if(first->next!=NULL) first->next->prev = NULL;
	if(list->last == first) list->last = NULL;
	list->count--;

	// Free Node
	void *value = first->value;
	free(first);
	return value;
}

dll_node_t *dll_node_new(void *value) {
	dll_node_t *node = (dll_node_t*)calloc(sizeof(dll_node_t),1);
	node->value = value;
	return node;
}

void dll_iterate(dll_t *list, dll_iterator func, void *context, bool forward) {
	dll_node_t *current = list->first;
	while (current!=NULL) {
		dll_node_t *j = forward ? current->next : current->prev;
		if(func(current->value, context)) break;
		current = j;
	}
}

void dll_free(dll_t *list) {
	dll_node_t *current = list->first;
	while (current!=NULL) {
		dll_node_t *next = current->next;
		free(current);
		current = next;
	}
	list->first = NULL;
	list->last = NULL;
}

dll_t *dll_new() {
	return calloc(sizeof(dll_t),1);
}
