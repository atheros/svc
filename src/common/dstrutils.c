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


dstrnode* dlist_find(dstrlist* list, const dstring* match) {
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


dstrnode* dlist_findcs(dstrlist* list, const char* match) {
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


dstring* dlist_join(dstrlist* list, const dstring* glue) {
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

dstring* dlist_joinc(dstrlist* list, int c) {
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


dstring* dlist_joincs(dstrlist* list, const char* glue) {
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



dstrlist* dsplit(const dstring* text, const dstring* on, size_t max) {
	dstrlist* result = dlist_new();
	int slen = on->len;
	unsigned int i, j, k = 0;

	if (max <= 1) {
		dlist_add(result, text);
		return result;
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

	if (max <= 1) {
		dlist_add(result, text);
		return result;
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

