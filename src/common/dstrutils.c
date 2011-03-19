/*
 * Copyright (c) 2010-2011 Przemysław Grzywacz
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of dstrlib nor the names of its contributors may
 *    be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "dstrutils.h"
#include <stdlib.h>


/**
 * Linked list: Initialize a list object.
 */
#define list_init(list) \
	{list->front = NULL; list->back = NULL; list->size = 0;}

/**
 * Linked list: Append a node to linked list.
 *
 * @see list_init
 * @see list_prepend
 */
#define list_append(__list, __node)	do {\
	if (__list->back) {				\
		__node->prev = __list->back;	\
		__node->next = NULL;			\
		__list->back->next = __node;	\
		__list->back = __node;			\
		__list->size++;				\
	} else {						\
		__list->front = __node;			\
		__list->back = __node;			\
		__node->prev = NULL;			\
		__node->next = NULL;			\
		__list->size = 1;				\
	} } while (0)

/**
 * Linked list: Prepend a node to a list.
 *
 * @see list_init
 * @see list_append
 */
#define list_prepend(list, node)	\
	if (list->front) {					\
		node->next = list->front;		\
		node->prev = NULL;				\
		list->front->prev = node;		\
		list->front = node;				\
		list->size++;					\
	} else {							\
		list->front = node;				\
		list->back = node;				\
		node->prev = NULL;				\
		node->next = NULL;				\
		list->size = 1;					\
	}

/**
 * Linked list: Erase node from a list.
 *
 * @see list_append
 * @see list_prepend
 */
#define list_erase(list, node)						\
	if (list->front == node && list->back == node) {	\
		list->front = NULL;								\
		list->back = NULL;								\
		list->size--;									\
	} else if (node == list->front) {					\
		node->next->prev = NULL;						\
		list->front = node->next;						\
		list->size--;									\
	} else if (node == list->back) {					\
		node->prev->next = NULL;						\
		list->back = node->prev;						\
		list->size--;									\
	} else {											\
		node->prev->next = node->next;					\
		node->next->prev = node->prev;					\
		list->size--;									\
	}

/**
 * Linked list: Returns the size of the list.
 */
#define list_size(list)		(list->size)

/**
 * Linked list: Returns the front (first) element of the list.
 */
#define list_front(list)	(list->front)

/**
 * Linked list: Returns the back (last) element of the list.
 */
#define list_back(list)		(list->back)

/**
 * Linked list: Returns the previous element from node.
 */
#define list_prev(node)		(node->prev)

/**
 * Linked list: Returns the next element from node.
 */
#define list_next(node)		(node->next)

/**
 * Linked list: Returns 1 if list is empty, 0 if it's not.
 */
#define list_empty(list)	(list->size == 0 ? 1 : 0)

/**
 * Linked list: Returns 1 if node is the last node in list, 0 if it's not.
 */
#define list_last(node)		(node->next == NULL ? 1 : 0)

/**
 * Linked list: Returns 1 if this is the first node in list, 0 if it's not.
 */
#define list_first(node)	(node->prev == NULL ? 1 : 0)




dstrlist* dlist_new() {
	dstrlist* list = (dstrlist*)malloc(sizeof(dstrlist));
	list_init(list);
	return list;
}


void dlist_free(dstrlist* list) {
	dstrnode* node;
	while (list->front) {
		node = list->front;
		if (node->string) {
			dfree(node->string);
		}
		
		list->front = node->next;
		free(node);
	}
	free(list);
}


void dlist_push(dstrlist* list, dstring* text) {
	dstrnode* node = (dstrnode*)malloc(sizeof(dstrnode));
	node->string = text;
	list_append(list, node);
}


void dlist_add(dstrlist* list, const dstring* text) {
	dstrnode* node = (dstrnode*)malloc(sizeof(dstrnode));
	node->string = dnewcopy(text);
	list_append(list, node);
}


