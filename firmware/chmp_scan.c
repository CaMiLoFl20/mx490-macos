#include "chmp_scan.h"

static void be16(uint8_t *p, uint16_t v) { p[0] = (uint8_t)(v >> 8); p[1] = (uint8_t)v; }
static void be32(uint8_t *p, uint32_t v) { p[0] = (uint8_t)(v >> 24); p[1] = (uint8_t)(v >> 16); p[2] = (uint8_t)(v >> 8); p[3] = (uint8_t)v; }

void mx490_chmp_header(uint16_t code, uint16_t flags, uint16_t param_len,
                       uint8_t out[MX490_CHMP_HEADER_SIZE]) {
    size_t i;
    be16(out, code); be16(out + 2, flags);
    for (i = 4; i < 14; ++i) out[i] = 0;
    be16(out + 14, param_len);
}

int mx490_chmp_scan_params(const struct mx490_chmp_scan_params *p,
                           uint8_t out[MX490_CHMP_SCAN_PARAM_SIZE]) {
    uint8_t sum = 0; size_t i;
    if (p == 0 || out == 0 || p->x_dpi == 0 || p->y_dpi == 0 ||
        p->width == 0 || p->height == 0) return -1;
    for (i = 0; i < MX490_CHMP_SCAN_PARAM_SIZE; ++i) out[i] = 0;
    out[0] = 1; out[1] = 1; out[2] = 1;
    be16(out + 8, (uint16_t)(0x8000u | p->x_dpi));
    be16(out + 10, (uint16_t)(0x8000u | p->y_dpi));
    be32(out + 0x14, p->width); be32(out + 0x18, p->height);
    out[0x1c] = p->grayscale ? 0x04 : 0x08;
    out[0x1d] = p->grayscale ? 8 : 24;
    out[0x1f] = 1; out[0x20] = 0xff; out[0x21] = 0x82;
    out[0x23] = 2; out[0x24] = 1; out[0x30] = 1;
    for (i = 0; i < MX490_CHMP_SCAN_PARAM_SIZE - 1; ++i) sum = (uint8_t)(sum + out[i]);
    out[MX490_CHMP_SCAN_PARAM_SIZE - 1] = (uint8_t)(0u - sum);
    return 0;
}

int mx490_chmp_read_image_length(const uint8_t *r, size_t n, uint32_t *len, int *last) {
    if (r == 0 || len == 0 || last == 0 || n < 16 || r[0] != 0x06 || r[1] != 0x06) return -1;
    *last = (r[8] & 0x20u) != 0;
    *len = ((uint32_t)r[12] << 24) | ((uint32_t)r[13] << 16) | ((uint32_t)r[14] << 8) | r[15];
    return 0;
}
