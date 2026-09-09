#include "bjnp_frame.h"
#include <assert.h>

int main(void)
{
    static const uint8_t payload[] = {0xdb, 0x20, 0, 0};
    uint8_t wire[32]; size_t n; const uint8_t *p; struct mx490_bjnp_frame f, g;
    f.version = 2; f.channel = 0x21; f.sequence = 13; f.kind = 0x17; f.payload_len = 4;
    assert(mx490_bjnp_encode(&f, payload, sizeof(payload), wire, sizeof(wire), &n) == 0);
    assert(n == 20 && wire[0] == 'B' && wire[5] == 0x21);
    assert(mx490_bjnp_decode(wire, n, &g, &p) == 0 && g.sequence == 13 && g.kind == 0x17 &&
           g.payload_len == 4 && p[0] == 0xdb && p[1] == 0x20);
    return 0;
}
