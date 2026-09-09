#include "chmp_scan.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>

int main(void) {
    uint8_t h[16], p[56], ping[1], r[16] = {0x06,0x06,0,0,0,0,0,0,0x20,0,0,0,0,0,0x01,0x02}; char xml[512];
    struct mx490_chmp_scan_params s = {300, 300, 2550, 3300, 0};
    uint32_t len; int last; size_t i; unsigned sum = 0;
    assert(mx490_chmp_ping(ping) == 1 && ping[0] == 0);
    assert(mx490_chmp_xml_start_job(xml, sizeof(xml), 1) > 0 && strstr(xml, "StartJob") != 0 && strstr(xml, "00000001") != 0);
    assert(mx490_chmp_xml_mode_shift(xml, sizeof(xml), 1) > 0 && strstr(xml, "ModeShift") != 0);
    assert(mx490_chmp_xml_end_job(xml, sizeof(xml), 1) > 0 && strstr(xml, "EndJob") != 0);
    mx490_chmp_header(0xd820, 1, 72, h);
    assert(h[0] == 0xd8 && h[1] == 0x20 && h[2] == 0 && h[3] == 1 && h[14] == 0 && h[15] == 72);
    assert(mx490_chmp_scan_params(&s, p) == 0);
    for (i = 0; i < sizeof(p); ++i) sum += p[i];
    assert((sum & 0xffu) == 0 && p[0x1c] == 8 && p[0x1d] == 24);
    assert(mx490_chmp_read_image_length(r, sizeof(r), &len, &last) == 0 && len == 0x102 && last == 1);
    return 0;
}
