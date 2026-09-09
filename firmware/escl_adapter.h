#ifndef MX490_ESCL_ADAPTER_H
#define MX490_ESCL_ADAPTER_H

#include <stddef.h>

/* Platform-neutral eSCL adapter boundary.
 *
 * The callbacks are the only printer-specific part. On MX490 they should
 * forward to the existing X-CISSE-SCAN/WSD scan-job implementation.
 */
struct mx490_escl_ops {
    int (*submit_job)(const char *request_xml, size_t request_len,
                      unsigned long *job_id, void *ctx);
    int (*next_document)(unsigned long job_id, const unsigned char **data,
                         size_t *len, int *last, void *ctx);
    int (*cancel_job)(unsigned long job_id, void *ctx);
    void *ctx;
};

struct mx490_escl_request {
    const char *method;
    const char *path;
    const unsigned char *body;
    size_t body_len;
};

struct mx490_escl_response {
    int status;
    const char *content_type;
    const char *location;
    const unsigned char *body;
    size_t body_len;
};

int mx490_escl_handle(const struct mx490_escl_request *request,
                      const struct mx490_escl_ops *ops,
                      struct mx490_escl_response *response);

#endif
