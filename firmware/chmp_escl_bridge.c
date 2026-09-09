#include "chmp_escl_bridge.h"

static int start_job(const char *xml, size_t xml_len, unsigned long *id, void *ctx)
{
    struct mx490_chmp_escl_bridge *b = (struct mx490_chmp_escl_bridge *)ctx;
    struct mx490_chmp_scan_params p;
    (void)xml;
    (void)xml_len;
    if (b == 0 || id == 0 || b->transport.start == 0) return -1;
    p.x_dpi = 300; p.y_dpi = 300;
    p.width = 2550; p.height = 3500; p.grayscale = 0;
    return b->transport.start(&p, id, b->transport.ctx);
}

static int next_document(unsigned long id, const unsigned char **data,
                         size_t *len, int *last, void *ctx)
{
    struct mx490_chmp_escl_bridge *b = (struct mx490_chmp_escl_bridge *)ctx;
    if (b == 0 || data == 0 || len == 0 || last == 0 ||
        b->transport.next == 0 || b->buffer == 0 || b->capacity == 0) return -1;
    if (b->transport.next(id, b->buffer, b->capacity, len, last,
                          b->transport.ctx) != 0) return -1;
    *data = b->buffer;
    return 0;
}

static int cancel_job(unsigned long id, void *ctx)
{
    struct mx490_chmp_escl_bridge *b = (struct mx490_chmp_escl_bridge *)ctx;
    if (b == 0 || b->transport.cancel == 0) return -1;
    return b->transport.cancel(id, b->transport.ctx);
}

void mx490_chmp_escl_bridge_init(struct mx490_chmp_escl_bridge *b,
                                 const struct mx490_chmp_escl_transport *t,
                                 unsigned char *buffer, size_t capacity)
{
    size_t i;
    if (b == 0) return;
    for (i = 0; i < sizeof(*b); ++i) ((unsigned char *)b)[i] = 0;
    if (t != 0) b->transport = *t;
    b->buffer = buffer;
    b->capacity = capacity;
}

void mx490_chmp_escl_bridge_ops(struct mx490_chmp_escl_bridge *b,
                                struct mx490_escl_ops *ops)
{
    if (ops == 0) return;
    ops->submit_job = start_job;
    ops->next_document = next_document;
    ops->cancel_job = cancel_job;
    ops->ctx = b;
}
