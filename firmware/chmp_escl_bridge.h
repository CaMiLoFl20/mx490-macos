#ifndef MX490_CHMP_ESCL_BRIDGE_H
#define MX490_CHMP_ESCL_BRIDGE_H

#include "escl_adapter.h"
#include "chmp_scan.h"

/* The callback owns the verified MX490 XML + binary sequence on port 8612. */
struct mx490_chmp_escl_transport {
    int (*start)(const struct mx490_chmp_scan_params *params,
                 unsigned long *job_id, void *ctx);
    int (*next)(unsigned long job_id, unsigned char *buffer, size_t capacity,
                size_t *length, int *last, void *ctx);
    int (*cancel)(unsigned long job_id, void *ctx);
    void *ctx;
};

struct mx490_chmp_escl_bridge {
    struct mx490_chmp_escl_transport transport;
    unsigned char *buffer;
    size_t capacity;
};

void mx490_chmp_escl_bridge_init(struct mx490_chmp_escl_bridge *bridge,
                                 const struct mx490_chmp_escl_transport *transport,
                                 unsigned char *buffer, size_t capacity);
void mx490_chmp_escl_bridge_ops(struct mx490_chmp_escl_bridge *bridge,
                                struct mx490_escl_ops *ops);

#endif
