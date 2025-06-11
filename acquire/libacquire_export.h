#ifndef LIBACQUIRE_EXPORT_H
#define LIBACQUIRE_EXPORT_H

#ifdef LIBACQUIRE_STATIC_DEFINE
#define LIBACQUIRE_EXPORT
#define LIBACQUIRE_NO_EXPORT
#else
#ifndef LIBACQUIRE_EXPORT
#ifdef LIBACQUIRE_EXPORTS
/* We are building this library */
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MSYS__) ||          \
    defined(__CYGWIN__)

#ifdef LIBACQUIRE_IMPLEMENTATION
# define LIBACQUIRE_EXPORT __declspec(dllexport)
#else
# define LIBACQUIRE_EXPORT __declspec(dllimport)
#endif


#elif defined(__SUNPRO_C)
#define LIBACQUIRE_EXPORT __global
#else
#define LIBACQUIRE_EXPORT __attribute__((visibility("default")))
#endif /* defined(_MSC_VER) || defined(__MINGW32__) || defined(__MSYS__) ||    \
          defined(__CYGWIN__) */
#else
/* We are using this library */
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MSYS__) ||          \
    defined(__CYGWIN__)

#ifdef LIBACQUIRE_IMPLEMENTATION
# define LIBACQUIRE_EXPORT __declspec(dllexport)
#else
# define LIBACQUIRE_EXPORT __declspec(dllimport)
#endif

#elif defined(__SUNPRO_C)
#define LIBACQUIRE_EXPORT __global
#else
#define LIBACQUIRE_EXPORT __attribute__((visibility("default")))
#endif /* defined(_MSC_VER) || defined(__MINGW32__) || defined(__MSYS__) ||    \
          defined(__CYGWIN__) */
#endif /* LIBACQUIRE_EXPORTS */
#endif /* ! LIBACQUIRE_EXPORT */

#ifndef LIBACQUIRE_NO_EXPORT
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MSYS__) ||          \
    defined(__CYGWIN__) || defined(__SUNPRO_C)
#define LIBACQUIRE_NO_EXPORT
#else
#define LIBACQUIRE_NO_EXPORT __attribute__((visibility("hidden")))
#endif /* defined(_MSC_VER) || defined(__MINGW32__) || defined(__MSYS__) ||    \
          defined(__CYGWIN__) || defined(__SUNPRO_C) */
#endif /* ! LIBACQUIRE_NO_EXPORT */
#endif /* LIBACQUIRE_STATIC_DEFINE */

#ifndef LIBACQUIRE_DEPRECATED
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MSYS__) ||          \
    defined(__CYGWIN__)
#define LIBACQUIRE_DEPRECATED __declspec(deprecated)
#elif defined(__SUNPRO_C)
#define LIBACQUIRE_DEPRECATED
#else
#define LIBACQUIRE_DEPRECATED __attribute__((__deprecated__))
#endif /* defined(_MSC_VER) || defined(__MINGW32__) || defined(__MSYS__) ||    \
          defined(__CYGWIN__) */
#endif /* ! LIBACQUIRE_DEPRECATED */

#ifndef LIBACQUIRE_DEPRECATED_EXPORT
#define LIBACQUIRE_DEPRECATED_EXPORT LIBACQUIRE_EXPORT LIBACQUIRE_DEPRECATED
#endif /* ! LIBACQUIRE_DEPRECATED_EXPORT */

#ifndef LIBACQUIRE_DEPRECATED_NO_EXPORT
#define LIBACQUIRE_DEPRECATED_NO_EXPORT LIBACQUIRE_NO_EXPORT LIBACQUIRE_DEPRECATED
#endif /* ! LIBACQUIRE_DEPRECATED_NO_EXPORT */

#if 0 /* DEFINE_NO_DEPRECATED */
#ifndef LIBACQUIRE_NO_DEPRECATED
#define LIBACQUIRE_NO_DEPRECATED
#endif /* ! LIBACQUIRE_NO_DEPRECATED */
#endif /* 0 */

#endif /* !LIBACQUIRE_EXPORT_H */
