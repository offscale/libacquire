#ifndef LIBACQUIRE_SHIM_H
#define LIBACQUIRE_SHIM_H

#include <stdlib.h>

char *strchrnul(const char *s, int c);

#ifndef HAVE_REALLOCARRAY
void *
reallocarray(void *optr, size_t nmemb, size_t size);
#endif

#ifndef __printflike
#define __printflike(fmtarg, firstvararg) \
	        __attribute__((__format__ (__printf__, fmtarg, firstvararg)))
#endif

#ifndef __CAST_AWAY_QUALIFIER
#define __CAST_AWAY_QUALIFIER(variable, qualifier, type)  (type) (long)(variable)
#endif

#ifndef __DECONST
#define __DECONST(type, var)    __CAST_AWAY_QUALIFIER(var, const, type)
#endif

#endif /* ! LIBACQUIRE_SHIM_H */
