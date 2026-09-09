#ifndef MX490_BJNP_FRAME_H
#define MX490_BJNP_FRAME_H

#include <stddef.h>
#include <stdint.h>

#define MX490_BJNP_HEADER_SIZE 16u

struct mx490_bjnp_frame {
    uint8_t version;
    uint8_t channel;
    uint32_t sequence;
    uint16_t kind;
    uint32_t payload_len;
};

int mx490_bjnp_encode(const struct mx490_bjnp_frame *frame,
                      const uint8_t *payload, size_t payload_len,
                      uint8_t *out, size_t cap, size_t *written);
int mx490_bjnp_decode(const uint8_t *in, size_t len,
                      struct mx490_bjnp_frame *frame,
                      const uint8_t **payload);

#endif
