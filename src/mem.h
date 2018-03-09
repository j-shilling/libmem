#ifndef __MEMMAP_H__
#define __MEMMAP_H__

#include <stddef.h>

void *memmap_alloc   (size_t size, int *err);
void *memmap_realloc (size_t size, int *err);
void *memmap_alloc0  (size_t size, int *err);
void  memmap_free    (void *mem, int *err);

void *memmap_ref     (void *mem, int *err);
void  memmap_unref   (void *mem, int *err);

#endif /* __MEMMAP_H__ */
