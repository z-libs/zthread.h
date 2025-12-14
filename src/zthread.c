
/*
 * zthread.h — Portable threads, mutexes, and synchronization primitives
 * Part of Zen Development Kit (ZDK)
 *
 * A single-header, cross-platform threading library that abstracts Win32 and
 * POSIX threads into a unified, type-safe C API. It requires zero configuration
 * and works out of the box on Windows, Linux, and macOS.
 *
 * Features:
 * • Native Win32 and POSIX (pthread) backends.
 * • Type-safe thread creation macros (zthread_create).
 * • Unified Mutex and Condition Variable primitives.
 * • Optional short names via ZTHREAD_SHORT_NAMES.
 * • Zero dependencies (only standard system headers).
 *
 * License: MIT
 * Author: Zuhaitz
 * Repository: https://github.com/z-libs/zthread.h
 * Version: 1.0.0
 */

#ifndef ZTHREAD_H
#define ZTHREAD_H

#include <stdlib.h>

#ifdef _WIN32
#   include <windows.h>
#   include <process.h>
    typedef HANDLE zthread_t;
    typedef CRITICAL_SECTION zmutex_t;
    typedef CONDITION_VARIABLE zcond_t;
    // Internal Windows thread signature.
#   define ZTHREAD_Func unsigned __stdcall
#else
#   include <pthread.h>
#   include <unistd.h>
    typedef pthread_t zthread_t;
    typedef pthread_mutex_t zmutex_t;
    typedef pthread_cond_t zcond_t;
    // Internal POSIX thread signature.
#   define ZTHREAD_Func void*
#endif

// The raw function signature required by the OS.
typedef void (*zthread_proxy_fn)(void* arg);

// Our API.

// Internal raw creation function (void* hell, yuck).
int zthread__create_ptr(zthread_t *t, zthread_proxy_fn func, void *arg);

/* * Type-safe creation macro.
 * Automatically casts the function and argument to void*.
 * Usage: zthread_create(&t, my_func, &my_data);
*/
#define zthread_create(t, func, arg) \
    zthread__create_ptr((t), (zthread_proxy_fn)(func), (void*)(arg))


