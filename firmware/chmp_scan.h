#ifndef MX490_CHMP_SCAN_H
#define MX490_CHMP_SCAN_H

#include <stddef.h>
#include <stdint.h>

#define MX490_CHMP_HEADER_SIZE 16u
#define MX490_CHMP_SCAN_PARAM_SIZE 56u

struct mx490_chmp_scan_params {
    uint16_t x_dpi;
    uint16_t y_dpi;
    uint32_t width;
    uint32_t height;
    uint8_t grayscale;
};

/* Build the 16-byte big-endian CHMP command header. */
void mx490_chmp_header(uint16_t code, uint16_t flags, uint16_t param_len,
                       uint8_t out[MX490_CHMP_HEADER_SIZE]);

/* Build the 56-byte d820 ScanParam3 payload. */
int mx490_chmp_scan_params(const struct mx490_chmp_scan_params *params,
                           uint8_t out[MX490_CHMP_SCAN_PARAM_SIZE]);

/* Validate the 16-byte d420 response prefix and return its data length. */
int mx490_chmp_read_image_length(const uint8_t *response, size_t response_len,
                                 uint32_t *data_len, int *last);

#endif
