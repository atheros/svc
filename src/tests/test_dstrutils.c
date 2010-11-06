#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dstr.h"
#include "dstrutils.h"

#include "tests_utils.h"

int main(int argc, char* argv[]) {
	dstrlist* l1;
	dstrnode *n1, *n2, *n3, *n4;
	dstring* s1;
	
	
	test_start("dlist_new()");
	l1 = dlist_new();
	test_assert(l1 != NULL);
	test_assert(l1->size == 0);
	test_assert(l1->front == NULL);
	test_assert(l1->back == NULL);
	test_end();
	
	
	test_start("dlist_free()");
	dlist_free(l1);
	test_assert(1);
	test_end();
	
	
	test_start("dlist_push()");
	l1 = dlist_new();
	s1 = dnew();
	dcpycs(s1, "test1");
	dlist_push(l1, s1);
	test_assert(strcmp(s1->data, "test1") == 0);
	test_assert(dcmpcs(s1, "test1") == 0);
	test_assert(l1->size == 1);
	test_assert(l1->front != NULL);
	test_assert(l1->front == l1->back);
	test_assert(l1->front->string == s1);
	test_assert(l1->front->prev == NULL);
	test_assert(l1->front->next == NULL);
	test_assert(dcmpcs(l1->front->string, "test1") == 0);
	dlist_free(l1);
	test_end();
	
	
	test_start("dlist_add()");
	l1 = dlist_new();
	s1 = dfromcs("Hello World!");
	
	dlist_add(l1, s1);
	dlist_add(l1, s1);
	dlist_add(l1, s1);
	
	test_assert(l1->size == 3);
	test_assert(l1->front != l1->back);
	test_assert(l1->front->string != l1->front->next->string);
	test_assert(dcmpcs(l1->front->string, "Hello World!") == 0);
	test_assert(dcmp(l1->front->string, l1->front->next->string) == 0);
	test_assert(dcmp(l1->front->string, l1->front->next->next->string) == 0);
	
	dlist_free(l1);
	dfree(s1);
	test_end();
	
	
	test_start("dlist_addcs()");
	l1 = dlist_new();
	
	dlist_addcs(l1, "Hello Guys");
	dlist_addcs(l1, "Hello Guys");
	dlist_addcs(l1, "Hello Guys");
	
	test_assert(l1->size == 3);
	test_assert(l1->front != l1->back);
	test_assert(l1->front->string != l1->front->next->string);
	test_assert(dcmpcs(l1->front->string, "Hello Guys") == 0);
	test_assert(dcmp(l1->front->string, l1->front->next->string) == 0);
	test_assert(dcmp(l1->front->string, l1->front->next->next->string) == 0);
	
	dlist_free(l1);
	test_end();
	
	
	test_start("dlist_addmem()");
	l1 = dlist_new();
	
	dlist_addmem(l1, "Hello Guys", 10);
	dlist_addmem(l1, "Hello Guys", 10);
	dlist_addmem(l1, "Hello Guys", 10);
	
	test_assert(l1->size == 3);
	test_assert(l1->front != l1->back);
	test_assert(l1->front->string != l1->front->next->string);
	test_assert(dcmpcs(l1->front->string, "Hello Guys") == 0);
	test_assert(dcmp(l1->front->string, l1->front->next->string) == 0);
	test_assert(dcmp(l1->front->string, l1->front->next->next->string) == 0);
	
	dlist_free(l1);
	test_end();
	
	
	test_start("dlist_addnode()");
	l1 = dlist_new();
	n1 = (dstrnode*)malloc(sizeof(dstrnode));
	n1->string = dfromcs("Hi there!");
	dlist_addnode(l1, n1);
	test_assert(l1->size == 1);
	test_assert(l1->front == l1->back);
	test_assert(dcmpcs(l1->front->string, "Hi there!") == 0);
	dlist_free(l1);
	test_end();
	
	
	test_start("dlist_erase()");
	l1 = dlist_new();
	dlist_addcs(l1, "Hi there!");
	test_assert(l1->size == 1);
	test_assert(l1->front == l1->back);
	test_assert(dcmpcs(l1->front->string, "Hi there!") == 0);
	dlist_erase(l1, l1->front);
	test_assert(l1->front == NULL);
	test_assert(l1->back == NULL);
	test_assert(l1->size == 0);
	dlist_free(l1);
	test_end();
	
	
	test_start("dlist_remove()");
	l1 = dlist_new();
	dlist_addcs(l1, "node 1");
	dlist_addcs(l1, "node 2");
	dlist_addcs(l1, "node 3");
	
	test_assert(l1->size == 3);
	n1 = l1->front->next;
	dlist_remove(l1, n1);
	
	test_assert(l1->size == 2);
	test_assert(dcmpcs(n1->string, "node 2") == 0);
	
	dlist_addnode(l1, n1);
	test_assert(l1->size == 3);
	test_assert(dcmpcs(l1->back->string, "node 2") == 0);
	
	dlist_free(l1);
	test_end();
	
	
	test_start("dlist_find()");
	l1 = dlist_new();
	dlist_addcs(l1, "node 1");
	dlist_addcs(l1, "node 2");
	dlist_addcs(l1, "node 3");

	test_end();
	
	test_start("dlist_join()");
	test_end();
	
	test_start("dlist_joinc()");
	test_end();
	
	test_start("dlist_joincs()");
	test_end();
	
	test_start("dlist_equals()");
	test_end();
	
	test_start("dsplit()");
	test_end();
	
	test_start("dsplit_on_cs()");
	test_end();
	
	test_start("dsplitcs()");
	test_end();
	
	test_start("dsplitcs_on_cs()");
	test_end();
	
	printf("Success.\n");
	return 0;
}
