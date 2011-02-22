/*
 * Copyright (c) 2009-2010 Przemysław Grzywacz
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
#include "dstr.h"
#include <stdlib.h>
#include <string.h>




#define put_z(s)	s->data[s->len] = 0
#define align(l)	((((l) >> 6) + 1) << 6)


static void set_size(dstring* s, dstrlen_t size) {
	dstrlen_t new_size = align(size);
	if (s->size >= new_size && s->size <= new_size+32) {
		return;
	}

	free(s->data);
	s->data = (char*)malloc(new_size);
	s->size = new_size;
}

static void add_size(dstring* s, dstrlen_t size) {
	dstrlen_t new_size;
	char* tmp;
	if (s->len + size <= s->size) {
		return;
	}

	new_size = align(s->len + size);
	tmp = (char*)malloc(new_size);
	memcpy(tmp, s->data, s->len+1);
	free(s->data);
	s->data = tmp;
	s->size = new_size;
}





dstring* dnew() {
	dstring* s = (dstring*)malloc(sizeof(dstring));
	s->len = 0;
	s->size = 32;
	s->data = (char*)malloc(32);
	return s;
}

dstring* dnewcopy(const dstring* str) {
	dstring* s = (dstring*)malloc(sizeof(dstring));
	s->len = str->len;
	s->size = align(str->len);
	s->data = (char*)malloc(s->size);
	memcpy(s->data, str->data, str->len+1);
	return s;
}

void dfree(dstring* str) {
	free(str->data);
	free(str);
}

dstring* dclear(dstring* str) {
	str->data[0] = 0;
	str->len = 0;
	put_z(str);
	return str;
}

dstring* dfromc(int chr) {
	dstring* s = dnew();
	s->data[0] = chr;
	s->len = 1;
	put_z(s);
	return s;
}

dstring* dfromcs(const char* str) {
	size_t len = strlen(str);
	dstring* s = (dstring*)malloc(sizeof(dstring));
	s->size = align(len);
	s->len = len;
	s->data = (char*)malloc(s->size);
	memcpy(s->data, str, len);
	put_z(s);
	return s;
}

dstring* dfrommem(const void* mem, size_t size) {
	dstring* s = (dstring*)malloc(sizeof(dstring));
	s->size = align(size);
	s->len = size;
	s->data = (char*)malloc(s->size);
	memcpy(s->data, mem, size);
	put_z(s);
	return s;
}

dstring* dcpy(dstring* dst, const dstring* src) {
	set_size(dst, src->len + 1);
	memcpy(dst->data, src->data, src->len+1);
	dst->len = src->len;
	return dst;
}

dstring* dcpyc(dstring* dst, int src) {
	set_size(dst, 1);
	dst->data[0] = src;
	dst->data[1] = 0;
	dst->len = 1;
	return dst;

}

dstring* dcpycs(dstring* dst, const char* src) {
	size_t len = strlen(src);
	set_size(dst, len+1);
	memcpy(dst->data, src, len+1);
	dst->len = len;
	return dst;
}

dstring* dcpymem(dstring* dst, const void* mem, size_t size) {
	set_size(dst, size+1);
	memcpy(dst->data, mem, size);
	dst->len = size;
	put_z(dst);
	return dst;
}

dstring* dncpy(dstring* dst, const dstring* src, dstrlen_t n) {
	if (src->len < n) n = src->len;
	set_size(dst, n+1);
	memcpy(dst->data, src->data, n);
	dst->len = n;
	put_z(dst);
	return dst;
}

dstring* dncpycs(dstring* dst, const char* src, dstrlen_t n) {
	size_t len = strlen(src);
	if (len < n) n = len;
	set_size(dst, n+1);
	memcpy(dst->data, src, n);
	dst->len = n;
	put_z(dst);
	return dst;
}

dstring* dcat(dstring* dst, const dstring* src) {
	add_size(dst, src->len+1);
	memcpy(dst->data+dst->len, src->data, src->len+1);
	dst->len+= src->len;
	return dst;
}

dstring* dcatc(dstring* dst, int src) {
	add_size(dst, 1);
	dst->data[dst->len] = src;
	dst->len++;
	put_z(dst);
	return dst;
}

dstring* dcatcs(dstring* dst, const char* src) {
	size_t len = strlen(src);
	add_size(dst, len+1);
	memcpy(dst->data+dst->len, src, len+1);
	dst->len+= len;
	return dst;
}

dstring* dcatmem(dstring* dst, const void* src, size_t size) {
	add_size(dst, size);
	memcpy(dst->data+dst->len, src, size);
	dst->len+= size;
	put_z(dst);
	return dst;
}

dstring* dncat(dstring* dst, const dstring* src, dstrlen_t n) {
	if (src->len < n) n = src->len;
	memcpy(dst->data+dst->len, src->data, n);
	dst->len+= n;
	put_z(dst);
	return dst;
}

dstring* dncatcs(dstring* dst, const char* src, dstrlen_t n) {
	size_t len = strlen(src);
	if (len < n) n = len;
	memcpy(dst->data+dst->len, src, n);
	dst->len+= n;
	put_z(dst);
	return dst;
}


int dcmp(const dstring* a, const dstring* b) {
	if (a->len < b->len) {
		return -1;
	} else if (a->len > b->len) {
		return 1;
	} else {
		return memcmp(a->data, b->data, a->len);
	}
}

int dcmpc(const dstring* a, int b) {
	if (a->len == 0) {
		return -1;
	} else if (a->len > 1) {
		return 1;
	} else if (a->data[0] < b) {
		return -1;
	} else if (a->data[0] > b) {
		return 1;
	} else {
		return 0;
	}
}

int dcmpcs(const dstring* a, const char* b) {
	size_t len = strlen(b);
	if (a->len < len) {
		return -1;
	} else if (a->len > len) {
		return 1;
	} else {
		return memcmp(a->data, b, len);
	}
}

int dcmpmem(const dstring* a, const void* b, size_t size) {
	if (a->len < size) {
		return -1;
	} else if (a->len > size) {
		return 1;
	} else {
		return memcmp(a->data, b, a->len);
	}
}


dstring* dsub(const dstring* a, dstrlen_t start, dstrlen_t length) {
	if (start > a->len) {
		return dnew();
	} else if (start+length > a->len) {
		length = a->len - start;
	}

	return dfrommem(a->data + start, length);
}

int dstartswith(const dstring* a, const dstring* b) {
	if (b->len == 0) {
		return 1;
	} else if (a->len < b->len) {
		return 0;
	} else if (memcmp(a->data, b->data, b->len) == 0) {
		return 1;
	} else {
		return 0;
	}
}

int dstartswithc(const dstring* a, int b) {
	if (a->len == 0) {
		return 0;
	} else if (a->data[0] == b) {
		return 1;
	} else {
		return 0;
	}
}

int dstartswithcs(const dstring* a, const char* b) {
	size_t len = strlen(b);
	if (len == 0) {
		return 1;
	} else if (a->len < len) {
		return 0;
	} else if (memcmp(a->data, b, len) == 0) {
		return 1;
	} else {
		return 0;
	}
}

int dendswith(const dstring* a, const dstring* b) {
	if (b->len == 0) {
		return 1;
	} else if (a->len < b->len) {
		return 0;
	} else if (memcmp(a->data + a->len - b->len, b->data, b->len) == 0) {
		return 1;
	} else {
		return 0;
	}

}

int dendswithc(const dstring* a, int b) {
	if (a->len == 0) {
		return 0;
	} else if (a->data[a->len-1] == b) {
		return 1;
	} else {
		return 0;
	}
}

int dendswithcs(const dstring* a, const char* b) {
	int len = strlen(b);
	if (len == 0) {
		return 1;
	} else if (a->len < len) {
		return 0;
	} else if (memcmp(a->data + a->len - len, b, len) == 0) {
		return 1;
	} else {
		return 0;
	}
}

int dpos(const dstring* a, const dstring* b, dstrlen_t start) {
	dstrlen_t last, i;
	if (a->len < b->len) {
		return -1;
	}

	last = a->len - b->len;
	if (start > last) {
		return -1;
	}

	for(i = start; i <= last; i++) {
		if (memcmp(a->data+i, b->data, b->len) == 0) {
			return i;
		}
	}

	return -1;
}

int dposc(const dstring* a, int b, dstrlen_t start) {
	dstrlen_t last, i;
	if (a->len == 0) {
		return -1;
	}

	last = a->len-1;
	if (start > last) {
		return -1;
	}

	for(i = start; i <= last; i++) {
		if (a->data[i] == b) {
			return i;
		}
	}

	return -1;
}

int dposcs(const dstring* a, const char* b, dstrlen_t start) {
	dstrlen_t last, i;
	size_t len = strlen(b);
	if (a->len < len) {
		return -1;
	}

	last = a->len - len;
	if (start > last) {
		return -1;
	}

	for(i = start; i <= last; i++) {
		if (memcmp(a->data+i, b, len) == 0) {
			return i;
		}
	}

	return -1;

}

