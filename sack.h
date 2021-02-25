#ifndef SACK_H_INCLUDED
#define SACK_H_INCLUDED

#include <stddef.h>

typedef struct sack {
    void ** ptrs;
    size_t used;
    size_t size;

} sack;

int sack_init(sack * s, size_t initsize);
void sack_deinit(sack * s);

int sack_growto(sack * s, size_t newsize);
int sack_add(sack * s, void * ptr);

void * sack_alloc(sack * s, size_t len);
char * sack_strdup(sack * s, const char * str);
char * sack_strduplen(sack * s, const char * str, size_t len);

#endif /* SACK_H_INCLUDED */

#ifdef SACK_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>

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
