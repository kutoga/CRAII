#ifndef CRAII_H
#define CRAII_H

/*
 * Base macros
 */

#ifdef __cplusplus
#define _craii_auto_type                                                auto
#else
#define _craii_auto_type                                                __auto_type
#endif

#define _CRAII_FORCE_INLINE                                             __attribute__((__always_inline__)) inline

#define _CRAII_CONCAT(a, b, c)                                          a ## __ ## b ## __ ## c
#define _CRAII_CONCAT2(a, b, c)                                         _CRAII_CONCAT(a, b, c)
#define _CRAII_VAR_NAME_BASE                                            _craii_var
#define _CRAII_VAR_NAME(counter)                                        _CRAII_CONCAT2(_CRAII_VAR_NAME_BASE, counter, __LINE__)

#define CLEANUP_VAR(cleanup_function)                                   __attribute__((__cleanup__(cleanup_function)))
#define _CRAII_CLEANUP_VAL(value, cleanup_function, counter, counter2)  \
_CRAII_FORCE_INLINE void _CRAII_VAR_NAME(counter)(void **val)           \
{                                                                       \
    cleanup_function(*val);                                             \
}                                                                       \
_craii_auto_type _CRAII_VAR_NAME(counter2)                              \
    CLEANUP_VAR(_CRAII_VAR_NAME(counter)) = (void *)(value);            \
(void)_CRAII_VAR_NAME(counter2)

#define CLEANUP_VAL(value, cleanup_function)                            _CRAII_CLEANUP_VAL((value), cleanup_function, __COUNTER__, __COUNTER__)

static _CRAII_FORCE_INLINE void _craii_cleanup_wrapper(void (*f)())
{
    f();
}

#define CLEANUP_FUNC(cleanup_function)                                  CLEANUP_VAL((cleanup_function), _craii_cleanup_wrapper)

/*
 * "library"
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

static inline void _craii_fclose_wrapper(FILE **file)
{
    if (*file != NULL) {
        fclose(*file);
    }
}

#define AUTO_CLOSE(t)                                                   CLEANUP_VAR(_craii_fclose_wrapper) t

#define _AUTO_FREE(t, counter)                                          \
_CRAII_FORCE_INLINE void _CRAII_VAR_NAME(counter) (t *value)            \
{                                                                       \
    free(*value);                                                       \
}                                                                       \
CLEANUP_VAR(_CRAII_VAR_NAME(counter)) t
#define AUTO_FREE(t)                                                    _AUTO_FREE(t, __COUNTER__)

#define _CRAII_M_LOCK_GUARD(lock, counter, counter2)                    \
_CRAII_CLEANUP_VAL((lock), pthread_mutex_unlock, counter, counter2);    \
pthread_mutex_lock(_CRAII_VAR_NAME(counter2))

#define M_LOCK_GUARD(lock)                                              _CRAII_M_LOCK_GUARD((lock), __COUNTER__, __COUNTER__)


#define _CRAII_S_LOCK_GUARD(lock, counter, counter2)                    \
_CRAII_CLEANUP_VAL((lock), pthread_spin_unlock, counter, counter2);     \
pthread_spin_lock(_CRAII_VAR_NAME(counter2))

#define S_LOCK_GUARD(lock)                                              _CRAII_S_LOCK_GUARD((lock), __COUNTER__, __COUNTER__)

#endif
