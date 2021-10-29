#ifndef LIBACQUIRE_SHIM_H
#define LIBACQUIRE_SHIM_H

#include <stdlib.h>

char *strchrnul(const char *s, int c);

#ifndef HAVE_REALLOCARRAY
void *
reallocarray(void *optr, size_t nmemb, size_t size);
#endif

#endif /* LIBACQUIRE_SHIM_H */
