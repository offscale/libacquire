#if !defined(_STDBOOL_H) && !defined(HAS_STDBOOL)
#define _STDBOOL_H

/**
 * @file acquire_stdbool.h
 * @brief Defines boolean type and constants for compatibility.
 *
 * stdbool isn't always included with every popular C89 implementation.
 * So this provides typedef for `bool` and defines `true` and `false`
 * constants if not provided by the platform. This variant is modified from
 * MUSL.
 */

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
