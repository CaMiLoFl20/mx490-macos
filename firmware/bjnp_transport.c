#include "bjnp_transport.h"

static int receive_frame(const struct mx490_bjnp_frame *f, const uint8_t *p,
                         size_t n, void *ctx)
{
    struct mx490_bjnp_transport *t = (struct mx490_bjnp_transport *)ctx;
    size_t i;
    if (t == 0 || f == 0 || f->sequence != t->waiting_sequence ||
        n > t->response_cap || (n != 0 && t->response == 0)) return -1;
    for (i = 0; i < n; ++i) t->response[i] = p[i];
    t->response_len = n; t->response_ready = 1; return 0;
}

void mx490_bjnp_transport_init(struct mx490_bjnp_transport *t,
                               uint8_t *stream_payload, size_t payload_cap,
                               mx490_bjnp_send_fn send, void *ctx)
{
    if (t == 0) return;
    t->send = send; t->ctx = ctx; t->next_sequence = 1;
    t->waiting_sequence = 0; t->response = 0; t->response_cap = 0;
    t->response_len = 0; t->response_ready = 0;
    mx490_bjnp_stream_init(&t->stream, stream_payload, payload_cap,
                           receive_frame, t);
}

int mx490_bjnp_transport_feed(struct mx490_bjnp_transport *t,
                              const uint8_t *data, size_t length)
{
    if (t == 0) return -1;
    return mx490_bjnp_stream_feed(&t->stream, data, length);
}

int mx490_bjnp_transport_exchange(struct mx490_bjnp_transport *t,
                                  uint16_t kind, const uint8_t *request,
                                  size_t request_len, uint8_t *response,
                                  size_t response_cap, size_t *response_len)
{
    uint8_t frame[MX490_BJNP_HEADER_SIZE + 2048];
    struct mx490_bjnp_frame f;
    size_t written;
    if (t == 0 || t->send == 0 || response_len == 0 || request_len > 2048) return -1;
    t->waiting_sequence = t->next_sequence++;
    t->response = response; t->response_cap = response_cap; t->response_ready = 0;
    f.version = 1; f.channel = 1; f.sequence = t->waiting_sequence; f.kind = kind;
    f.payload_len = (uint32_t)request_len;
    if (mx490_bjnp_encode(&f, request, request_len, frame, sizeof(frame), &written) != 0 ||
        t->send(t->ctx, frame, written) != 0) return -1;
    /* The platform socket loop must call feed() as bytes arrive. */
    if (!t->response_ready) return -2;
    *response_len = t->response_len; return 0;
}
