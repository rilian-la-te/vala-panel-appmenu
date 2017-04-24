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
 * File:   com_jarego_jayatana_collections.h
 * Author: Jared González
 */
#ifndef COM_JAREGO_JAYATANA_LIST_INDEX_H_
#define COM_JAREGO_JAYATANA_LIST_INDEX_H_

/**
 * Estructura de entrada de lista
 */
typedef struct {
        long id;
        void *data;
} ListIndexEntry;

/**
 * Estructura de lista
 */
typedef struct {
        ListIndexEntry **entries;
        unsigned long allocated;
        unsigned long size;
} ListIndex;

/**
 * Crear nueva lista
 */
ListIndex *collection_list_index_new();
/**
 * Agregar nuevo elemento a la lista
 */
void collection_list_index_add(ListIndex *, long, void *);
/**
 * Obtener un elemento de la lista
 */
void *collection_list_index_get(ListIndex *, long);
/**
 * Eliminar un elemento de la lista
 */
void *collection_list_index_remove(ListIndex *, long);
/**
 * Destruir lista
 */
void collection_list_index_destory(ListIndex *);
/**
 * Agregar elemento al final de la lista
 */
void collection_list_index_add_last(ListIndex *, void *);
/**
 * Obtener el ultimo elemento de la lista
 */
void *collection_list_index_get_last(ListIndex *);
/**
 * Eliminar el ultimo elemento de la lista
 */
void *collection_list_index_remove_last(ListIndex *);

#endif /* COM_JAREGO_JAYATANA_LIST_INDEX_H_ */
