#include "bjnp_stream.h"
#include <assert.h>

static int seen;
static int frame(const struct mx490_bjnp_frame *f, const uint8_t *p, size_t n, void *ctx)
{
    (void)ctx; assert(f->sequence == 9 && f->kind == 0x17 && n == 3);
    assert(p[0] == 1 && p[2] == 3); ++seen; return 0;
}
int main(void)
{
    uint8_t wire[32], payload[8]; size_t n; struct mx490_bjnp_frame f = {2, 0x21, 9, 0x17, 3};
    struct mx490_bjnp_stream s; f.payload_len = 3;
    assert(mx490_bjnp_encode(&f, (const uint8_t[]){1,2,3}, 3, wire, sizeof(wire), &n) == 0);
    mx490_bjnp_stream_init(&s, payload, sizeof(payload), frame, 0);
    assert(mx490_bjnp_stream_feed(&s, wire, 5) == 0);
    assert(mx490_bjnp_stream_feed(&s, wire + 5, n - 5) == 0 && seen == 1);
    return 0;
}
