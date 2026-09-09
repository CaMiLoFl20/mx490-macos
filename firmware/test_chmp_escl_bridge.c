#include "chmp_escl_bridge.h"

#include <assert.h>
#include <string.h>

static int started;
static int start(const struct mx490_chmp_scan_params *p, unsigned long *id, void *ctx)
{
    (void)ctx; assert(p->x_dpi == 300 && p->width == 2550); started = 1; *id = 7; return 0;
}
static int next(unsigned long id, unsigned char *b, size_t cap, size_t *n, int *last, void *ctx)
{
    (void)ctx; assert(id == 7 && cap >= 3); b[0] = 1; b[1] = 2; b[2] = 3; *n = 3; *last = 1; return 0;
}
static int cancel(unsigned long id, void *ctx) { (void)ctx; return id == 7 ? 0 : -1; }

int main(void)
{
    unsigned char buffer[8];
    struct mx490_chmp_escl_bridge bridge;
    struct mx490_chmp_escl_transport transport = {start, next, cancel, 0};
    struct mx490_escl_ops ops;
    struct mx490_escl_request req = {"POST", "/eSCL/ScanJobs", 0, 0};
    struct mx490_escl_response response;
    mx490_chmp_escl_bridge_init(&bridge, &transport, buffer, sizeof(buffer));
    mx490_chmp_escl_bridge_ops(&bridge, &ops);
    assert(mx490_escl_handle(&req, &ops, &response) == 0 && started);
    req = (struct mx490_escl_request){"GET", "/eSCL/ScanJobs/00000000-0000-4000-8000-000000000007/NextDocument", 0, 0};
    assert(mx490_escl_handle(&req, &ops, &response) == 1 && response.body_len == 3);
    req = (struct mx490_escl_request){"DELETE", "/eSCL/ScanJobs/00000000-0000-4000-8000-000000000007", 0, 0};
    assert(mx490_escl_handle(&req, &ops, &response) == 0);
    return 0;
}
