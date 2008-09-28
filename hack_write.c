#define fwrite ye_olde_fwrite
#include <stdio.h>

#include <dlfcn.h>

#undef fwrite

int fwrite(const void *ptr, int size, int nmemb,
	FILE *stream) {

  void * libc = dlopen("/lib/libc.so.6", RTLD_LAZY); /* never closed, rofl */
  int (*lol_fwrite) (const void *, int, int, FILE *);
  *(void **) (&lol_fwrite) = dlsym(libc, "fwrite");
  (*lol_fwrite)(ptr, size, nmemb, stream);

	
	/* (*lol_fwrite)("rofl\n", 1, 5, stream);
	(*lol_fwrite)(ptr, size, nmemb, stream);
	(*lol_fwrite)("unrofl\n", 1, 5, stream); */
}

