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
    PSCHSL_CTX_OPT_COMP,               // enum PSCHSL_Ctx_Opt_Comp comp -- Set the compression mode -- default is NONE
    PSCHSL_CTX_OPT_AUTOCONTENTLENHDR,  // int enabled -- Enable/disable automatically adding a Content-Length response
                                       //   header if one does not exist -- default is enabled
    PSCHSL_CTX_OPT_AUTOSERVERHDR,      // int enabled -- Enable/disable automatically sending the Server response header
                                       //   ("PSCHSL/<version>") -- default is enabled
    PSCHSL_CTX_OPT_AUTOCONTENTTYPEHDR, // int enabled -- Enable/disable automatically setting the initial value of the
                                       //   Content-Type header to "text/html; charset=utf-8" -- default is enabled
    PSCHSL_CTX_OPT_IMMEMIT,            // int enabled -- If enabled, immediately (and continue to immediately) emit the
                                       //   response once the first PutText or PutBytes call is made -- default is
                                       //   disabled
    PSCHSL_CTX_OPT_OPTIPATH,           // int enabled -- If enabled, at most 1 SetStatus call can be made and must be
                                       //   made first, followed by any SetHeader calls which will not edit previously-
                                       //   set headers, and then any PutText or PutBytes calls; DelHeader becomes inert
                                       //   and AUTOCONTENTLENHDR is disabled -- default is disabled
    PSCHSL_CTX_OPT_SELECTTIME,         // uint64_t us -- Amount of microseconds to wait for new activity on a connection
                                       //   before checking if PSCHSL_Stop was called -- default is 1 sec
    PSCHSL_CTX_OPT_TIMEOUT,            // uint64_t us -- Amount of microseconds to wait for the client to send a valid
                                       //   request -- default is 15 sec
};
enum PSCHSL_Ctx_Opt_Comp {
    PSCHSL_CTX_OPT_COMP_NONE,
    PSCHSL_CTX_OPT_COMP_ZLIB
};

enum PSCHSL_Ctx_CBStatus {
    PSCHSL_CTX_CBSTATUS_OK,         // Send response and keep connection open
    PSCHSL_CTX_CBSTATUS_DISCONNECT, // Send response and close connection
    PSCHSL_CTX_CBSTATUS_ABORT       // Close connection without sending response
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

// Set the HTTP status code and text
//   - If text is NULL, infer the text from the code (empty if the code is not recognized)
//   - Returns non-zero for success, zero for failure
int PSCHSL_Resp_SetStatus(struct PSCHSL_Ctx*, int code, const char* text);
// Get the current HTTP status code and text
//   - Returns NULL or a string that is valid until the callback returns or SetStatus is called
const char* PSCHSL_Resp_GetStatus(struct PSCHSL_Ctx*, int* code);

// Sets a response header
//   - Returns non-zero for success, zero for failure
int PSCHSL_Resp_SetHeader(struct PSCHSL_Ctx*, const char* name, const char* value);
// Deletes a response header
void PSCHSL_Resp_DelHeader(struct PSCHSL_Ctx*, const char* name);

// Write text response content
//   - Returns non-zero for success, zero for failure
int PSCHSL_Resp_PutText(struct PSCHSL_Ctx*, const char* text);
// Write binary response content
//   - Returns non-zero for success, zero for failure
int PSCHSL_Resp_PutBytes(struct PSCHSL_Ctx*, size_t sz, void* data);

//// ---------------------- ////
//// ----- MAIN STATE ----- ////
//// ---------------------- ////

struct PSCHSL;
enum PSCHSL_Opt {
    PSCHSL_OPT_BINDADDR,        // char* addr -- Address to bind to -- default is 0.0.0.0
    PSCHSL_OPT_BINDPORT,        // unsigned port -- Port to bind to -- default is 8080
    PSCHSL_OPT_DEFAULTCTXOPT,   // enum PSCHSL_Ctx_Opt ctxopt, ... -- Set default options for I/O contexts
    PSCHSL_OPT_DEFAULTRQSTMOPT, // char* method, enum PSCHSL_Ctx_Opt ctxopt, ... -- Set default options for a request
                                //   method
    PSCHSL_OPT_MAXURILEN,       // size_t maxlen -- Any request target or URI longer than this will result in error 414
                                //   -- default is 64KiB
    PSCHSL_OPT_CANONURI,        // int enabled -- Enable/disable resolving . and .. in URIs
    PSCHSL_OPT_MAXRQSTHDRLEN,   // size_t maxlen -- Any headers longer than this will result in error 431 -- default is
                                //   SIZE_MAX
    PSCHSL_OPT_MAXRQSTHDRMEM,   // size_t maxlen -- Max amount of memory to allocate for storing headers; going over
                                //   will result in error 431 -- default is 1MiB
    PSCHSL_OPT_THREADPOOL_MIN,  // unsigned min -- Min amount of threads -- default is 2
    PSCHSL_OPT_THREADPOOL_MAX,  // unsigned max -- Max amount of threads -- default is 16
    PSCHSL_OPT_THREADPOOL_OLD,  // uint64_t us -- When above min, stop threads that have no activity for at least the
                                //   given amount of microseconds -- default is 0
    PSCHSL_OPT_SELECTTIME,      // uint64_t us -- Amount of microseconds to wait for a new connection, or UINT64_MAX to
                                //   block; if using PSCHSL_Step and doing your own timing/waiting, you may want to set
                                //   this to 0 -- default is UINT64_MAX
    PSCHSL_OPT_CHKSTOPAFTERSEL, // int enabled -- Enable/disable checking if PSCHSL_Stop was called after waiting on a
                                //   new connection (if so and a new connection was made, immediately close it and exit)
                                //   -- default is disabled
    PSCHSL_OPT_CBONERROR        // int enabled -- Enable/disable calling the request handler to set headers and content
                                //   on errors; the default status will be set to a code appropriate for the error that
                                //   was encountered instead of the usual 200 "OK" -- default is enabled
};

// Creates a PSCHSL state
//   - Returns NULL on failure
struct PSCHSL* PSCHSL_Create(void);
// Destroys a PSCHSL state
void PSCHSL_Destroy(struct PSCHSL*);
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
int PSCHSL_SetRqstHandler(struct PSCHSL*, const char* method, PSCHSL_Ctx_Callback cb, void* userdata);
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
