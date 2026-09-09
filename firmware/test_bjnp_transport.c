#include "bjnp_transport.h"
#include <assert.h>

struct fake { struct mx490_bjnp_transport *t; uint8_t *wire; size_t n; };
static int send_frame(void *ctx, const uint8_t *p, size_t n) {
    struct fake *f = (struct fake *)ctx; struct mx490_bjnp_frame h;
    const uint8_t *payload_ptr;
    uint8_t payload[8] = {0x06, 0x06}; uint8_t out[32]; size_t w;
    assert(mx490_bjnp_decode(p, n, &h, &payload_ptr) == 0);
    h.kind = 0x9000; h.payload_len = 2;
    assert(mx490_bjnp_encode(&h, payload, 2, out, sizeof(out), &w) == 0);
    assert(mx490_bjnp_transport_feed(f->t, out, 5) == 0);
    assert(mx490_bjnp_transport_feed(f->t, out + 5, w - 5) == 0);
    f->n = n; return 0;
}
int main(void) {
    uint8_t storage[16], response[8], request[2] = {1, 2}; size_t n;
    struct mx490_bjnp_transport t; struct fake f = {&t, 0, 0};
    mx490_bjnp_transport_init(&t, storage, sizeof(storage), send_frame, &f);
    assert(mx490_bjnp_transport_exchange(&t, 0x1000, request, 2, response, sizeof(response), &n) == 0);
    assert(n == 2 && response[0] == 0x06 && response[1] == 0x06);
    return 0;
}
