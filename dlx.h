/*
Copyright 2022 Ge' Weijers

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following
conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer
   in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

#ifndef DLX_H
#define DLX_H

#ifdef __cplusplus
extern "C" {
#endif


#include <inttypes.h>

// value stored by user of library in a row
typedef uintptr_t value_t;

#define VALUE_MAX UINTPTR_MAX

// type of solution printing routine
typedef void (*success_t)(unsigned long count, const value_t *, unsigned);

// setup the cover matrix
void init_matrix(unsigned verbose);

// release storage
void free_matrix(void);

// add a new row to the problem
void add_row (value_t value, const unsigned v[], unsigned vlen);

// search for solutions
unsigned long search(unsigned long max, success_t);


#ifdef __cplusplus
}
#endif

#endif //DLX_H

// vim:ts=4 sts=4 sw=4 et:
