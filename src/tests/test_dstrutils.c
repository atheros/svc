#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dstr.h"
#include "dstrutils.h"

#include "tests_utils.h"

int main(int argc, char* argv[]) {
	dstrlist *l1, *l2;
	dstrnode *n1;
	dstring *s1, *s2;
	
	
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
	l1 = dlist_new();
	dlist_addcs(l1, "hello");
	dlist_addcs(l1, "world");
	s1 = dfromcs(" ");
	
	s2 = dlist_join(l1, s1);
	
	test_assert(s2 != NULL);
	test_assert(s2->data != NULL);
	test_assert(dcmpcs(s2, "hello world") == 0);
	
	dfree(s2);
	dfree(s1);
	dlist_free(l1);
	test_end();

	
	test_start("dlist_joinc()");
	l1 = dlist_new();
	dlist_addcs(l1, "hello");
	dlist_addcs(l1, "world");
	
	s1 = dlist_joinc(l1, ' ');
	
	test_assert(s1 != NULL);
	test_assert(s1->data != NULL);
	test_assert(dcmpcs(s1, "hello world") == 0);
	
	dfree(s1);
	dlist_free(l1);
	test_end();

	
	test_start("dlist_joincs()");
	l1 = dlist_new();
	dlist_addcs(l1, "hello");
	dlist_addcs(l1, "world");
	
	s1 = dlist_joincs(l1, " - ");
	
	test_assert(s1 != NULL);
	test_assert(s1->data != NULL);
	test_assert(dcmpcs(s1, "hello - world") == 0);
	
	dfree(s1);
	dlist_free(l1);
	test_end();
	
	test_start("dlist_equals()");
	l1 = dlist_new();
	l2 = dlist_new();
	
	test_assert(dlist_equals(l1, l2));
	
	dlist_addcs(l1, "foo");
	dlist_addcs(l2, "foo");
	
	test_assert(dlist_equals(l1, l2));
	
	dlist_addcs(l1, "foo");
	dlist_addcs(l2, "bar");
	
	test_assert(dlist_equals(l1, l2) == 0);
	
	dlist_erase(l1, l1->back);
	dlist_addcs(l1, "bar");
	
	test_assert(dlist_equals(l1, l2));
	
	dlist_free(l1);
	dlist_free(l2);
	
	test_end();
	
	
	test_start("dsplit()");
	s1 = dfromcs("foo bar");
	s2 = dfromcs(" ");
	l1 = dsplit(s1, s2, 0);
	
	test_assert(l1 != NULL);
	test_assert(l1->size == 2);
	test_assert(strcmp(l1->front->string->data, "foo") == 0);
	test_assert(strcmp(l1->back->string->data, "bar") == 0);
	
	dlist_free(l1);
	
	l1 = dsplit(s1, s2, 1);
	test_assert(l1 != NULL);
	test_assert(l1->size == 1);
	test_assert(strcmp(l1->front->string->data, "foo bar") == 0);
	dlist_free(l1);
	
	dfree(s1);
	dfree(s2);
	test_end();
	
	
	test_start("dsplit_on_cs()");
	s1 = dfromcs("foo bar");
	l1 = dsplit_on_cs(s1, " ", 0);
	
	test_assert(l1 != NULL);
	test_assert(l1->size == 2);
	test_assert(strcmp(l1->front->string->data, "foo") == 0);
	test_assert(strcmp(l1->back->string->data, "bar") == 0);
	
	dlist_free(l1);
	
	l1 = dsplit_on_cs(s1, " ", 1);
	test_assert(l1 != NULL);
	test_assert(l1->size == 1);
	test_assert(strcmp(l1->front->string->data, "foo bar") == 0);
	test_end();
	
	
	test_start("dsplitcs()");
	s2 = dfromcs(" ");
	l1 = dsplitcs("foo bar", s2, 0);
	
	test_assert(l1 != NULL);
	test_assert(l1->size == 2);
	test_assert(strcmp(l1->front->string->data, "foo") == 0);
	test_assert(strcmp(l1->back->string->data, "bar") == 0);
	
	dlist_free(l1);
	
	l1 = dsplitcs("foo bar", s2, 1);
	test_assert(l1 != NULL);
	test_assert(l1->size == 1);
	test_assert(strcmp(l1->front->string->data, "foo bar") == 0);
	dlist_free(l1);
	
	dfree(s2);
	test_end();

	
	test_start("dsplitcs_on_cs()");
	l1 = dsplitcs_on_cs("foo bar", " ", 0);
	
	test_assert(l1 != NULL);
	test_assert(l1->size == 2);
	test_assert(strcmp(l1->front->string->data, "foo") == 0);
	test_assert(strcmp(l1->back->string->data, "bar") == 0);
	
	dlist_free(l1);
	
	l1 = dsplitcs_on_cs("foo bar", " ", 1);
	test_assert(l1 != NULL);
	test_assert(l1->size == 1);
	test_assert(strcmp(l1->front->string->data, "foo bar") == 0);
	dlist_free(l1);
	test_end();
	
	printf("Success.\n");
	return 0;
}
