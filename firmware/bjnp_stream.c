#include "bjnp_stream.h"

static int header_complete(const uint8_t *h)
{
    return h[0] == 'B' && h[1] == 'J' && h[2] == 'N' && h[3] == 'P';
}
static uint16_t rd16(const uint8_t *p) { return (uint16_t)(((uint16_t)p[0] << 8) | p[1]); }
static uint32_t rd32(const uint8_t *p) { return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3]; }

void mx490_bjnp_stream_init(struct mx490_bjnp_stream *s, uint8_t *payload,
                            size_t cap, mx490_bjnp_frame_fn fn, void *ctx)
{
    size_t i;
    if (s == 0) return;
    for (i = 0; i < sizeof(*s); ++i) ((uint8_t *)s)[i] = 0;
    s->payload = payload; s->payload_cap = cap; s->on_frame = fn; s->ctx = ctx;
}

int mx490_bjnp_stream_feed(struct mx490_bjnp_stream *s, const uint8_t *data,
                           size_t length)
{
    size_t used = 0;
    if (s == 0 || (data == 0 && length != 0) || s->on_frame == 0) return -1;
    while (used < length) {
        if (s->header_used < MX490_BJNP_HEADER_SIZE) {
            s->header[s->header_used++] = data[used++];
            if (s->header_used == MX490_BJNP_HEADER_SIZE) {
                if (!header_complete(s->header)) return -1;
                s->frame.version = s->header[4]; s->frame.channel = s->header[5];
                s->frame.sequence = rd32(s->header + 6);
                s->frame.kind = rd16(s->header + 10);
                s->frame.payload_len = rd32(s->header + 12);
                if (s->frame.payload_len > s->payload_cap) return -1;
                s->payload_used = 0;
            }
            continue;
        }
        {
            size_t n = length - used;
            if (n > s->frame.payload_len - s->payload_used)
                n = s->frame.payload_len - s->payload_used;
            if (n && s->payload == 0) return -1;
            for (size_t i = 0; i < n; ++i) s->payload[s->payload_used + i] = data[used + i];
            used += n; s->payload_used += n;
        }
        if (s->payload_used == s->frame.payload_len) {
            if (s->on_frame(&s->frame, s->payload, s->payload_used, s->ctx) != 0) return -1;
            s->header_used = 0; s->payload_used = 0;
        }
    }
    return 0;
}
