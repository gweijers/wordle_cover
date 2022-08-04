//
//  words.c, created 2022-08-03 22:09:18-0700 by ge
//
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>

#include "dlx.h"


void error(int err, const char* fmt, ...)
{
    va_list vl;
    va_start(vl, fmt);
    vfprintf(stderr, fmt, vl);
    va_end(vl);
    if (err != 0)
        fprintf(stderr, ": %s\n", strerror(err));
    else
        putc('\n', stderr);
    exit(EXIT_FAILURE);
}


enum { BBB = 102400 };
enum { MAXW = 128 };


//
//  encode word into column numbers
//
unsigned encode(const char* word, unsigned *res)
{
    uint32_t bits = 0;
    char c;
    const char *s = word;
    while ((c = *s++) != '\0') {
        if (c < 'a' || c > 'z')
            error(0, "word %s contains a %c", word, c);
        bits |= (UINT32_C(1) << (c - 'a'));
    }
    unsigned count = 0;
    for (unsigned i = 0; i < 26; i++) {
        if ((bits >> i) & 0x1)
            res[count++] = i;
    }
    return count;
}


void success(unsigned long count, const value_t result[], unsigned n)
{
    printf("%lu:", count);
    for (unsigned i = 0; i < n; i++) {
        const char *w = (const char *)result[i];
        if (w != NULL)
            printf(" %s", w);
    }
    putchar('\n');
}


int main (int argc, char *argv[])
{
    //  make sure there's a file name
    if (argc != 2)
        error(0, "need a file name");
    //  open file
    const char *fn = argv[1];
    FILE *f = fopen(fn, "r");
    if (f == NULL)
        error(errno, "can't open %s", fn);
    //  set up empty buffer
    char *buffer = NULL;
    size_t bufsize = 0;
    //  initialize dancing links
    init_matrix(0);
    //  read all the words and generate the matrix
    for (;;) {
        //  allocate more buffer if we run low
        if (bufsize < MAXW) {
            buffer = (char *)malloc(BBB);
            if (buffer == NULL)
                error(errno, "out of memory");
            bufsize = BBB;
        }
        //  read the next line
        if (fgets(buffer, MAXW, f) == NULL)
            break;
        //  strip whitespace from the end of the line
        size_t len = strlen(buffer);
        while (len > 0 && isspace(buffer[len-1]))--len;
        //  length must be 5, else ignore word
        if (len == 5) {
            //  encode word in column numbers
            unsigned v[5];
            //  terminate line
            buffer[len++] = '\0';
            if (encode(buffer, v) == 5) {
                //  add row with word
                add_row((uintptr_t)buffer, v, 5);
                //  skip word
                buffer += len;
                bufsize -= len;
            }
        }
    }
    //  put constraints for single unused letter
    //  (or we get the 'solution' of 25 single letters
    //  column 26 makes sure we only get one single letter
    for (unsigned i = 0; i < 26; i++) {
        unsigned v[2];
        v[0] = i;
        v[1] = 26;
        add_row((uintptr_t)NULL, v, 2);
    }
    //  search for solution
    unsigned long total = search(ULONG_MAX, success);
    //  release matrix
    free_matrix();
    //  print # of solutions
    fprintf(stderr, "%lu solutions found\n", total);
    return EXIT_SUCCESS;
}


//  vim:ts=4 sts=4 sw=4 et:
