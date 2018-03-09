#include <stdlib.h>
#include <mem.h>

int main (int argc, char *argv[]) {
  int err = 0;

  void *block = memmap_alloc (10, &err);
  if (err)
    exit (EXIT_FAILURE);

  memmap_free (block, &err);
  if (err)
    exit (EXIT_FAILURE);

  return EXIT_SUCCESS;
}
