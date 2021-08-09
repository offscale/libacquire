/* Modififed from  MUSL */

#ifndef _STDBOOL_H
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

#define bool size_t
#define true 1
#define false (!true)

#endif
