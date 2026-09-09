#include "chmp_session.h"

static int has_text(const char *p, size_t n, const char *needle) {
    size_t i, j;
    for (i = 0; i < n; ++i) {
        for (j = 0; needle[j] && i + j < n && p[i + j] == needle[j]; ++j) {}
        if (!needle[j]) return 1;
    }
    return 0;
}

static int xml_ok(struct mx490_chmp_session *s, size_t (*build)(char *, size_t, unsigned long)) {
    char request[512], response[512]; size_t n, got = 0;
    if (s == 0 || s->xml_exchange == 0 || build == 0) return -1;
    n = build(request, sizeof(request), s->job_id);
    if (n >= sizeof(request) || s->xml_exchange(s->ctx, request, n, response,
                                                sizeof(response) - 1, &got) != 0 || got == 0) return -1;
    return (has_text(response, got, "OK") || has_text(response, got, "Success")) ? 0 : -1;
}

static int exchange_ack(struct mx490_chmp_session *s, const uint8_t *req, size_t n) {
    uint8_t response[64]; size_t got = 0;
    if (s == 0 || s->exchange == 0 || s->exchange(s->ctx, req, n, response, sizeof(response), &got) != 0 || got < 2) return -1;
    return response[0] == 0x06 && response[1] == 0x06 ? 0 : -1;
}

int mx490_chmp_session_start(struct mx490_chmp_session *s, const struct mx490_chmp_scan_params *p) {
    uint8_t req[16 + MX490_CHMP_SCAN_PARAM_SIZE];
    if (s == 0 || p == 0 || s->exchange == 0 || s->active) return -1;
    if (s->xml_exchange != 0) {
        if (xml_ok(s, mx490_chmp_xml_start_job) != 0 ||
            xml_ok(s, mx490_chmp_xml_mode_shift) != 0) return -1;
    }
    mx490_chmp_header(0xdb20, 1, 0, req);
    if (exchange_ack(s, req, sizeof(req) - MX490_CHMP_SCAN_PARAM_SIZE) != 0) return -1;
    mx490_chmp_scan_params(p, req + 16);
    mx490_chmp_header(0xd820, 1, MX490_CHMP_SCAN_PARAM_SIZE, req);
    if (exchange_ack(s, req, sizeof(req)) != 0) return -1;
    mx490_chmp_header(0xd920, 1, 0, req);
    if (exchange_ack(s, req, 16) != 0) return -1;
    s->active = 1; s->ready = 0; return 0;
}

int mx490_chmp_session_poll(struct mx490_chmp_session *s, uint8_t *state) {
    uint8_t req[16], response[32]; size_t got = 0;
    if (s == 0 || state == 0 || !s->active || s->exchange == 0) return -1;
    mx490_chmp_header(0xda20, 1, 8, req);
    if (s->exchange(s->ctx, req, sizeof(req), response, sizeof(response), &got) != 0 || got < 9 || response[0] != 0x06 || response[1] != 0x06) return -1;
    *state = response[8]; s->ready = (*state == 3); return 0;
}

int mx490_chmp_session_read(struct mx490_chmp_session *s, uint8_t *data, size_t cap, size_t *len, int *last) {
    uint8_t req[16]; size_t got = 0; uint32_t n;
    if (s == 0 || data == 0 || len == 0 || last == 0 || !s->active || !s->ready || s->exchange == 0 || cap < 16) return -1;
    mx490_chmp_header(0xd420, 1, 0, req); req[12] = 0; req[13] = 8;
    /* The caller owns the response buffer. The transport writes the 16-byte
     * response header followed by JPEG bytes into it. */
    if (s->exchange(s->ctx, req, sizeof(req), data, cap, &got) != 0 || mx490_chmp_read_image_length(data, got, &n, last) != 0 || n > cap - 16u || got < 16u + n) return -1;
    for (uint32_t i = 0; i < n; ++i) data[i] = data[16 + i];
    *len = n; return 0;
}

int mx490_chmp_session_cancel(struct mx490_chmp_session *s) {
    uint8_t req[16];
    if (s == 0 || !s->active || s->exchange == 0) return -1;
    mx490_chmp_header(0xef20, 1, 0, req);
    if (exchange_ack(s, req, sizeof(req)) != 0) return -1;
    if (s->xml_exchange != 0 && xml_ok(s, mx490_chmp_xml_end_job) != 0) return -1;
    s->active = 0; s->ready = 0; return 0;
}
