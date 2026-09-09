#ifndef MX490_BJNP_TRANSPORT_H
#define MX490_BJNP_TRANSPORT_H

#include "bjnp_stream.h"

typedef int (*mx490_bjnp_send_fn)(void *ctx, const uint8_t *data, size_t length);

struct mx490_bjnp_transport {
    mx490_bjnp_send_fn send;
    void *ctx;
    uint32_t next_sequence;
    uint32_t waiting_sequence;
    uint8_t *response;
    size_t response_cap;
    size_t response_len;
    int response_ready;
    struct mx490_bjnp_stream stream;
};

void mx490_bjnp_transport_init(struct mx490_bjnp_transport *t,
                               uint8_t *stream_payload, size_t payload_cap,
                               mx490_bjnp_send_fn send, void *ctx);
int mx490_bjnp_transport_feed(struct mx490_bjnp_transport *t,
                              const uint8_t *data, size_t length);
int mx490_bjnp_transport_exchange(struct mx490_bjnp_transport *t,
                                  uint16_t kind, const uint8_t *request,
                                  size_t request_len, uint8_t *response,
                                  size_t response_cap, size_t *response_len);

#endif
