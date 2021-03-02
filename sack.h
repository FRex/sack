#ifndef SACK_H_INCLUDED
#define SACK_H_INCLUDED

#include <stddef.h> /* for size_t */

typedef struct sack {
    void ** ptrs; /* pointers table to keep pointers to free in */
    size_t used;  /* how many elements of ptrs table are used   */
    size_t size;  /* how many void * elements are there in ptrs */

} sack;

/* must be called before any other functions are, wont alloc if initsize = 0,
   not safe to call on already initialized sack that isn't empty - it will leak
   the table and all pointers inside it, use sack_freeall or sack_deinit */
int sack_init(sack * s, size_t initsize);

/* free all the pointers but keep the table so it can be reused, idempotent, not
   safe to call on uninitialized sack (memset/calloc to 0 or sack_init first) */
void sack_freepointers(sack * s);

/* frees all the pointers and then the table, idempotent, sack is reusable
   without needing to sack_init again, not safe to call on uninitialized sack
   structure (memset/calloc it to 0 or call sack_init on it first) */
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

/* allocates len block with sack_alloc and copies mem content into it, returns
   NULL on failure, does not free mem in any way */
void * sack_memdup(sack * s, const void * mem, size_t len);

/* allocate newlen block with sack_alloc and then copy len bytes into it,
   returns NULL on failure or if len > newlen, does not free mem in any way */
void * sack_memdupmore(sack * s, const void * mem, size_t len, size_t newlen);

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

void sack_freepointers(sack * s)
{
    size_t i;

    for(i = 0u; i < s->used; ++i)
        free(s->ptrs[i]);

    s->used = 0u;
}

void sack_deinit(sack * s)
{
    sack_freepointers(s);
    free(s->ptrs);
    s->size = 0u;
    s->ptrs = NULL;
}

int sack_growto(sack * s, size_t newsize)
{
    void ** newptrs;

    if(newsize < s->size)
        return 0;

    newptrs = (void**)realloc(s->ptrs, newsize * sizeof(void*));
    if(!newptrs)
        return 0;

    s->ptrs = newptrs;
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

    if(!str || len == 0u)
        return NULL;

    ret = (char*)sack_alloc(s, len + 1);
    if(!ret)
        return NULL;

    memcpy(ret, str, len);
    ret[len] = '\0';
    return ret;
}

void * sack_memdup(sack * s, const void * mem, size_t len)
{
    return sack_memdupmore(s, mem, len, len);
}

void * sack_memdupmore(sack * s, const void * mem, size_t len, size_t newlen)
{
    void * ret;

    if(!mem || len == 0u || len > newlen)
        return NULL;

    ret = sack_alloc(s, newlen);
    if(!ret)
        return NULL;

    memcpy(ret, mem, len);
    return ret;
}

#endif /* SACK_IMPLEMENTATION */
