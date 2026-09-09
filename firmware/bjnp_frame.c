#include "bjnp_frame.h"

static void be16(uint8_t *p, uint16_t v) { p[0] = (uint8_t)(v >> 8); p[1] = (uint8_t)v; }
static void be32(uint8_t *p, uint32_t v) { p[0] = (uint8_t)(v >> 24); p[1] = (uint8_t)(v >> 16); p[2] = (uint8_t)(v >> 8); p[3] = (uint8_t)v; }
static uint16_t rd16(const uint8_t *p) { return (uint16_t)(((uint16_t)p[0] << 8) | p[1]); }
static uint32_t rd32(const uint8_t *p) { return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3]; }

int mx490_bjnp_encode(const struct mx490_bjnp_frame *f, const uint8_t *payload,
                      size_t n, uint8_t *out, size_t cap, size_t *written)
{
    if (f == 0 || out == 0 || written == 0 || (payload == 0 && n != 0) ||
        n > 0xffffffffu || cap < MX490_BJNP_HEADER_SIZE + n) return -1;
    out[0] = 'B'; out[1] = 'J'; out[2] = 'N'; out[3] = 'P';
    out[4] = f->version; out[5] = f->channel; out[6] = 0; out[7] = 0;
    be32(out + 6, f->sequence); be16(out + 10, f->kind); be32(out + 12, f->payload_len);
    if (n) { size_t i; for (i = 0; i < n; ++i) out[16 + i] = payload[i]; }
    *written = MX490_BJNP_HEADER_SIZE + n; return 0;
}

int mx490_bjnp_decode(const uint8_t *in, size_t len, struct mx490_bjnp_frame *f,
                      const uint8_t **payload)
{
    uint32_t n;
    if (in == 0 || f == 0 || payload == 0 || len < 16 || in[0] != 'B' ||
        in[1] != 'J' || in[2] != 'N' || in[3] != 'P') return -1;
    n = rd32(in + 12); if (n > len - 16) return -1;
    f->version = in[4]; f->channel = in[5]; f->sequence = rd32(in + 6);
    f->kind = rd16(in + 10); f->payload_len = n; *payload = in + 16; return 0;
}
