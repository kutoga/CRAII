#ifndef CRAII_H
#define CRAII_H

/*
 * Base macros
 */

#ifdef __cplusplus
#define _craii_auto_type                                            auto
#else
#define _craii_auto_type                                            __auto_type
#endif

#define _CRAII_CONCAT(a, b, c)                                      a ## __ ## b ## __ ## c
#define _CRAII_CONCAT2(a, b, c)                                     _CRAII_CONCAT(a, b, c)
#define _CRAII_VAR_NAME_BASE                                        _craii_var
#define _CRAII_VAR_NAME(counter)                                    _CRAII_CONCAT2(_CRAII_VAR_NAME_BASE, counter, __LINE__)

#define CLEANUP_VAR(cleanup_function)                               __attribute__((__cleanup__(cleanup_function)))
#define _CRAII_CLEANUP_VAL(value, cleanup_function, counter)        \
_craii_auto_type _CRAII_VAR_NAME(counter)                           \
    CLEANUP_VAR(cleanup_function) = (value);                        \
(void)_CRAII_VAR_NAME(counter)

#define CLEANUP_VAL(value, cleanup_function)                        _CRAII_CLEANUP_VAL((value), cleanup_function, __COUNTER__)

static inline void _craii_cleanup_wrapper(void (**f)())
{
    (*f)();
}

#define CLEANUP_FUNC(cleanup_function)                               CLEANUP_VAL((cleanup_function), _craii_cleanup_wrapper)

/*
 * "library"
 */

#include <stdio.h>
#include <pthread.h>

static inline void _craii_fclose_wrapper(FILE **file)
{
    if (*file) {
        fclose(*file);
    }
}

#define AUTO_CLOSE                                                  CLEANUP_VAR(_craii_fclose_wrapper)


static inline void _craii_mutex_cleanup(pthread_mutex_t **lock)
{
    pthread_mutex_unlock(*lock);
}

#define _CRAII_M_LOCK_GUARD(lock, counter)                          \
_CRAII_CLEANUP_VAL((lock), _craii_mutex_cleanup, counter);          \
pthread_mutex_lock(_CRAII_VAR_NAME(counter))

#define M_LOCK_GUARD(lock)                                          _CRAII_M_LOCK_GUARD((lock), __COUNTER__)


static inline void _craii_spin_cleanup(pthread_spinlock_t **lock)
{
    pthread_spin_unlock(*lock);
}

#define _CRAII_S_LOCK_GUARD(lock, counter)                          \
_CRAII_CLEANUP_VAL((lock), _craii_spin_cleanup, counter);           \
pthread_spin_lock(_CRAII_VAR_NAME(counter))

#define S_LOCK_GUARD(lock)                                          _CRAII_S_LOCK_GUARD((lock), __COUNTER__)

#endif
