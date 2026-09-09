#ifndef MX490_CHMP_SESSION_H
#define MX490_CHMP_SESSION_H

#include "chmp_scan.h"

typedef int (*mx490_chmp_exchange_fn)(void *ctx, const uint8_t *request,
                                      size_t request_len, uint8_t *response,
                                      size_t response_cap, size_t *response_len);

struct mx490_chmp_session {
    mx490_chmp_exchange_fn exchange;
    void *ctx;
    uint8_t active;
    uint8_t ready;
};

int mx490_chmp_session_start(struct mx490_chmp_session *s,
                              const struct mx490_chmp_scan_params *params);
int mx490_chmp_session_poll(struct mx490_chmp_session *s, uint8_t *state);
int mx490_chmp_session_read(struct mx490_chmp_session *s, uint8_t *data,
                            size_t cap, size_t *len, int *last);
int mx490_chmp_session_cancel(struct mx490_chmp_session *s);

#endif
