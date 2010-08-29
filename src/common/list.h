/*
 * list.h
 *
 *  Created on: 2009-11-29
 *      Author: przemek
 */

#ifndef LIST_H_
#define LIST_H_


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
#define list_append(__list, __node)	\
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
	}

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

#endif /* LIST_H_ */