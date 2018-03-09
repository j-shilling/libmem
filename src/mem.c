#include <mem.h>

#include <errno.h>
#include <string.h>
#include <sys/mman.h>

#define header_of(mem)	  (((struct memheader_t*)mem)-1)

struct memheader_t {
  size_t size;
  unsigned int ref_count;
};

struct list_t {
  struct memheader_t *block;
  struct list_t *prev;
  struct list_t *next;
};

static struct list_t *free_blocks = NULL;

static inline void *alloc (size_t size, int *err) {
  if (size <= 0)
    return NULL;

  void *ret = mmap (NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ret == ((void *)-1) && *err)
    *err = errno;

  return ret;
}

static inline int dealloc (int *err) {
  if (!free_blocks)
    return 0;

  struct memheader_t *header = free_blocks->block;
  struct list_t *head = free_blocks;
  free_blocks = free_blocks->next;

  if (munmap (header, header->size + sizeof (struct memheader_t)) == -1) {
    if (err)
      *err = errno;
    return 0;
  }

  if (munmap (head, sizeof (struct list_t)) == -1) {
    if (err)
      *err = errno;
    return 0;
  }

  return 1;
}

static struct memheader_t *get_block (size_t size, int *err) {
  if (size == 0)
    return NULL;

  struct memheader_t *ret = NULL;
  for (struct list_t *cur = free_blocks; cur; cur = cur->next) {
    if (cur->block->size > size) {
	continue;
    } else if (cur->block->size == size) {
      ret = cur->block;
      cur->prev->next = cur->next;
      cur->next->prev = cur->prev;

      if (munmap (cur, sizeof (struct list_t)) == -1) {
	if (err)
	  *err = errno;
	return NULL;
      }
    } else if (cur->block->size < size) {
      struct list_t *node = cur->prev;

      ret = node->block;
      node->prev->next = node->next;
      node->next->prev = node->prev;

      if (munmap (cur, sizeof (struct list_t)) == -1) {
	if (err)
	  *err = errno;
	return NULL;
      }
    }
  }

  if (!ret) {
    int err_code = 0;
    ret = alloc (size + sizeof (struct memheader_t), &err_code);
    if (!ret) {
      if (err_code == ENOMEM) {
	if (dealloc (err))
	  return get_block (size, err);
      }
      return NULL;
    }

    ret->size = size;
  }

  return ret;
}

void *memmap_alloc   (size_t size, int *err) {
  struct memheader_t *header = get_block (size, err);
  if (!header)
    return NULL;

  header->ref_count = 1;

  return (void*)(header + 1);
}

void *memmap_alloc0  (size_t size, int *err) {
  void *ret = memmap_alloc (size, err);
  if (!ret)
    return NULL;

  memset (ret, 0, size);

  return ret;
}

void  memmap_free    (void *mem, int *err) {
  if (!mem)
    return;

  struct memheader_t *header = header_of (mem);
  if (!free_blocks) {
    free_blocks = alloc (sizeof (struct list_t), err);
    free_blocks->block = header;
    free_blocks->prev = NULL;
    free_blocks->next = NULL;
  } else if (!free_blocks->next) {
    if (free_blocks->block->size > header->size) {
      struct list_t *node = alloc (sizeof (struct list_t), err);
      if (!node)
	return;

      node->block = header;
      node->prev = free_blocks;
      node->next = NULL;
      free_blocks->next = node;
    } else {
      struct list_t *node = alloc (sizeof (struct list_t), err);
      if (!node)
	return;

      node->block = header;
      node->next = free_blocks;
      node->prev = NULL;     
      free_blocks->prev = node;
    }
  } else {
    struct list_t *node = alloc (sizeof (struct list_t), err);
    if (!node)
      return;
    node->block = header;

    struct list_t *cur = free_blocks;
    while (cur->next && cur->next->block->size > header->size)
      cur = cur->next;
    if (cur->next) {
      node->next = cur->next;
      node->prev = cur;

      cur->next->prev = node;
      cur->next = node;
    } else {
      if (cur->block->size > header->size) {
	cur->next = node;
	node->prev = cur;
	node->next = NULL;
      } else {
	node->next = cur;
	node->prev = cur->prev;
	cur->prev = node;
      }
    }
  }
}

void *memmap_ref     (void *mem, int *err) {
  if (!mem)
    return mem;

  header_of (mem)->ref_count ++;

  return mem;
}

void  memmap_unref   (void *mem, int *err) {
  if (!mem)
    return;

  struct memheader_t *header = header_of (mem);
  header->ref_count --;
  if (!header->ref_count)
    memmap_free (mem, err);
}
