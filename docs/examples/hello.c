// Hello world example

#include <pschsl/pschsl.h>
#include <signal.h>

static struct PSCHSL* state;

static void sigh(int sig) {
    PSCHSL_Stop(state);
}

static enum PSCHSL_Ctx_CBStatus callback(struct PSCHSL_Ctx* ctx, void* userdata) {
    PSCHSL_Resp_SetHeader(ctx, "Content-Language", "en-US");
    PSCHSL_Resp_PutText(ctx, "<html><body><h1>Hello World from ");
    PSCHSL_Resp_PutText(ctx, PSCHSL_Rqst_GetTarget(ctx));
    char* tmp = PSCHSL_Rqst_GetHeader(ctx, "Host");
    if (tmp) {
        PSCHSL_Resp_PutText(ctx, " on ");
        PSCHSL_Resp_PutText(ctx, tmp);
    }
    PSCHSL_Resp_PutText(ctx, "!</h1></body></html>");
    return PSCHSL_CTX_CBSTATUS_OK;
}

int main() {
    state = PSCHSL_Create();
    signal(SIGINT, sigh);
    PSCHSL_SetRqstHandler("GET", callback, NULL);
    PSCHSL_Run(state);
    PSCHSL_Delete(state);
    return 0;
}
