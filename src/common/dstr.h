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

/**
 * Dynamic strings library.
 *
 * @version 0.3
 */

#ifndef DSTR_H_
#define DSTR_H_

#ifdef __cplusplus
#include <cstring>
#else
#include <string.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif



typedef unsigned int dstrlen_t;

struct _dstring {
	dstrlen_t len;
	dstrlen_t size;
	char* data;
};

typedef struct _dstring dstring;


dstring* dnew();
dstring* dnewcopy(const dstring* str);
void dfree(dstring* str);
dstring* dclear(dstring* str);
dstring* dfromc(int chr);
dstring* dfromcs(const char* str);
dstring* dfrommem(const void* mem, size_t size);
dstring* dcpy(dstring* dst, const dstring* src);
dstring* dcpyc(dstring* dst, int src);
dstring* dcpycs(dstring* dst, const char* src);
dstring* dcpymem(dstring* dst, const void* mem, size_t size);
dstring* dncpy(dstring* dst, const dstring* src, dstrlen_t n);
dstring* dncpycs(dstring* dst, const char* src, dstrlen_t n);
dstring* dcat(dstring* dst, const dstring* src);
dstring* dcatc(dstring* dst, int src);
dstring* dcatcs(dstring* dst, const char* src);
dstring* dcatmem(dstring* dst, const void* src, size_t size);
dstring* dncat(dstring* dst, const dstring* src, dstrlen_t n);
dstring* dncatcs(dstring* dst, const char* src, dstrlen_t n);
int dcmp(const dstring* a, const dstring* b);
int dcmpc(const dstring* a, int b);
int dcmpcs(const dstring* a, const char* b);
int dcmpmem(const dstring* a, const void* b, size_t size);
dstring* dsub(const dstring* a, dstrlen_t start, dstrlen_t length);
int dstartswith(const dstring* a, const dstring* b);
int dstartswithc(const dstring* a, int b);
int dstartswithcs(const dstring* a, const char* b);
int dendswith(const dstring* a, const dstring* b);
int dendswithc(const dstring* a, int b);
int dendswithcs(const dstring* a, const char* b);
int dpos(const dstring* a, const dstring* b, dstrlen_t start);
int dposc(const dstring* a, int b, dstrlen_t start);
int dposcs(const dstring* a, const char* b, dstrlen_t start);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* DSTR_H_ */