void dlist_addcs(dstrlist* list, const char* text) {
	dstrnode* node = (dstrnode*)malloc(sizeof(dstrnode));
	dstring* dtext = dfromcs(text);
	node->string = dtext;
	list_append(list, node);
}


void dlist_addmem(dstrlist* list, const void* text, size_t size) {
	dstrnode* node = (dstrnode*)malloc(sizeof(dstrnode));
	dstring* dtext = dfrommem(text, size);
	node->string = dtext;
	list_append(list, node);
}


void dlist_addnode(dstrlist* list, dstrnode* node) {
	list_append(list, node);
}


void dlist_erase(dstrlist* list, dstrnode* node) {
	dfree(node->string);
	list_erase(list, node);
	free(node);
}


void dlist_remove(dstrlist* list, dstrnode* node) {
	list_erase(list, node);
}


dstrnode* dlist_find(const dstrlist* list, const dstring* match) {
	dstrnode* node = list->front;
	while(node) {
		if (dcmp(node->string, match) == 0) {
			return node;
		} else {
			node = node->next;
		}
	}
	return NULL;
}


dstrnode* dlist_findcs(const dstrlist* list, const char* match) {
	dstrnode* node = list->front;
	while(node) {
		if (dcmpcs(node->string, match) == 0) {
			return node;
		} else {
			node = node->next;
		}
	}
	return NULL;
}


dstring* dlist_join(const dstrlist* list, const dstring* glue) {
	dstrnode* node = list->front;
	dstring* s = dnew();
	
	while(node) {
		dcat(s, node->string);
		node = node->next;
		if (node) {
			dcat(s, glue);
		}
	}
	
	return s;
}

dstring* dlist_joinc(const dstrlist* list, int c) {
	dstrnode* node = list->front;
	dstring* s = dnew();
	
	while(node) {
		dcat(s, node->string);
		node = node->next;
		if (node) {
			dcatc(s, c);
		}
	}
	
	return s;
}


dstring* dlist_joincs(const dstrlist* list, const char* glue) {
	dstrnode* node = list->front;
	dstring* s = dnew();
	int len = strlen(glue);
	
	while(node) {
		dcat(s, node->string);
		node = node->next;
		if (node) {
			dcatmem(s, glue, len);
		}
	}
	
	return s;
}


int dlist_equals(const dstrlist* l1, const dstrlist* l2) {
	dstrnode *n1, *n2;
	if (l1->size != l2->size) {
		return 0;
	}
	
	n1 = l1->front;
	n2 = l2->front;
	
	while(n1) {
		if (dcmp(n1->string, n2->string) != 0) {
			return 0;
		}
		
		n1 = n1->next;
		n2 = n2->next;
	}
	
	return 1;
}

dstring** dlist_tovector(const dstrlist* list) {
	dstring** vec;
	dstrnode* node;
	int i = 0;
	vec = (dstring**)malloc(sizeof(dstrlist*) * (list->size + 1));
	vec[list->size] = NULL;
	node = list->front;
	while(node) {
		vec[i++] = dnewcopy(node->string);
		node = node->next;
	}

	return vec;
}



dstring** dvec_new(size_t size) {
	dstring** vec;
	int i;
	vec = (dstring**)malloc(sizeof(dstring*) * (size + 1));
	for(i = 0; i < size; i++) {
		vec[i] = dnew();
	}

	vec[size] = NULL;

	return vec;
}

void dvec_free(dstring** vector) {
	int i;
	for(i = 0; vector[i] != NULL; i++) {
		dfree(vector[i]);
	}

	free(vector);
}

size_t dvec_size(dstring** vector) {
	size_t i = 0;
	while(vector[i] != NULL) i++;
	return i;
}


