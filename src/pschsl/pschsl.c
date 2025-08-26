#define PSCHSL_NOLEGACY
#include "pschsl.h"

const unsigned (*PSCHSL_GetVersion(void))[3] {
    static const unsigned ver[3] = {
        PSCHSL_VER_MAJOR,
        PSCHSL_VER_MINOR,
        PSCHSL_VER_PATCH
    };
    return &ver;
}

struct PSCHSL_Ctx {
    int placeholder;
};

struct PSCHSL {
    int placeholder;
};
