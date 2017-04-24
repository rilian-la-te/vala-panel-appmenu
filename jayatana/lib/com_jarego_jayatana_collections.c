/*
 * Copyright (c) 2014 Jared González
 *
 * Permission is hereby granted, free of charge, to any
 * person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice
 * shall be included in all copies or substantial portions of
 * the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * File:   com_jarego_jayatana_collections.c
 * Author: Jared González
 */
#include "com_jarego_jayatana_collections.h"

#include <stdlib.h>

/**
 * Redimencionar lista
 */
void collection_list_realloc(ListIndex *list) {
	if (list->size == list->allocated) {
		list->allocated *= 2;
		list->entries = (ListIndexEntry **) realloc(list->entries,
				sizeof(ListIndexEntry *) * list->allocated);
	}
}

/**
 * Crear nueva lista
 */
ListIndex *collection_list_index_new() {
	ListIndex *list = (ListIndex *) malloc(sizeof(ListIndex));
	list->entries = (ListIndexEntry **) malloc(sizeof(ListIndexEntry *));
	list->allocated = 1;
	list->size = 0;
	return list;
}

/**
 * Agregar nuevo elemento a la lista
 */
void collection_list_index_add(ListIndex *list, long id, void *data) {
	collection_list_realloc(list);
	ListIndexEntry *entry = (ListIndexEntry *) malloc(sizeof(ListIndexEntry));
	entry->id = id;
	entry->data = data;
	list->entries[list->size] = entry;
	list->size++;
}

/**
 * Obtener un elemento de la lista
 */
void *collection_list_index_get(ListIndex *list, long id) {
	int i;
	for (i = 0; i < list->size; i++) {
		if (list->entries[i]->id == id)
			return list->entries[i]->data;
	}
	return NULL;
}

/**
 * Eliminar un elemento de la lista
 */
void *collection_list_index_remove(ListIndex *list, long id) {
	int i;
	for (i = 0; i < list->size; i++) {
		if (list->entries[i]->id == id) {
			void *data = list->entries[i]->data;
			free(list->entries[i]);
			int j;
			for (j = i + 1; j < list->size; j++) {
				list->entries[j - 1] = list->entries[j];
			}
			list->size--;
			return data;
		}
	}
	return NULL;
}

/**
 * Destruir lista
 */
void collection_list_index_destory(ListIndex *list) {
	int i;
	for (i = 0; i < list->size; i++)
		free(list->entries[i]);
	free(list->entries);
	free(list);
}

/**
 * Agregar elemento al final de la lista
 */
void collection_list_index_add_last(ListIndex *list, void *data) {
	collection_list_realloc(list);
	ListIndexEntry *entry = (ListIndexEntry *) malloc(sizeof(ListIndexEntry));
	entry->id = list->size;
	entry->data = data;
	list->entries[list->size] = entry;
	list->size++;
}

/**
 * Obtener el ultimo elemento de la lista
 */
void *collection_list_index_get_last(ListIndex *list) {
	return list->entries[list->size - 1]->data;
}

/**
 * Eliminar el ultimo elemento de la lista
 */
void *collection_list_index_remove_last(ListIndex *list) {
	if (list->size == 0)
		return NULL;
	void *data = collection_list_index_get_last(list);
	free(list->entries[list->size - 1]);
	list->entries[list->size - 1] = NULL;
	list->size--;
	return data;
}
