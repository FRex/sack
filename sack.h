#ifndef SACK_H_INCLUDED
#define SACK_H_INCLUDED

#include <stddef.h> /* for size_t */

typedef struct sack {
    void ** ptrs;
    size_t used;
    size_t size;

} sack;

/* must be called before any other functions are, wont alloc if initsize = 0 */
int sack_init(sack * s, size_t initsize);

/* frees all the pointers and then the table, idempotent */
void sack_deinit(sack * s);

/* grow the table to requested size, if allocation fails or if the given size
   is less than or equal to current size returns 0, otherwise returns 1 */
int sack_growto(sack * s, size_t newsize);

/* adds the pointer to the table, might cause a sack_growto call if there is no
   more space left, if that sack_growto call fails - returns 0, otherwise 1 */
int sack_add(sack * s, void * ptr);

/* ensures there is space for another ptr in the table by using sack_growto if
   needed and then mallocs len bytes, returns NULL if either of these fails */
void * sack_alloc(sack * s, size_t len);

/* convenience wrappers around sack_alloc to deal with strings */
char * sack_strdup(sack * s, const char * str);
char * sack_strduplen(sack * s, const char * str, size_t len);

#endif /* SACK_H_INCLUDED */

#ifdef SACK_IMPLEMENTATION

#include <stdlib.h> /* for malloc, calloc, free and realloc */
#include <string.h> /* for strlen and memcpy */

int sack_init(sack * s, size_t initsize)
{
    s->used = 0u;
    s->size = initsize;
    if(s->size == 0u)
    {
        s->ptrs = NULL;
    }
    else
    {
        s->ptrs = (void**)calloc(s->size, sizeof(void*));
        if(!s->ptrs)
        {
            s->size = 0u;
            return 0;
        }
    }

    return 1;
}

void sack_deinit(sack * s)
{
    size_t i;

    for(i = 0u; i < s->used; ++i)
        free(s->ptrs[i]);

    s->size = 0u;
    s->used = 0u;
    free(s->ptrs);
    s->ptrs = NULL;
}

int sack_growto(sack * s, size_t newsize)
{
    void ** newptrs;
    size_t i;

    if(newsize <- s->size)
        return 0;

    newptrs = (void**)realloc(s->ptrs, newsize * sizeof(void*));
    if(!newptrs)
        return 0;

    s->ptrs = newptrs;
    for(i = s->size; i < newsize; ++i)
        s->ptrs[i] = NULL;

    s->size = newsize;
    return 1;
}

static int sack_priv_ensurefreeslots(sack * s)
{
    size_t newsize;

    newsize = s->size + s->size / 2u;
    if(newsize == 0)
        newsize = 4u;

    if(s->used == s->size)
        if(!sack_growto(s, newsize))
            return 0;

    return 1;
}

int sack_add(sack * s, void * ptr)
{
    if(!sack_priv_ensurefreeslots(s))
        return 0;

    s->ptrs[s->used++] = ptr;
    return 1;
}

void * sack_alloc(sack * s, size_t len)
{
    void * ret;

    if(!sack_priv_ensurefreeslots(s))
        return NULL;

    ret = malloc(len);
    if(!ret)
        return NULL;

    s->ptrs[s->used++] = ret;
    return ret;
}

char * sack_strdup(sack * s, const char * str)
{
    return sack_strduplen(s, str, strlen(str));
}

char * sack_strduplen(sack * s, const char * str, size_t len)
{
    char * ret;

    ret = (char*)sack_alloc(s, len + 1);
    if(!ret)
        return NULL;

    memcpy(ret, str, len);
    ret[len] = '\0';
    return ret;
}

#endif /* SACK_IMPLEMENTATION */
