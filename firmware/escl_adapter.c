#include "escl_adapter.h"

#include <string.h>

static const unsigned char capabilities[] =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<ScannerCapabilities xmlns=\"http://schemas.microsoft.com/windows/2006/01/wdp/scan\">"
    "<Version>2.0</Version><MakeAndModel>Canon MX490 series</MakeAndModel>"
    "<Platen><Width>216</Width><Height>297</Height></Platen>"
    "<PlatenMinimumOpticalResolution>75</PlatenMinimumOpticalResolution>"
    "<PlatenMaximumOpticalResolution>600</PlatenMaximumOpticalResolution>"
    "</ScannerCapabilities>";

static const unsigned char status[] =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<ScannerStatus xmlns=\"http://schemas.microsoft.com/windows/2006/01/wdp/scan\">"
    "<ScannerState>Idle</ScannerState><Jobs><Job/></Jobs></ScannerStatus>";

static int path_is(const char *path, const char *expected)
{
    return path != 0 && strcmp(path, expected) == 0;
}

static void set_response(struct mx490_escl_response *response, int status_code,
                         const char *type, const unsigned char *body,
                         size_t len)
{
    response->status = status_code;
    response->content_type = type;
    response->body = body;
    response->body_len = len;
}

int mx490_escl_handle(const struct mx490_escl_request *request,
                      const struct mx490_escl_ops *ops,
                      struct mx490_escl_response *response)
{
    static unsigned long current_job;
    const unsigned char *data;
    size_t len;
    int last;
    int rc;

    if (request == 0 || response == 0 || ops == 0 || request->method == 0 ||
        request->path == 0) return -1;

    if (strcmp(request->method, "GET") == 0 &&
        path_is(request->path, "/eSCL/ScannerCapabilities")) {
        set_response(response, 200, "application/xml", capabilities,
                     sizeof(capabilities) - 1);
        return 0;
    }
    if (strcmp(request->method, "GET") == 0 &&
        path_is(request->path, "/eSCL/ScannerStatus")) {
        set_response(response, 200, "application/xml", status,
                     sizeof(status) - 1);
        return 0;
    }
    if (strcmp(request->method, "POST") == 0 &&
        path_is(request->path, "/eSCL/ScanJobs")) {
        if (ops->submit_job == 0) return -1;
        rc = ops->submit_job((const char *)request->body, request->body_len,
                             &current_job, ops->ctx);
        if (rc != 0) {
            set_response(response, 500, "text/plain", 0, 0);
            return rc;
        }
        set_response(response, 201, "text/plain", 0, 0);
        return 0;
    }
    if (strcmp(request->method, "GET") == 0 &&
        path_is(request->path, "/eSCL/ScanJobs/1/NextDocument")) {
        if (ops->next_document == 0) return -1;
        rc = ops->next_document(current_job, &data, &len, &last, ops->ctx);
        if (rc != 0) {
            set_response(response, 404, "text/plain", 0, 0);
            return rc;
        }
        set_response(response, 200, "image/jpeg", data, len);
        return last ? 1 : 0;
    }
    if (strcmp(request->method, "DELETE") == 0 &&
        path_is(request->path, "/eSCL/ScanJobs/1")) {
        if (ops->cancel_job == 0) return -1;
        rc = ops->cancel_job(current_job, ops->ctx);
        set_response(response, rc == 0 ? 200 : 500, "text/plain", 0, 0);
        return rc;
    }
    set_response(response, 404, "text/plain", 0, 0);
    return -1;
}
