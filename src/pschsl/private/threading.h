#ifndef PSCHSL_THREADING_H
#define PSCHSL_THREADING_H

#include <stdbool.h>

#ifndef PSCHSL_THREADING_USESTDTHREAD
    #if defined(_WIN32) && !defined(PSCHSL_THREADING_USEWINPTHREAD)
        #include <windows.h>
    #else
        #include <pthread.h>
    #endif
#else
    #include <threads.h>
#endif

struct thread_t;
struct thread_data {
    struct thread_t* self;
    void* args;
    volatile bool shouldclose;
};
typedef void* (*threadfunc_t)(struct thread_data*);
typedef struct thread_t {
    #ifndef PSCHSL_THREADING_USESTDTHREAD
    #if defined(_WIN32) && !defined(PSCHSL_THREADING_USEWINPTHREAD)
    HANDLE thread;
    #else
    pthread_t thread;
    #endif
    #else
    thrd_t thread;
    #endif
    char* name;
    threadfunc_t func;
    struct thread_data data;
    void* ret;
} thread_t;
#ifndef PSCHSL_THREADING_USESTDTHREAD
#if defined(_WIN32) && !defined(PSCHSL_THREADING_USEWINPTHREAD)
typedef CRITICAL_SECTION mutex_t;
#else
typedef pthread_mutex_t mutex_t;
#endif
#else
typedef mtx_t mutex_t;
#endif
struct accesslock {
    volatile unsigned counter; // TODO: make atomic
    mutex_t lock;
};

bool PSCHSL__CreateThread(thread_t*, const char* name, threadfunc_t func, void* args);
void PSCHSL__QuitThread(thread_t*);
void PSCHSL__DestroyThread(thread_t*, void** ret);
#define createThread PSCHSL__CreateThread
#define quitThread PSCHSL_QuitThread
#define destroyThread PSCHSL_DestroyThread

#ifndef PSCHSL_THREADING_USESTDTHREAD
    #if defined(_WIN32) && !defined(PSCHSL_THREADING_USEWINPTHREAD)
        #define yield() Sleep(0)
    #else
        #define yield() sched_yield()
    #endif
#else
    #define yield() thrd_yield()
#endif

static inline bool createMutex(mutex_t* m) {
    #ifndef PSCHSL_THREADING_USESTDTHREAD
    #if defined(_WIN32) && !defined(PSCHSL_THREADING_USEWINPTHREAD)
    InitializeCriticalSection(m);
    return true;
    #else
    return !pthread_mutex_init(m, NULL);
    #endif
    #else
    return (mtx_init(m, mtx_plain) == thrd_success);
    #endif
}
static inline void lockMutex(mutex_t* m) {
    #ifndef PSCHSL_THREADING_USESTDTHREAD
    #if defined(_WIN32) && !defined(PSCHSL_THREADING_USEWINPTHREAD)
    EnterCriticalSection(m);
    #else
    while (pthread_mutex_lock(m)) {}
    #endif
    #else
    while (mtx_lock(m) != thrd_success) {}
    #endif
}
static inline void unlockMutex(mutex_t* m) {
    #ifndef PSCHSL_THREADING_USESTDTHREAD
    #if defined(_WIN32) && !defined(PSCHSL_THREADING_USEWINPTHREAD)
    LeaveCriticalSection(m);
    #else
    while (pthread_mutex_unlock(m)) {}
    #endif
    #else
    while (mtx_unlock(m) != thrd_success) {}
    #endif
}
static inline void destroyMutex(mutex_t* m) {
    #ifndef PSCHSL_THREADING_USESTDTHREAD
    #if defined(_WIN32) && !defined(PSCHSL_THREADING_USEWINPTHREAD)
    DeleteCriticalSection(m);
    #else
    pthread_mutex_destroy(m);
    #endif
    #else
    mtx_destroy(m);
    #endif
}

static inline bool createAccessLock(struct accesslock* a) {
    if (!createMutex(&a->lock)) return false;
    a->counter = 0;
    return true;
}
static inline void destroyAccessLock(struct accesslock* a) {
    lockMutex(&a->lock);
    while (a->counter) {
        unlockMutex(&a->lock);
        yield();
        lockMutex(&a->lock);
    }
    destroyMutex(&a->lock);
}
static inline void acquireReadAccess(struct accesslock* a) {
    lockMutex(&a->lock);
    ++a->counter;
    unlockMutex(&a->lock);
}
static inline void releaseReadAccess(struct accesslock* a) {
    lockMutex(&a->lock);
    --a->counter;
    unlockMutex(&a->lock);
}
static inline void acquireWriteAccess(struct accesslock* a) {
    lockMutex(&a->lock);
    while (a->counter) {
        unlockMutex(&a->lock);
        yield();
        lockMutex(&a->lock);
    }
}
static inline void releaseWriteAccess(struct accesslock* a) {
    unlockMutex(&a->lock);
}
static inline void readToWriteAccess(struct accesslock* a) {
    lockMutex(&a->lock);
    --a->counter;
    while (a->counter) {
        unlockMutex(&a->lock);
        yield();
        lockMutex(&a->lock);
    }
}
static inline void writeToReadAccess(struct accesslock* a) {
    ++a->counter;
    unlockMutex(&a->lock);
}
static inline void yieldReadAccess(struct accesslock* a) {
    lockMutex(&a->lock);
    --a->counter;
    unlockMutex(&a->lock);
    yield();
    lockMutex(&a->lock);
    ++a->counter;
    unlockMutex(&a->lock);
}

#endif
