#include "private/time.h"

#include <stddef.h>
#include <stdbool.h>
#if !(defined(__APPLE__) && defined(__MACH__))
    #include <time.h>
#else
    #include <mach/mach_time.h>
    #include <CoreServices/CoreServices.h>
#endif

#ifdef _WIN32
    LARGE_INTEGER perfctfreq;
    uint64_t perfctmul = 1000000;
#endif

uint64_t altutime(void) {
    #if defined(_WIN32)
        LARGE_INTEGER time;
        QueryPerformanceCounter(&time);
        return time.QuadPart * perfctmul / perfctfreq.QuadPart;
    #elif defined(__APPLE__) && defined(__MACH__)
        uint64_t time = mach_absolute_time();
        Nanoseconds nsec = AbsoluteToNanoseconds(*(AbsoluteTime*)&time);
        return (*(uint64_t*)&nsec) / 1000;
    #else
        struct timespec time;
        clock_gettime(CLOCK_MONOTONIC, &time);
        return time.tv_sec * 1000000 + time.tv_nsec / 1000;
    #endif
}

void microwait(uint64_t d) {
    #if defined(_WIN32)
        #ifndef _MSC_VER
        static __thread HANDLE timer = NULL;
        #else
        static __declspec(thread) HANDLE timer = NULL;
        #endif
        if (!timer) timer = CreateWaitableTimer(NULL, true, NULL);
        LARGE_INTEGER _d = {.QuadPart = d * -10};
        SetWaitableTimer(timer, &_d, 0, NULL, NULL, false);
        WaitForSingleObject(timer, INFINITE);
    #elif defined(__APPLE__) && defined(__MACH__)
        d *= 1000;
        AbsoluteTime time = NanosecondsToAbsolute(*(Nanoseconds*)&d);
        mach_wait_until(*(uint64_t*)&time);
    #else
        struct timespec dts;
        dts.tv_sec = d / 1000000;
        dts.tv_nsec = (d % 1000000) * 1000;
        nanosleep(&dts, NULL);
    #endif
}
