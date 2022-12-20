#ifndef DLL
#define DLL

#include <stdint.h>
#include <stdbool.h>

typedef struct dll_node {
	struct dll_node *next; 
	struct dll_node *prev; 
	void *value; 
} dll_node_t;

typedef struct dll {
	dll_node_t *first;
	dll_node_t *last;
	uint32_t count;
} dll_t;

typedef bool (*dll_iterator)(void *value, void *context);

void dll_push(dll_t *list, void *value);
void *dll_pop(dll_t *list);
void dll_unshift(dll_t *list, void *value);
void *dll_shift(dll_t *list);
dll_t *dll_new();
void dll_free(dll_t *list);

void dll_iterate(dll_t *list, dll_iterator func, void *context, bool forward);

dll_node_t *dll_node_new(void *value);

#endif