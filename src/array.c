#include "array.h"
#include <stdlib.h>
#include <string.h>

Array array_create(size_t sz) {
	Array ret;
	ret.array = malloc(64 * sz * sizeof(void *));
	ret.size = sz;
	ret.used = 0;
	return ret;
}

void array_add(Array *l, void *element) {
	if(l->used >= l->size) {
		l->array = realloc(l->array, l->size * 2 * sizeof(void*));
		l->size *= 2;
	}
	l->array[l->used] = element;
	l->used++;
}

void *array_get(Array l, unsigned index) {
	return l.array[index];
}

unsigned array_index_of(Array l, const void *element) {
	for(int i = 0; i < l.used; i++) {
		if(l.array[i] == element)
			return i;
	}
	return -1;
}

void array_destroy(Array *l) {
	free(l->array);
	l->array = NULL;
}

bool array_is_in(Array l, const void *n) {
	for(int i = 0; i < l.used; i++) {
		if(l.array[i] == n)
			return true;
	}
	return false;
}

