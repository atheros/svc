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

/**
 * Dynamic strings utility library.
 *
 * @version 0.3
 */

#ifndef __DSTRUTILS_H
#define __DSTRUTILS_H

#include "dstr.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _dstrnode {
	struct _dstrnode *prev, *next;
	dstring* string;
} dstrnode;

typedef struct _dstrlist {
	dstrnode *front, *back;
	size_t size;
} dstrlist;


typedef struct _dsdict_node {
	struct _dsdict_node *prev, *next;
	dstring *key, *value;
} dsdict_node;

typedef struct {
	dsdict_node *front, *back;
	size_t size;
} dsdict;



#define DSTRLEX_OK		0
#define DSTRLEX_ESCAPE	1
#define DSTRLEX_STRING	2


/** allocate a new string list */
dstrlist* dlist_new();

/** free list with all the nodes and strings */
void dlist_free(dstrlist* list);

/** add text to list (without copy) */
void dlist_push(dstrlist* list, dstring* text);

/** add text to the list (with copy) */
void dlist_add(dstrlist* list, const dstring* text);

/** add a C string to list */
void dlist_addcs(dstrlist* list, const char* text);

/** add a string from memory to list */
void dlist_addmem(dstrlist* list, const void* text, size_t size);

/** add a string node to list */
void dlist_addnode(dstrlist* list, dstrnode* node);

/** remove node from list and free the node and string */
void dlist_erase(dstrlist* list, dstrnode* node);

/** remove node from list without freeing the node or the string */
void dlist_remove(dstrlist* list, dstrnode* node);

/** return a node with matching string (or NULL if not found) */
dstrnode* dlist_find(const dstrlist* list, const dstring* match);

/** return a node with matching string (or NULL if not found) */
dstrnode* dlist_findcs(const dstrlist* list, const char* match);

/** joins all elements in the list using glue and returns a new string */
dstring* dlist_join(const dstrlist* list, const dstring* glue);

/** joins all elements in the list using glue and returns a new string */
dstring* dlist_joinc(const dstrlist* list, int c);

/** joins all elements in the list using glue and returns a new string */
dstring* dlist_joincs(const dstrlist* list, const char* glue);

/** returns 1 if both list are equal in size and in contents */
int dlist_equals(const dstrlist* l1, const dstrlist* l2);

/** convert list to a vector (terminated by NULL element) */
dstring** dlist_tovector(const dstrlist* list);



/** allocate a vector of given size with empty strings */
dstring** dvec_new(size_t size);

/** free dstring vector (NULL terminated) */
void dvec_free(dstring** vector);

/** returns the size of the vector (last valid index, pointing to NULL string) */
size_t dvec_size(dstring** vector);



/** splits on on and returns a list */
dstrlist* dsplit(const dstring* text, const dstring* on, size_t max);

/** splits on on and returns a list */
dstrlist* dsplit_on_cs(const dstring* text, const char* on, size_t max);

/** splits on on and returns a list */
dstrlist* dsplitcs(const char* text, const dstring* on, size_t max);

/** splits on on find and returns a list */
dstrlist* dsplitcs_on_cs(const char* text, const char* on, size_t max);



/**
 * Minimalistic lexer implementation.
 * 
 * Tokens are separated by whitespaces (space, tab, \r and \n).
 * 
 * If a literal token ends with an opening quote, it starts a new token
 * (so foo"bar"foo will generate 3 tokens, foo, bar and foo).
 * 
 * Valid escapes:
 * 		\n
 * 		\r
 * 		\0
 * 		\"
 *      \t
 * 		\\
 *
 * If an escape is invalid, the escaped character (without \) will be
 * written to output.
 *
 * If you need whitespaces in a token, you need to quote it with ".
 * 
 * Returns tokens found in string.
 * On error NULL is return and errorcode will contain a numeric code of
 * what went wrong (if errorcode != NULL).
 * 
 * DSTRLEX_OK		- no error
 * DSTRLEX_ESCAPE	- unterminated escape
 * DSTRLEX_STRING	- unterminated string
 */
dstrlist* dstrlex_parse(const dstring* text, int* errorcode);

/**
 * Escape a string to be valid when using dstrlex_parse() function.
 */
dstring* dstrlex_escape(const dstring* text);



dsdict* dsdict_new();
void dsdict_free(dsdict* dict);
void dsdict_set(dsdict* dict, const char* key, const dstring* value);
void dsdict_setcs(dsdict* dict, const char* key, const char* value);
const dstring* dsdict_get(dsdict* dict, const char* key);
void dsdict_remove(dsdict* dict, const char* key);
dsdict_node* dsdict_find(dsdict* dict, dstring* value, dsdict_node* iterator);
dsdict_node* dsdict_findcs(dsdict* dict, const char* value, dsdict_node* iterator);
dsdict_node* dsdict_iter(dsdict* dict, const char* key);

#ifdef __cplusplus
}
#endif

#endif /* !__DSTRUTILS_H */
