#include "shim.h"
#include <limits.h>

/* stolen from musl */

#define M_ALIGN (sizeof(size_t))
#define M_ONES ((size_t)-1 / UCHAR_MAX)
#define M_HIGHS (M_ONES * (UCHAR_MAX / 2 + 1))
#define M_HASZERO(x) ((x)-M_ONES & ~(x)&M_HIGHS)

char *strchrnul(const char *s, int c) {
#ifdef __GNUC__
  typedef size_t __attribute__((__may_alias__)) word;
  const word *w;
#endif /* __GNUC__ */
  c = (unsigned char)c;
  if (!c)
    return (char *)s + strlen(s);

#ifdef __GNUC__
  {
    for (; (uintptr_t)s % M_ALIGN; s++)
      if (!*s || *(unsigned char *)s == c)
        return (char *)s;
    {
      size_t k = M_ONES * c;
      for (w = (void *)s; !M_HASZERO(*w) && !M_HASZERO(*w ^ k); w++)
        ;
    }
  }
  s = (void *)w;
#endif /* __GNUC__ */
  for (; *s && *(unsigned char *)s != c; s++)
    ;
  return (char *)s;
}

#undef M_ALIGN
#undef M_ONES
#undef M_HIGHS
#undef M_HASZERO

#ifndef HAVE_REALLOCARRAY

/* FROM
 * https://github.com/openssh/openssh-portable/blob/37f9220/openbsd-compat/reallocarray.c
 */

#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>

#define MUL_NO_OVERFLOW ((size_t)1 << (sizeof(size_t) * 4))

void *reallocarray(void *optr, size_t nmemb, size_t size) {
  if ((nmemb >= MUL_NO_OVERFLOW || size >= MUL_NO_OVERFLOW) && nmemb > 0 &&
      SIZE_MAX / nmemb < size) {
    errno = ENOMEM;
    return NULL;
  }
  return realloc(optr, size * nmemb);
}
#endif /* ! HAVE_REALLOCARRAY */
