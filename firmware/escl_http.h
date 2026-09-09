#ifndef MX490_ESCL_HTTP_H
#define MX490_ESCL_HTTP_H

#include "escl_adapter.h"

/* Serialize one eSCL response into an HTTP/1.1 response buffer. */
int mx490_escl_http_response(const struct mx490_escl_response *response,
                             unsigned char *out, size_t cap, size_t *written);

#endif
