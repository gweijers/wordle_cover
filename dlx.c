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

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <inttypes.h>
#include <limits.h>

#include "dlx.h"

/*
 *  node structure representing cover matrix
 */
typedef struct node *link_ptr, node_t;

struct node {
    link_ptr left, right, up, down;
    link_ptr header;
    value_t value;
};


/*
 *  link two node left to right
 */
static inline void link_lr(link_ptr l, link_ptr r)
{
    l->right = r; r->left = l;
}


/*
 *  remove node 'p'
 */
static inline void unlink_lr(link_ptr p)
{
    link_lr(p->left, p->right);
}


/*
 *  restore removed node 'p'
 */
static inline void relink_lr(link_ptr p)
{
    p->left->right = p; p->right->left = p;
}


/*
 *  link two node up and down
 */
static inline void link_tb(link_ptr t, link_ptr b)
{
    t->down = b; b->up = t;
}


/*
 *  remove node 'p'
 */
static inline void unlink_tb(link_ptr p)
{
    link_tb(p->up, p->down);
}


/*
 *  restore removed node 'p'
 */
static inline void relink_tb(link_ptr p)
{
    p->up->down = p; p->down->up = p;
}


/*
 *  cover operation
 *  removes column and its nodes from matrix
 */
static void cover_column(link_ptr c)
{
    link_ptr r = c->down;
    unlink_lr(c);
    while (r != c) {
        link_ptr p = r->right;
        while (p != r) {
            unlink_tb(p);
            p->header->value--;
            p = p->right;
        }
        r = r->down;
    }
}


/*
 *  uncover operation
 *  restores column and its nodes to matrix
 */
static void uncover_column(link_ptr c)
{
    link_ptr r = c->up;
    while (r != c) {
        link_ptr p = r->left;
        while (p != r) {
            relink_tb(p);
            p->header->value++;
            p = p->left;
        }
        r = r->up;
    }
    relink_lr(c);
}


/*
 *  'no-fail' memory (re-)allocator
 */
static void * xcalloc(void *o, size_t n, size_t s)
{
    void *p = realloc(o, n * s);
    if (p == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }
    return p;
}

/*
 * maximum of two unsigned values
 */
static inline unsigned umax(unsigned a, unsigned b)
{
    return (a > b ? a : b);
}


/*
 *  the cover matrix
 */
static node_t header = { &header, &header, &header, &header, 0 , 0 };
static link_ptr *colref = NULL;
static unsigned colref_alloc = 0;
static unsigned colcount, rowcount, nodecount;


/*
 *  release all the memory
 */
void free_matrix(void)
{
    // free all rows
    link_ptr p = header.down;
    while (p != &header) {
        link_ptr q = p->down;
        free(p);
        p = q;
    }
    // free all column headers
    p = header.right;
    while (p != &header) {
        link_ptr q = p->right;
        free(p);
        p = q;
    }
    // free column index table
    free(colref);
    // recreate initial conditions
    colref = NULL;
    colref_alloc = 0;
    link_lr(&header, &header);
    link_tb(&header, &header);
    header.header = NULL;
    header.value = 0;
    colcount = rowcount = nodecount = 0;
}

// verbosity level
static unsigned verbose = 0;

/*
 * initialize matrix (just frees previous one)
 */
void init_matrix(unsigned v)
{
    free_matrix();
    verbose = v;
}


/*
 * add a matrix row. v[] is a list of increasing column numbers
 */
void add_row (value_t value, const unsigned v[], unsigned vlen)
{
    if (vlen > 0) {
        link_ptr last;
        unsigned i;
        unsigned cols_needed = 0;
        if (verbose >= 3) {
            fprintf(stderr, "==%u:", rowcount);
            for (i = 0; i < vlen; i++)
                fprintf(stderr, " %u", v[i]);
            putc('\n', stderr);
        }
        for (i = 0; i < vlen; i++) {
            if (v[i] < cols_needed) {
                fprintf(stderr, "non-monotonic column list");
                abort(); // core dump
                return;
            }
            cols_needed = v[i]+1u;
        }
        link_ptr nodes = (link_ptr)xcalloc(NULL, vlen+1, sizeof(node_t));
        // link row into list of rows used for freeing memory
        link_tb(nodes, header.down);
        link_tb(&header, nodes);

        // grow the # of columns if needed
        if (colcount < cols_needed) {
            // grow the colref array
            if (colref_alloc < cols_needed) {
                colref_alloc = umax(cols_needed, 2*colref_alloc);
                colref = (link_ptr *)xcalloc(colref, colref_alloc, sizeof(link_ptr));
            }
            // add column header nodes
            while (colcount < cols_needed) {
                link_ptr p = (link_ptr)xcalloc(NULL, 1, sizeof(node_t));
                link_lr(header.left, p);
                link_lr(p, &header);
                link_tb(p, p);
                p->value = 0;
                p->header = 0;
                colref[colcount++] = p;
            }
        }

        // we're using an extra node to link the rows together,
        // and temporarily use it to link the row nodes
        last = nodes;

        for (i = 0; i < vlen; i++) {
            link_ptr p, h;
            unsigned col = v[i];
            // link the node to its header node
            // and to its neighbors
            h = colref[col];
            p = &nodes[i+1];
            p->header = h;
            p->value = value;
            link_tb(h->up, p);
            link_tb(p, h);
            h->value++;
            link_lr(last, p);
            last = p;
        }
        // complete the circular link
        link_lr(last, nodes->right);
        // update the row and node counts
        rowcount++;
        nodecount += vlen;
    }
}


/*
 *  backtracking search
 */
static unsigned long rec_search(unsigned k, value_t O[], unsigned long pos,
                                unsigned long max, success_t success)
{
    // use heuristic to find next column to cover
    value_t l = ~(value_t)0;
    link_ptr col = NULL, r, j;
    for (r = header.right; r != &header; r = r->right)
        if (r->value < l) {
            l = r->value;
            col = r;
        }
    // no column left: we found a result
    if (col == NULL) {
        success(pos, O, k);
        return 1u;
    }
    // cover the column
    cover_column(col);
    // count solutions
    unsigned long solcount = 0u;
    // try all possible values
    for (r = col->down; r != col && pos + solcount < max; r = r->down) {
        O[k] = r->value;
        for (j = r->right; j != r; j = j->right)
            cover_column(j->header);
        solcount += rec_search(k+1u, O, pos + solcount, max, success);
        for (j = r->left; j != r; j = j->left)
            uncover_column(j->header);
    }
    // uncover the column
    uncover_column(col);
    // return result count
    return solcount;
}


/*
 *  start search
 */
unsigned long search(unsigned long max, success_t success)
{
    // trivial case
    if (max == 0u)
        return 0u;

    // allocate result buffer
    value_t *O = (value_t *)xcalloc(NULL, rowcount, sizeof(value_t));

    // show some statistics of required
    if (verbose) {
        unsigned i;
        fprintf(stderr, "Cover matrix: %u cols %u rows %u nodes\n",
               colcount, rowcount, nodecount);
        fprintf(stderr, "counts:");
        for (i = 0; i < colcount; i++)
            fprintf(stderr, " %ju", (uintmax_t)colref[i]->value);
        fprintf(stderr, "\n");
    }
    // search
    unsigned long r = rec_search(0, O, 0, max, success);
    // free memory
    free(O);
    // return result count
    return r;
}

// vim:ts=4 sts=4 sw=4 et:
