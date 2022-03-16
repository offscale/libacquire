/*
 * stdbool isn't always included with every popular C89 implementation
 *
 * This variant is modified from MUSL
 * */

#if !defined(_STDBOOL_H) && !defined(HAS_STDBOOL)
#define _STDBOOL_H

#ifdef bool
#undef bool
#endif
#ifdef true
#undef true
#endif
#ifdef false
#undef false
#endif

#include <stdlib.h>

typedef size_t bool;
#define true 1
#define false (!true)

#endif /* !defined(_STDBOOL_H) && !defined(HAS_STDBOOL) */
