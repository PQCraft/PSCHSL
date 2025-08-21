#ifndef PSCHSL_H
#define PSCHSL_H

// Scroll to the bottom for the main state interface
// It is down there for forward-decl reasons

//// ------------------------ ////
//// ----- DEPENDENCIES ----- ////
//// ------------------------ ////

#include <stddef.h>

//// ---------------------------- ////
//// ----- STATELESS THINGS ----- ////
//// ---------------------------- ////

#define PSCHSL_VERSION 2025082100

unsigned PSCHSL_GetVersion(void);

//// ----------------------- ////
//// ----- I/O CONTEXT ----- ////
//// ----------------------- ////

struct PSCHSL_Ctx;
enum PSCHSL_Ctx_Opt {
    PSCHSL_CTX_OPT_USERDATA,             // void* -- User data to pass to the callbacks
    PSCHSL_CTX_OPT_CACHE,                // int enabled -- Enable/disable caching this response
    PSCHSL_CTX_OPT_COMP,                 // enum PSCHSL_Ctx_Opt_Comp comp -- Set the compression mode (default is NONE)
    PSCHSL_CTX_OPT_AUTOCONTENTLENHEADER, // int enabled -- Enable/disable automatically sending the Content-Length
                                         //   response header (default is disabled)
    PSCHSL_CTX_OPT_AUTOSERVERHEADER,     // int enabled -- Enable/disable automatically sending the Server response
                                         //   header ("PSCHSL/<version>") (default is enabled)
    PSCHSL_CTX_OPT_IMMEMIT,              // int enabled -- If enabled, immediately (and continue to immediately) emit
                                         //   the response once the first Put[Text]Content call is made; if used with
                                         //   OPTIPATH, output everything immediately (default is disabled)
    PSCHSL_CTX_OPT_OPTIPATH              // int enabled -- If enabled, SetStatus must be called first, followed by any
                                         //   SetHeader calls, and then Put[Text]Content; DelHeader becomes inert
                                         //   (default is disabled)
};
enum PSCHSL_Ctx_Opt_Comp {
    PSCHSL_CTX_OPT_COMP_NONE,
    PSCHSL_CTX_OPT_COMP_ZLIB
};

enum PSCHSL_Ctx_CBStatus {
    PSCHSL_CTX_CBSTATUS_OK,
    PSCHSL_CTX_CBSTATUS_CACHED,
    PSCHSL_CTX_CBSTATUS_DISCONNECT
};
typedef enum PSCHSL_Ctx_CBStatus (*PSCHSL_Ctx_Callback)(struct PSCHSL_Ctx*, void* userdata);

// Set an option
//   - Returns non-zero for success, zero for failure
int PSCHSL_Ctx_SetOpt(struct PSCHSL_Ctx*, enum PSCHSL_Ctx_Opt, ...);

// Get the PSCHSL state the given context is associated with
struct PSCHSL* PSCHSL_Ctx_GetState(struct PSCHSL_Ctx*);

//// ------------------- ////
//// ----- REQUEST ----- ////
//// ------------------- ////

// Return the target given in the method parsed as a URI
//   - On success, returns a string that is valid until the callback returns, or NULL on failure
const char* PSCHSL_Rqst_GetTarget(struct PSCHSL_Ctx*);
// Return the target given in the method unparsed
//   - If incquery is non-zero, include the query params
//   - On success, returns a string that is valid until the callback returns, or NULL on failure
const char* PSCHSL_Rqst_GetRawTarget(struct PSCHSL_Ctx*, int incquery);

// Find a query param provided in the URI
//   - On success, returns a string that is valid until the callback returns, or NULL on failure
const char* PSCHSL_Rqst_GetQueryParam(struct PSCHSL_Ctx*, const char* name);
// Return the number of query params
size_t PSCHSL_Rqst_GetQueryParamCount(struct PSCHSL_Ctx*);
// Get the name of a query param by index
//   - On success, returns a string that is valid until the callback returns, or NULL on failure
const char* PSCHSL_Rqst_GetQueryParamNameByIndex(struct PSCHSL_Ctx*, size_t i);
// Get a query param by index
//   - On success, returns a string that is valid until the callback returns, or NULL on failure
const char* PSCHSL_Rqst_GetQueryParamByIndex(struct PSCHSL_Ctx*, size_t i);

// Find a header
//   - On success, returns a string that is valid until the callback returns, or NULL on failure
const char* PSCHSL_Rqst_GetHeader(struct PSCHSL_Ctx*, const char* name);
// Return the number of headers
size_t PSCHSL_Rqst_GetHeaderCount(struct PSCHSL_Ctx*, const char* name);
// Get the name of a header by index
//   - On success, returns a string that is valid until the callback returns, or NULL on failure
const char* PSCHSL_Rqst_GetHeaderNameByIndex(struct PSCHSL_Ctx*, size_t i);
// Get a header by index
//   - On success, returns a string that is valid until the callback returns, or NULL on failure
const char* PSCHSL_Rqst_GetHeaderByIndex(struct PSCHSL_Ctx*, size_t i);

// Read in the request content
//   - Returns the amount of bytes successfully read
size_t PSCHSL_Rqst_ReadContent(struct PSCHSL_Ctx*, size_t len, char* out);

//// -------------------- ////
//// ----- RESPONSE ----- ////
//// -------------------- ////

