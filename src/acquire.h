#ifndef LIBACQUIRE_ACQUIRE_H
#define LIBACQUIRE_ACQUIRE_H

#include <stdlib.h>
#include "stdbool.h"

extern const char* get_download_dir();
extern bool is_downloaded(const char*,const char*);
extern int download(const char*, const char*, const char*, bool, size_t, size_t);
extern int download_many(const char*[], const char*, const char*, bool, size_t, size_t);

#endif //LIBACQUIRE_ACQUIRE_H
