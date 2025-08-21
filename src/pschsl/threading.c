#include "private/threading.h"

#undef createThread
#undef quitThread
#undef destroyThread

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) && !defined(PSCHSL_THREADING_USEWINPTHREAD) && !defined(PSCHSL_THREADING_USESTDTHREAD)
static DWORD WINAPI threadwrapper(LPVOID t) {
    ((thread_t*)t)->ret = ((thread_t*)t)->func(&((thread_t*)t)->data);
    ExitThread(0);
    return 0;
}
#elif defined(PSCHSL_THREADING_USESTDTHREAD)
static int threadwrapper(void* t) {
    ((thread_t*)t)->ret = ((thread_t*)t)->func(&((thread_t*)t)->data);
    thrd_exit(0);
    return 0;
}
#else
static void* threadwrapper(void* t) {
    #ifndef PSCHSL_THREADING_NONAMES
        #ifndef PSCHSL_THREADING_USESTDTHREAD
            #if defined(__GLIBC__)
                pthread_setname_np(((thread_t*)t)->thread, ((thread_t*)t)->name);
            #elif defined(__NetBSD__)
                pthread_setname_np(((thread_t*)t)->thread, "%s", ((thread_t*)t)->name);
            #elif defined(__FreeBSD__) || defined(__OpenBSD__)
                pthread_set_name_np(((thread_t*)t)->thread, ((thread_t*)t)->name);
            #endif
        #endif
    #endif
    ((thread_t*)t)->ret = ((thread_t*)t)->func(&((thread_t*)t)->data);
    pthread_exit(((thread_t*)t)->ret);
    return ((thread_t*)t)->ret;
}
#endif

bool PSCHSL__CreateThread(thread_t* t, const char* n, threadfunc_t f, void* a) {
    t->name = (n) ? strdup(n) : NULL;
    t->func = f;
    t->data.self = t;
    t->data.args = a;
    t->data.shouldclose = false;
    bool fail;
    #ifndef PSCHSL_THREADING_USESTDTHREAD
        #if defined(_WIN32) && !defined(PSCHSL_THREADING_USEWINPTHREAD)
            fail = !(t->thread = CreateThread(NULL, 0, threadwrapper, t, 0, NULL));
        #else
            fail = pthread_create(&t->thread, NULL, threadwrapper, t);
        #endif
    #else
        fail = (thrd_create(&t->thread, (thrd_start_t)threadwrapper, t) != thrd_success);
    #endif
    if (fail) {
        free(t->name);
        return false;
    }
    return true;
}

void PSCHSL__QuitThread(thread_t* t) {
    t->data.shouldclose = true;
}

void PSCHSL__DestroyThread(thread_t* t, void** r) {
    t->data.shouldclose = true;
    #ifndef PSCHSL_THREADING_USESTDTHREAD
        #if defined(_WIN32) && !defined(PSCHSL_THREADING_USEWINPTHREAD)
            WaitForSingleObject(t->thread, INFINITE);
            if (r) *r = t->ret;
        #else
            pthread_join(t->thread, r);
        #endif
    #else
        thrd_join(t->thread, NULL);
        if (r) *r = t->ret;
    #endif
    free(t->name);
}
