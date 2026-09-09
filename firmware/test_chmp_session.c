#include "chmp_session.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>

struct fake { int step; int xml_step; };
static int exchange(void *ctx, const uint8_t *req, size_t n, uint8_t *out, size_t cap, size_t *got) {
    struct fake *f = (struct fake *)ctx; (void)cap;
    assert(n >= 16); out[0] = 0x06; out[1] = 0x06; *got = 2;
    if (req[0] == 0xda && req[1] == 0x20) { out[8] = 3; *got = 9; }
    if (req[0] == 0xd4 && req[1] == 0x20) { out[8] = 0x20; out[12] = 0; out[13] = 0; out[14] = 0; out[15] = 4; out[16] = 0xff; out[17] = 0xd8; out[18] = 0xff; out[19] = 0xd9; *got = 20; }
    ++f->step; return 0;
}
static int xml_exchange(void *ctx, const char *req, size_t n, char *out, size_t cap, size_t *got) {
    struct fake *f = (struct fake *)ctx;
    assert(cap > 0 && n > 0);
    if (f->xml_step == 0) assert(req[0] == '<' && strstr(req, "StartJob") != 0);
    else if (f->xml_step == 1) assert(strstr(req, "ModeShift") != 0);
    else assert(strstr(req, "EndJob") != 0);
    assert(f->xml_step < 3);
    out[0] = '<'; out[1] = 'O'; out[2] = 'K'; out[3] = '>';
    *got = 4; ++f->xml_step; return 0;
}
int main(void) {
    struct fake f = {0, 0}; struct mx490_chmp_session s = {exchange, &f, 0, 0, xml_exchange, 1};
    struct mx490_chmp_scan_params p = {300,300,2550,3300,0}; uint8_t state, image[32]; size_t n; int last;
    assert(mx490_chmp_session_start(&s, &p) == 0 && s.active);
    assert(mx490_chmp_session_poll(&s, &state) == 0 && state == 3);
    assert(mx490_chmp_session_read(&s, image, sizeof(image), &n, &last) == 0 && n == 4 && last);
    assert(image[0] == 0xff && mx490_chmp_session_cancel(&s) == 0 && !s.active);
    assert(f.xml_step == 3);
    return 0;
}