/* * Strict wrapper macro.
 * Generates a void* proxy to ensure strict standard compliance.
 * Usage: ZTHREAD_WRAP(my_worker, int*, count) { ... }
*/
#define ZTHREAD_WRAP(name, type, var)                               \
    void name##__impl(type var);                                    \
    void name(void* _void_arg) { name##__impl((type)_void_arg); }   \
    void name##__impl(type var)


// Thread control.
void zthread_join(zthread_t t);
void zthread_detach(zthread_t t);
void zthread_sleep(int ms);

// Mutexes.
void zmutex_init(zmutex_t *m);
void zmutex_lock(zmutex_t *m);
void zmutex_unlock(zmutex_t *m);
void zmutex_destroy(zmutex_t *m);

// Condition variables.
void zcond_init(zcond_t *c);
void zcond_wait(zcond_t *c, zmutex_t *m);
void zcond_signal(zcond_t *c);
void zcond_broadcast(zcond_t *c);
void zcond_destroy(zcond_t *c);

// Short names (optional).
#ifdef ZTHREAD_SHORT_NAMES
    typedef zthread_t   thread_t;
    typedef zmutex_t    mutex_t;
    typedef zcond_t     cond_t;

#   define thread_create   zthread_create
#   define thread_join     zthread_join
#   define thread_detach   zthread_detach
#   define thread_sleep    zthread_sleep
    
    // Macro wrapper alias.
#   define THREAD_WRAP     ZTHREAD_WRAP

#   define mutex_init      zmutex_init
#   define mutex_lock      zmutex_lock
#   define mutex_unlock    zmutex_unlock
#   define mutex_destroy   zmutex_destroy

#   define cond_init       zcond_init
#   define cond_wait       zcond_wait
#   define cond_signal     zcond_signal
#   define cond_broadcast  zcond_broadcast
#   define cond_destroy    zcond_destroy
#endif

#ifdef ZTHREAD_IMPLEMENTATION

struct zthread__wrap 
{ 
    zthread_proxy_fn f; 
    void *arg; 
};

#ifdef _WIN32
static unsigned __stdcall zthread__proxy_entry(void *p) 
{
    struct zthread__wrap *w = (struct zthread__wrap*)p;
    w->f(w->arg); free(w); return 0;
}

int zthread__create_ptr(zthread_t *t, zthread_proxy_fn func, void *arg) 
{
    struct zthread__wrap *w = (struct zthread__wrap*)malloc(sizeof(*w)); 
    w->f = func; w->arg = arg;
    *t = (HANDLE)_beginthreadex(NULL, 0, zthread__proxy_entry, w, 0, NULL);
    return *t ? 0 : 1;
}

void zthread_join(zthread_t t) 
{ 
    WaitForSingleObject(t, INFINITE); 
    CloseHandle(t); 
}

void zthread_detach(zthread_t t) 
{ 
    CloseHandle(t); 
}

void zthread_sleep(int ms) 
{ 
    Sleep(ms); 
}

void zmutex_init(zmutex_t *m) 
{ 
    InitializeCriticalSection(m); 
}

void zmutex_lock(zmutex_t *m) 
{ 
    EnterCriticalSection(m); 
}

void zmutex_unlock(zmutex_t *m) 
{ 
    LeaveCriticalSection(m); 
}

void zmutex_destroy(zmutex_t *m) 
{ 
    DeleteCriticalSection(m); 
}

void zcond_init(zcond_t *c) 
{ 
    InitializeConditionVariable(c); 
}
void zcond_wait(zcond_t *c, zmutex_t *m) 
{ 
    SleepConditionVariableCS(c, m, INFINITE); 
}

void zcond_signal(zcond_t *c) 
{ 
    WakeConditionVariable(c); 
}

void zcond_broadcast(zcond_t *c) 
{ 
    WakeAllConditionVariable(c); 
}

void zcond_destroy(zcond_t *c) 
{ 
    (void)c; 
} 

#else
// POSIX implementation.

static void* zthread__proxy_entry(void *p) 
{
    struct zthread__wrap *w = (struct zthread__wrap*)p;
    w->f(w->arg); free(w); return NULL;
}

int zthread__create_ptr(zthread_t *t, zthread_proxy_fn func, void *arg) 
{
    struct zthread__wrap *w = (struct zthread__wrap*)malloc(sizeof(*w)); 
    w->f = func; w->arg = arg;
    return pthread_create(t, NULL, zthread__proxy_entry, w);
}

void zthread_join(zthread_t t) 
{ 
    pthread_join(t, NULL); 
}

void zthread_detach(zthread_t t) 
{ 
    pthread_detach(t); 
}

void zthread_sleep(int ms) 
{ 
    usleep(ms * 1000); 
}

void zmutex_init(zmutex_t *m) 
{ 
    pthread_mutex_init(m, NULL); 
}
void zmutex_lock(zmutex_t *m) 
{ 
    pthread_mutex_lock(m); 
}

void zmutex_unlock(zmutex_t *m) 
{ 
    pthread_mutex_unlock(m); 
}

void zmutex_destroy(zmutex_t *m) 
{ 
    pthread_mutex_destroy(m); 
}

void zcond_init(zcond_t *c) 
{ 
    pthread_cond_init(c, NULL); 
}

void zcond_wait(zcond_t *c, zmutex_t *m) 
{ 
    pthread_cond_wait(c, m); 
}

void zcond_signal(zcond_t *c) 
{ 
    pthread_cond_signal(c); 
}

void zcond_broadcast(zcond_t *c) 
{ 
    pthread_cond_broadcast(c); 
}

void zcond_destroy(zcond_t *c) 
{ 
    pthread_cond_destroy(c); 
}
#endif

#endif // ZTHREAD_IMPLEMENTATION
#endif // ZTHREAD_H 