dstrlist* dsplit(const dstring* text, const dstring* on, size_t max) {
	dstrlist* result = dlist_new();
	int slen = on->len;
	unsigned int i, j, k = 0;

	if (max == 1) {
		dlist_add(result, text);
		return result;
	} else if (max <= 0) {
		max = text->len;
	}

	j = 0;
	i = dpos(text, on, 0);
	while(i != -1) {
		dlist_push(result, dsub(text, j, i - j));
		k++;
		j = i + slen;
		if (k + 1 == max) {
			dlist_push(result, dsub(text, j, text->len - j));
			return result;
		}
		i = dpos(text, on, j);
	}
	dlist_push(result, dsub(text, j, text->len - j));

	return result;
}


dstrlist* dsplit_on_cs(const dstring* text, const char* on, size_t max) {
	dstrlist* result = dlist_new();
	int slen = strlen(on);
	unsigned int i, j, k = 0;

	if (max == 1) {
		dlist_add(result, text);
		return result;
	} else if (max <= 0) {
		max = text->len;
	}

	j = 0;
	i = dposcs(text, on, 0);
	while(i != -1) {
		dlist_push(result, dsub(text, j, i - j));
		k++;
		j = i + slen;
		if (k + 1 == max) {
			dlist_push(result, dsub(text, j, text->len - j));
			return result;
		}
		i = dposcs(text, on, j);
	}
	dlist_push(result, dsub(text, j, text->len - j));

	return result;
}




dstrlist* dsplitcs(const char* text, const dstring* on, size_t max) {
	dstring *s;
	dstrlist *l;
	s = dfromcs(text);
	l = dsplit(s, on, max);
	dfree(s);
	return l;
}


dstrlist* dsplitcs_on_cs(const char* text, const char* on, size_t max) {
	dstring *s;
	dstrlist *l;
	s = dfromcs(text);
	l = dsplit_on_cs(s, on, max);
	dfree(s);
	return l;
}


dstrlist* dstrlex_parse(const dstring* text, int* errorcode) {
	int in_escape = 0;
	int in_string = 0;
	dstring* buffer = dnew();
	dstrlist* tokens = dlist_new();
	dstrlen_t i, len;
	int ch;
	
	if (errorcode) {
		*errorcode = DSTRLEX_OK;
	}
	
	len = text->len;
	for(i = 0; i < len; i++) {
		ch = text->data[i];
		
		if (in_escape) {
			switch (ch) {
			case 'n':
				dcatc(buffer, '\n');
				break;
			case 'r':
				dcatc(buffer, '\r');
				break;
			case '0':
				dcatc(buffer, '\0');
				break;
			case '"':
				dcatc(buffer, '"');
				break;
			case 't':
				dcatc(buffer, '\t');
				break;
			case '\\':
				dcatc(buffer, '\\');
				break;
			default:
				dcatc(buffer, ch);
				break;
			}
			in_escape = 0;
		} else if (in_string) {
			if (ch == '"') {
				dlist_add(tokens, buffer);
				dclear(buffer);
				in_string = 0;
			} else if (ch == '\\') {
				in_escape = 1;
			} else {
				dcatc(buffer, ch);
			}
		} else {
			if (ch == '"') {
				if (buffer->len) {
					dlist_add(tokens, buffer);
					dclear(buffer);
				}
				
				in_string = 1;
			} else if (ch == '\\') {
				in_escape = 1;
			} else if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
				if (buffer->len) {
					dlist_add(tokens, buffer);
					dclear(buffer);
				}
			} else {
				dcatc(buffer, ch);
			}
		}
	}

	if (in_escape) {
		if (errorcode) *errorcode = DSTRLEX_ESCAPE;
		dfree(buffer);
		dlist_free(tokens);
		return NULL;
	} else if (in_string) {
		if (errorcode) *errorcode = DSTRLEX_STRING;
		dfree(buffer);
		dlist_free(tokens);
		return NULL;
	} else if (buffer->len) {
		dlist_add(tokens, buffer);
	}
	
	dfree(buffer);
	return tokens;
}

dstring* dstrlex_escape(const dstring* text) {
	return dstrlex_escape_mem(text->data, text->len);
}
	
