#include "escl_adapter.h"

#include <assert.h>
#include <string.h>

static int submitted;
static int submit(const char *xml, size_t len, unsigned long *id, void *ctx)
{
    (void)ctx;
    submitted = (xml != 0 && len == 3);
    *id = 42;
    return 0;
}
static int next(unsigned long id, const unsigned char **data, size_t *len,
                int *last, void *ctx)
{
    static const unsigned char jpeg[] = {0xff, 0xd8, 0xff, 0xd9};
    (void)ctx;
    assert(id == 42);
    *data = jpeg; *len = sizeof(jpeg); *last = 1;
    return 0;
}
static int cancel(unsigned long id, void *ctx)
{
    (void)ctx; return id == 42 ? 0 : -1;
}

int main(void)
{
    struct mx490_escl_ops ops = {submit, next, cancel, 0};
    struct mx490_escl_response response;
    struct mx490_escl_request request;
    unsigned char body[] = {'x', 'm', 'l'};
    request = (struct mx490_escl_request){"GET", "/eSCL/ScannerCapabilities", 0, 0};
    assert(mx490_escl_handle(&request, &ops, &response) == 0 && response.status == 200);
    request = (struct mx490_escl_request){"POST", "/eSCL/ScanJobs", body, sizeof(body)};
    assert(mx490_escl_handle(&request, &ops, &response) == 0 && submitted &&
           response.location != 0);
    request = (struct mx490_escl_request){"GET", "/eSCL/ScanJobs/00000000-0000-4000-8000-000000000042/NextDocument", 0, 0};
    assert(mx490_escl_handle(&request, &ops, &response) == 1 && response.body_len == 4);
    request = (struct mx490_escl_request){"DELETE", "/eSCL/ScanJobs/00000000-0000-4000-8000-000000000042", 0, 0};
    assert(mx490_escl_handle(&request, &ops, &response) == 0 && response.status == 200);
    return 0;
}
