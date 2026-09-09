#ifndef MX490_BJNP_STREAM_H
#define MX490_BJNP_STREAM_H

#include "bjnp_frame.h"

typedef int (*mx490_bjnp_frame_fn)(const struct mx490_bjnp_frame *frame,
                                   const uint8_t *payload, size_t length,
                                   void *ctx);

struct mx490_bjnp_stream {
    uint8_t header[MX490_BJNP_HEADER_SIZE];
    size_t header_used;
    uint8_t *payload;
    size_t payload_cap;
    size_t payload_used;
    struct mx490_bjnp_frame frame;
    mx490_bjnp_frame_fn on_frame;
    void *ctx;
};

void mx490_bjnp_stream_init(struct mx490_bjnp_stream *stream,
                            uint8_t *payload, size_t payload_cap,
                            mx490_bjnp_frame_fn on_frame, void *ctx);
int mx490_bjnp_stream_feed(struct mx490_bjnp_stream *stream,
                           const uint8_t *data, size_t length);

#endif
