#include "dstr.h"
#include "tests_utils.h"

int main(int argc, char* argv[]) {
	dstring *s1, *s2, *s3, *s4, *s5, *s6;
	
	/* dstring* dnew(); */
	test_start("dnew()");
	s1 = dnew();
	test_assert(s1 != NULL);
	test_assert(s1->data != NULL);
	test_assert(s1->size > 0);
	test_assert(s1->len == 0);
	test_assert(s1->data[0] == 0);
	test_end();
	
	/* dstring* dnewcopy(const dstring* str); */
	test_start("dnewcopy()");
	s2 = dnewcopy(s1);
	test_assert(s2 != NULL);
	test_assert(s2->data != NULL);
	test_assert(s2->size > 0);
	test_assert(s2->len == 0);
	test_assert(s2->data[0] == 0);
	s1->data[0] = 'A';
	s1->data[1] = 0;
	s1->len = 1;
	dfree(s2);
	s2 = dnewcopy(s1);
	test_assert(s2->len == 1);
	test_assert(strcmp(s1->data, s2->data) == 0);
	test_end();
	
	/* void dfree(dstring* str); */
	test_start("dfree()");
	dfree(s1);
	dfree(s2);
	test_assert(1);
	test_end();
	
	/* dstring* dfromc(int chr); */
	test_start("dfromc()");
	s1 = dfromc('X');
	test_assert(s1 != NULL);
	test_assert(s1->len == 1);
	test_assert(s1->data != NULL);
	test_assert(strcmp(s1->data, "X") == 0);
	dfree(s1);
	test_end();
	
	/* dstring* dfromcs(const char* str); */
	test_start("dfromcs()");
	s1 = dfromcs("Hello World!");
	test_assert(s1 != NULL);
	test_assert(s1->len == 12);
	test_assert(s1->data != NULL);
	test_assert(strcmp(s1->data, "Hello World!") == 0);
	dfree(s1);
	test_end();
	
	/* dstring* dfrommem(const void* mem, size_t size); */
	test_start("dfrommem()");
	s1 = dfrommem("Hello World!", 12);
	test_assert(s1 != NULL);
	test_assert(s1->len == 12);
	test_assert(s1->data != NULL);
	test_assert(strcmp(s1->data, "Hello World!") == 0);
	dfree(s1);
	test_end();
	
	/* dstring* dcpy(dstring* dst, const dstring* src); */
	test_start("dcpy()");
	s1 = dfromcs("Hello ");
	s2 = dfromcs("World!!!");
	s3 = dcpy(s1, s2);
	
	test_assert(s1 == s3);
	test_assert(s1->len == 8);
	test_assert(s1->data != NULL);
	test_assert(strcmp(s1->data, "World!!!") == 0);
	dfree(s1);
	dfree(s2);
	test_end();
	
	/* dstring* dcpyc(dstring* dst, int src); */
	test_start("dcpyc()");
	s1 = dfromcs("Hello");
	s2 = dcpyc(s1, '!');
	
	test_assert(s2 == s1);
	test_assert(s1->len == 1);
	test_assert(s1->data != NULL);
	test_assert(strcmp(s1->data, "!") == 0);
	dfree(s1);
	test_end();

	/* dstring* dcpycs(dstring* dst, const char* src); */
	test_start("dcpycs");
	s1 = dfromcs("Hello");
	s2 = dcpycs(s1, "Hola!");
	
	test_assert(s2 == s1);
	test_assert(s1->len == 5);
	test_assert(s1->data != NULL);
	test_assert(strcmp(s1->data, "Hola!") == 0);
	dfree(s1);
	test_end();
	
	/* dstring* dcpymem(dstring* dst, const void* mem, size_t size); */
	test_start("dcpymem");
	s1 = dfromcs("Hello");
	s2 = dcpymem(s1, "Foo!", 4);
	
	test_assert(s2 == s1);
	test_assert(s1->len == 4);
	test_assert(s1->data != NULL);
	test_assert(strcmp(s1->data, "Foo!") == 0);
	dfree(s1);
	test_end();
	
	/* dstring* dncpy(dstring* dst, const dstring* src, dstrlen_t n); */
	test_start("dncpy");
	s1 = dfromcs("Hello");
	s2 = dfromcs("World!");
	s1 = dncpy(s1, s2, 5);
	
	test_assert(s1->len == 5);
	test_assert(strcmp(s1->data, "World") == 0);
	
	s1 = dncpy(s1, s2, 20);
	test_assert(s1->len == 6);
	test_assert(strcmp(s1->data, "World!") == 0);
	
	dfree(s1);
	dfree(s2);
	test_end();
	
	/* dstring* dncpycs(dstring* dst, const char* src, dstrlen_t n); */
	test_start("dncpycs");
	s1 = dfromcs("Hello");
	s1 = dncpycs(s1, "World!", 5);
	
	test_assert(s1->len == 5);
	test_assert(strcmp(s1->data, "World") == 0);
	
	s1 = dncpycs(s1, "World!", 20);
	test_assert(s1->len == 6);
	test_assert(strcmp(s1->data, "World!") == 0);
	
	dfree(s1);
	test_end();
	
	/* dstring* dcat(dstring* dst, const dstring* src); */
	test_start("dcat");
	s1 = dfromcs("Hello ");
	s2 = dfromcs("World!");
	s3 = dcat(s1, s2);
	
	test_assert(s1 == s3);
	test_assert(s1->len == 12);
	test_assert(strcmp(s1->data, "Hello World!") == 0);
	
	dfree(s1);
	dfree(s2);
	test_end();
	
	/* dstring* dcatc(dstring* dst, int src); */
	test_start("dcatc");
	s1 = dfromcs("Hello");
	s3 = dcatc(s1, '!');
	
	test_assert(s1 == s3);
	test_assert(s1->len == 6);
	test_assert(strcmp(s1->data, "Hello!") == 0);
	
	dfree(s1);
	test_end();
	
	/* dstring* dcatcs(dstring* dst, const char* src); */
	test_start("dcatcs");
	s1 = dfromcs("Hello ");
	s3 = dcatcs(s1, "World!");
	
	test_assert(s1 == s3);
	test_assert(s1->len == 12);
	test_assert(strcmp(s1->data, "Hello World!") == 0);
	
	dfree(s1);
	test_end();
	
	/* dstring* dcatmem(dstring* dst, const void* src, size_t size); */
	test_start("dcatmem");
	s1 = dfromcs("Hello ");
	s3 = dcatmem(s1, "Guys!", 5);
	
	test_assert(s1 == s3);
	test_assert(s1->len == 11);
	test_assert(strcmp(s1->data, "Hello Guys!") == 0);
	
	dfree(s1);
	test_end();
	
	/* dstring* dncat(dstring* dst, const dstring* src, dstrlen_t n); */
	test_start("dncat");
	s2 = dfromcs("Guys!");
	s1 = dfromcs("Hello ");
	s3 = dncat(s1, s2, 4);
	
	test_assert(s1 == s3);
	test_assert(s1->len == 10);
	test_assert(strcmp(s1->data, "Hello Guys") == 0);
	dfree(s1);
	
	s1 = dfromcs("Hello ");
	s3 = dncat(s1, s2, 8);
	
	test_assert(s1 == s3);
	test_assert(s1->len == 11);
	test_assert(strcmp(s1->data, "Hello Guys!") == 0);
	dfree(s1);
	
	dfree(s2);
	test_end();
	
	/* dstring* dncatcs(dstring* dst, const char* src, dstrlen_t n); */
	test_start("dncatcs");
	s1 = dfromcs("Hello ");
	s3 = dncatcs(s1, "Guys!", 4);
	
	test_assert(s1 == s3);
	test_assert(s1->len == 10);
	test_assert(strcmp(s1->data, "Hello Guys") == 0);
	dfree(s1);
	
	s1 = dfromcs("Hello ");
	s3 = dncatcs(s1, "Guys!", 8);
	
	test_assert(s1 == s3);
	test_assert(s1->len == 11);
	test_assert(strcmp(s1->data, "Hello Guys!") == 0);
	dfree(s1);
	test_end();
	
	/* int dcmp(const dstring* a, const dstring* b); */
	test_start("dcmp");
	s1 = dfromcs("ddd");
	s2 = dfromcs("aaa");
	s3 = dfromcs("zzz");
	s4 = dfromcs("bo");
	s5 = dfromcs("aaaa");
	s6 = dfromcs("ddd");
	
	test_assert(dcmp(s1, s2) != 0);
	test_assert(dcmp(s1, s3) != 0);
	test_assert(dcmp(s1, s4) != 0);
	test_assert(dcmp(s1, s5) != 0);
	test_assert(dcmp(s1, s6) == 0);
	
	dfree(s1);
	dfree(s2);
	dfree(s3);
	dfree(s4);
	dfree(s5);
	dfree(s6);
	test_end();
	
	/* int dcmpc(const dstring* a, int b); */
	test_start("dcmpc");
	s1 = dfromc('X');
	test_assert(dcmpc(s1, 'a') != 0);
	test_assert(dcmpc(s1, 'x') != 0);
	test_assert(dcmpc(s1, 'X') == 0);
	dfree(s1);
	test_end();
	
	/* int dcmpcs(const dstring* a, const char* b); */
	test_start("dcmpcs");
	s1 = dfromcs("Foo");
	test_assert(dcmpcs(s1, "FO") != 0);
	test_assert(dcmpcs(s1, "barr") != 0);
	test_assert(dcmpcs(s1, "Foo") == 0);
	dfree(s1);
	test_end();
	
	/* int dcmpmem(const dstring* a, const void* b, size_t size); */
	test_start("dcmpmem");
	s1 = dfromcs("Foo");
	test_assert(dcmpmem(s1, "FO", 2) != 0);
	test_assert(dcmpmem(s1, "barr", 4) != 0);
	test_assert(dcmpmem(s1, "Foo", 3) == 0);
	dfree(s1);
	test_end();
	
	/* dstring* dsub(const dstring* a, dstrlen_t start, dstrlen_t length); */
	test_start("dsub");
	s1 = dfromcs("hello world!");
	s2 = dsub(s1, 0, 5);
	s3 = dsub(s1, 6, 6);
	s4 = dsub(s1, 6, 12);
	s5 = dsub(s1, 20, 10);
	
	test_assert(dcmpcs(s2, "hello") == 0);
	test_assert(dcmpcs(s3, "world!") == 0);
	test_assert(dcmpcs(s4, "world!") == 0);
	test_assert(dcmpcs(s5, "") == 0);
	
	dfree(s1);
	dfree(s2);
	dfree(s3);
	dfree(s4);
	dfree(s5);
	test_end();
	
	/* int dstartswith(const dstring* a, const dstring* b); */
	test_start("dstartswith");
	s1 = dfromcs("Is this ok?");
	s2 = dfromcs("Is");
	s3 = dfromcs("bla");
	
	test_assert(dstartswith(s1, s2));
	test_assert(!dstartswith(s1, s3));
	
	dfree(s1);
	dfree(s2);
	dfree(s3);
	test_end();
	
	/* int dstartswithc(const dstring* a, int b); */
	test_start("dstartswithc");
	s1 = dfromcs("Is this ok?");
	
	test_assert(dstartswithc(s1, 'I'));
	test_assert(!dstartswithc(s1, 'O'));
	
	dfree(s1);
	test_end();
	
	/* int dstartswithcs(const dstring* a, const char* b); */
	test_start("dstartswithcs");
	s1 = dfromcs("Is this ok?");
	
	test_assert(dstartswithcs(s1, "Is"));
	test_assert(!dstartswithcs(s1, "is"));
	
	dfree(s1);
	test_end();
	
	/* int dendswith(const dstring* a, const dstring* b); */
	test_start("dendswith");
	s1 = dfromcs("Is this ok?");
	s2 = dfromcs("ok?");
	s3 = dfromcs("Ok?");
	
	test_assert(dendswith(s1, s2));
	test_assert(!dendswith(s1, s3));
	
	dfree(s1);
	dfree(s2);
	dfree(s3);
	test_end();
	
	/* int dendswithc(const dstring* a, int b); */
	test_start("dendswithc");
	s1 = dfromcs("Is this ok?");
	
	test_assert(dendswithc(s1, '?'));
	test_assert(!dendswithc(s1, '!'));
	
	dfree(s1);
	test_end();
	
	/* int dendswithcs(const dstring* a, const char* b); */
	test_start("dendswithcs");
	s1 = dfromcs("Is this ok?");
	
	test_assert(dendswithcs(s1, "ok?"));
	test_assert(!dendswithcs(s1, "ok!"));
	
	dfree(s1);
	test_end();
	
	/* int dpos(const dstring* a, const dstring* b, dstrlen_t start); */
	test_start("dpos");
	s1 = dfromcs("this is a good place");
	s2 = dfromcs("is");
	s3 = dfromcs("bla");
	
	test_assert(dpos(s1, s2, 0) == 2);
	test_assert(dpos(s1, s2, 4) == 5);
	test_assert(dpos(s1, s2, 7) == -1);
	test_assert(dpos(s1, s3, 44) == -1);
	
	dfree(s1);
	dfree(s2);
	dfree(s3);
	test_end();
	
	/* int dposc(const dstring* a, int b, dstrlen_t start); */
	test_start("dposc");
	s1 = dfromcs("this is a good place");
	
	test_assert(dposc(s1, 'i', 0) == 2);
	test_assert(dposc(s1, 'i', 4) == 5);
	test_assert(dposc(s1, 'j', 7) == -1);
	test_assert(dposc(s1, 'x', 44) == -1);
	
	dfree(s1);
	test_end();
	
	/* int dposcs(const dstring* a, const char* b, dstrlen_t start); */
	test_start("dposcs");
	s1 = dfromcs("this is a good place");
	
	test_assert(dposcs(s1, "is", 0) == 2);
	test_assert(dposcs(s1, "is", 4) == 5);
	test_assert(dposcs(s1, "is", 7) == -1);
	test_assert(dposcs(s1, "bla", 44) == -1);
	
	dfree(s1);
	test_end();
	
	
	return 0;
}