// Set the HTTP status text
//   - Returns non-zero for success, zero for failure; if not called, "200 OK" is assumed
int PSCHSL_Resp_SetStatus(struct PSCHSL_Ctx*, const char* text);

// Sets a response header
//   - Returns non-zero for success, zero for failure
int PSCHSL_Resp_SetHeader(struct PSCHSL_Ctx*, const char* name, const char* value);
// Deletes a response header
void PSCHSL_Resp_DelHeader(struct PSCHSL_Ctx*, const char* name);

// Write response content
//   - Returns non-zero for success, zero for failure
int PSCHSL_Resp_PutContent(struct PSCHSL_Ctx*, size_t sz, const char* data);
// Write response content from a null-terminated string
//   - Returns non-zero for success, zero for failure
int PSCHSL_Resp_PutTextContent(struct PSCHSL_Ctx*, const char* text);

//// ---------------------- ////
//// ----- MAIN STATE ----- ////
//// ---------------------- ////

struct PSCHSL;
enum PSCHSL_Opt {
    PSCHSL_OPT_BINDADDR,        // char* addr -- Address to bind to (default is 0.0.0.0)
    PSCHSL_OPT_BINDPORT,        // unsigned port -- Port to bind to (default is 8080)
    PSCHSL_OPT_DEFAULTCTXOPT,   // enum PSCHSL_Ctx_Opt ctxopt, ... -- Set default options for I/O contexts
    PSCHSL_OPT_DEFAULTRQSTMOPT, // char* method, enum PSCHSL_Ctx_Opt ctxopt, ... -- Set default options for a request
                                //   method
    PSCHSL_OPT_CACHE,           // int enabled -- Enable (non-0) or disable (0) caching responses (default is enabled)
    PSCHSL_OPT_CACHE_MAXENT,    // size_t maxent -- Max amount of responses to cache (default is 4096)
    PSCHSL_OPT_CACHE_MAXSIZE,   // size_t maxsz -- Any response longer than this will not be cached (default is 1MiB)
    PSCHSL_OPT_CACHE_MAXMEM,    // size_t maxmem -- Max amount of memory to use for caching response text (default is
                                //   256MiB)
    PSCHSL_OPT_CACHE_MAXSAC,    // size_t maxsac -- Max amount of cache entries to sacrifice when a cache limit is hit
                                //   (default is 3)
    PSCHSL_OPT_CACHE_SACLARGE,  // int enabled -- Enable/disable sacraficing large entries instead of old entries
                                //   (default is disabled)
    PSCHSL_OPT_MAXURILEN,       // size_t maxlen -- Any request target or URI longer than this will result in error 414
                                //   (default is 64KiB)
    PSCHSL_OPT_MAXRQSTHDRLEN,   // size_t maxlen -- Any headers longer than this will result in error 431 (default is
                                //   SIZE_MAX)
    PSCHSL_OPT_MAXRQSTHDRMEM,   // size_t maxlen -- Max amount of memory to allocate for storing headers; going over
                                //   will result in error 431 (default is 1MiB)
    PSCHSL_OPT_THREADPOOL_MIN,  // unsigned min -- Min amount of threads (default is 2)
    PSCHSL_OPT_THREADPOOL_MAX,  // unsigned max -- Max amount of threads (default is 16)
    PSCHSL_OPT_THREADPOOL_OLD,  // uint64_t us -- When above min, stop threads that have no activity for at least the
                                //   given amount of microseconds (default is 0)
    PSCHSL_OPT_SELECTTIME,      // uint64_t us -- Amount of microseconds to wait for a new connection, or UINT64_MAX to
                                //   block; if using PSCHSL_Step and doing your own timing/waiting, you may want to set
                                //   this to 0 (default is UINT64_MAX)
    PSCHSL_OPT_CHKSTOPAFTERSEL  // int enabled -- Enable/disable checking if PSCHSL_Stop was called after waiting on a
                                //   new connection (if so and a new connection was made, immediately close it and exit)
                                //   (default is disabled)
};

// Initializes a PSCHSL state
//   - Returns non-zero for success, zero for failure
int PSCHSL_Init(struct PSCHSL*);
// Deinitializes a PSCHSL state
void PSCHSL_DeInit(struct PSCHSL*);
// Runs a PSCHSL state until PSCHSL_Stop is called
void PSCHSL_Run(struct PSCHSL*);
// Does a single async run
//   - Returns zero if PSCHSL_Stop was called before, non-zero otherwise
int PSCHSL_Step(struct PSCHSL*);
// Requests that a PSCHSL state cease operation
void PSCHSL_Stop(struct PSCHSL*);
// Checks is PSCHSL_Stop was called
int PSCHSL_IsStopRqstd(struct PSCHSL*);

// Sets an option
//   - Returns non-zero for success, zero for failure
int PSCHSL_SetOpt(struct PSCHSL*, enum PSCHSL_Opt, ...);

// Bind a handler callback to a request method
//   - Returns non-zero for success, zero for failure
int PSCHSL_SetRqstHandler(struct PSCHSL*, const char* method, PSCHSL_Ctx_Callback cb);
// Unbind a handler from a request method
//   - If delopt is non-zero, also reset the options set with DEFAULTRQSTMOPT
void PSCHSL_DelRqstHandler(struct PSCHSL*, const char* method, int delopt);

//// ------------------------- ////
//// ----- LEGACY COMPAT ----- ////
//// ------------------------- ////

#ifndef PSCHSL_NOLEGACY
    #include "legacy.h"
#endif

#endif
