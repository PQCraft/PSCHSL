// Extremely basic file server

#include <pschsl/pschsl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>

static struct PSCHSL* state;

static void sigh(int sig) {
    PSCHSL_Stop(state);
}

static enum PSCHSL_Ctx_CBStatus callback(struct PSCHSL_Ctx* ctx, void* userdata) {
    char* path;
    {
        const char* tmppath = PSCHSL_Rqst_GetTarget(ctx);
        if (!tmppath || !*tmppath) {
            PSCHSL_Resp_SetStatus(ctx, 400, NULL);
            return PSCHSL_CTX_CBSTATUS_OK;
        }
        size_t len = strlen(path) + 1;
        path = malloc(len + 2);
        path[0] = '.';
        path[1] = '/';
        memcpy(path + 2, tmppath, len);
    }
    struct stat s;
    FILE* f;
    if (stat(path, &s) || !S_ISREG(s.st_mode) || !(f = fopen(path, "rb"))) {
        free(path);
        PSCHSL_Resp_SetStatus(ctx, 404, NULL);
        return PSCHSL_CTX_CBSTATUS_OK;
    }
    free(path);
    char c;
    while (1) {
        c = fgetc(f);
        if (c == EOF) break;
        if (!isprint(c)) {
            PSCHSL_Resp_SetHeader(ctx, "Content-Type", "application/octet-stream");
            fseek(f, 0, SEEK_END);
            goto not_text;
        }
    }
    PSCHSL_Resp_SetHeader(ctx, "Content-Type", "text/plain");
    not_text:;
    long sz = ftell(f);
    char buf[512];
    snprintf(buf, 512, "%zu", sz);
    PSCHSL_Resp_SetHeader(ctx, "Content-Length", buf);
    fseek(f, 0, SEEK_SET);
    while (sz >= 512) {
        fread(buf, 1, 512, f);
        PSCHSL_Resp_PutBytes(ctx, 512, buf);
        sz -= 512;
    }
    if (sz) {
        fread(buf, 1, sz, f);
        PSCHSL_Resp_PutBytes(ctx, sz, buf);
    }
    return PSCHSL_CTX_CBSTATUS_OK;
}

int main() {
    state = PSCHSL_Create();
    signal(SIGINT, sigh);
    PSCHSL_SetOpt(state, PSCHSL_OPT_DEFAULTCTXOPT, PSCHSL_CTX_OPT_AUTOCONTENTTYPEHDR, 0);
    PSCHSL_SetOpt(state, PSCHSL_OPT_DEFAULTCTXOPT, PSCHSL_CTX_OPT_IMMEMIT, 1);
    PSCHSL_SetOpt(state, PSCHSL_OPT_DEFAULTCTXOPT, PSCHSL_CTX_OPT_OPTIPATH, 1);
    PSCHSL_SetRqstHandler("GET", callback, NULL);
    PSCHSL_Run(state);
    PSCHSL_Delete(state);
    return 0;
}