dstring* dstrlex_escape_cs(const char* text) {
	return dstrlex_escape_mem(text, strlen(text));
}

dstring* dstrlex_escape_mem(const void* mem, dstrlen_t size) {
	dstrlen_t i;
	dstring* o;
	int c;
	
	o = dnew();
	
	for(i = 0; i < size; i++) {
		c = ((const char*)mem)[i];
		switch (c) {
		case '\n':
			dcatcs(o, "\\n");
			break;
		case '\r':
			dcatcs(o, "\\r");
			break;
		case 0:
			dcatcs(o, "\\0");
			break;
		case '\"':
			dcatcs(o, "\\\"");
			break;
		case '\t':
			dcatcs(o, "\\t");
			break;
		case '\\':
			dcatcs(o, "\\\\");
			break;
		default:
			dcatc(o, c);
			break;
		}
	}
	
	return o;

}


const char* dstrlex_errstr(int code) {
	switch (code) {
	case DSTRLEX_OK:		return "Ok";
	case DSTRLEX_ESCAPE:	return "Invalid escape sequence";
	case DSTRLEX_STRING:	return "Unterminated string";
	default:				return "Unknown error";
	}
}




dsdict* dsdict_new() {
	dsdict* dict = (dsdict*)malloc(sizeof(dsdict));
	list_init(dict);
	return dict;
}

void dsdict_free(dsdict* dict) {
	dsdict_node* n = dict->front;
	while (dict->front) {
		n = dict->front;
		dfree(n->key);
		dfree(n->value);
		dict->front = n->next;
		free(n);
	}

	free(dict);
}

void dsdict_set(dsdict* dict, const char* key, const dstring* value) {
	dsdict_node* node = dsdict_iter(dict, key);

	if (node) {
		dcpy(node->value, value);
	} else {
		node = (dsdict_node*)malloc(sizeof(dsdict_node));
		node->key = dfromcs(key);
		node->value = dnewcopy(value);
		list_append(dict, node);
	}
}

void dsdict_setcs(dsdict* dict, const char* key, const char* value) {
	dsdict_node* node = dsdict_iter(dict, key);

	if (node) {
		dcpycs(node->value, value);
	} else {
		node = (dsdict_node*)malloc(sizeof(dsdict_node));
		node->key = dfromcs(key);
		node->value = dfromcs(value);
		list_append(dict, node);
	}
}

const dstring* dsdict_get(dsdict* dict, const char* key) {
	dsdict_node* node = dsdict_iter(dict, key);

	if (node) {
		return node->value;
	} else {
		return NULL;
	}
}

void dsdict_remove(dsdict* dict, const char* key) {
	dsdict_node* node = dsdict_iter(dict, key);

	if (node) {
		dfree(node->key);
		dfree(node->value);
		list_erase(dict, node);
		free(node);
	}
}

dsdict_node* dsdict_find(dsdict* dict, dstring* value, dsdict_node* iterator) {
	dsdict_node* node;

	if (iterator) {
		node = iterator->next;
	} else {
		node = dict->front;
	}

	while(node) {
		if (dcmp(node->value, value) == 0) {
			return node;
		}
		node = node->next;
	}

	return NULL;
}

dsdict_node* dsdict_findcs(dsdict* dict, const char* value, dsdict_node* iterator) {
	dsdict_node* node;
	size_t len = strlen(value);

	if (iterator) {
		node = iterator->next;
	} else {
		node = dict->front;
	}

	while(node) {
		if (dcmpmem(node->value, value, len) == 0) {
			return node;
		}
		node = node->next;
	}

	return NULL;
}

dsdict_node* dsdict_iter(dsdict* dict, const char* key) {
	dsdict_node* node;
	size_t len = strlen(key);

	node = dict->front;

	while(node) {
		if (dcmpmem(node->key, key, len) == 0) {
			return node;
		}
		node = node->next;
	}

	return NULL;
}
