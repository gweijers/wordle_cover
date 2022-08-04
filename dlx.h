/************************************************************************\
 *  Sudoku solver and generator                                         *
 *                                                                      *
 *  © 2016-2017 Gé Weijers. All Rights Reserved                         *
 *                                                                      *
 *  Dancing Links exact cover solver                                    *
 *                                                                      *
\************************************************************************/
/*
 *  Dancing Links exact cover solver
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
