/*
 * Copyright (c) 2010 Przemysław Grzywacz
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
 * @version 0.1
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
dstrnode* dlist_find(dstrlist* list, const dstring* match);

/** return a node with matching string (or NULL if not found) */
dstrnode* dlist_findcs(dstrlist* list, const char* match);

/** joins all elements in the list using glue and returns a new string */
dstring* dlist_join(dstrlist* list, const dstring* glue);

/** joins all elements in the list using glue and returns a new string */
dstring* dlist_joinc(dstrlist* list, int c);

/** joins all elements in the list using glue and returns a new string */
dstring* dlist_joincs(dstrlist* list, const char* glue);

/** returns 1 if both list are equal in size and in contents */
int dlist_equals(const dstrlist* l1, const dstrlist* l2);




/** splits a on find and returns a list */
dstrlist* dsplit(const dstring* text, const dstring* on, size_t max);

/** splits a on find and returns a list */
dstrlist* dsplit_on_cs(const dstring* text, const char* on, size_t max);

/** splits a on find and returns a list */
dstrlist* dsplitcs(const char* text, const dstring* on, size_t max);

/** splits a on find and returns a list */
dstrlist* dsplitcs_on_cs(const char* text, const char* on, size_t max);


#ifdef __cplusplus
}
#endif

#endif /* !__DSTRUTILS_H */