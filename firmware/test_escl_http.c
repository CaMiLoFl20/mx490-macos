#include "escl_http.h"
#include <assert.h>
#include <string.h>
int main(void) {
    static const unsigned char body[] = "abc"; unsigned char out[256]; size_t n;
    struct mx490_escl_response r = {201, "text/plain", "/eSCL/ScanJobs/x", body, 3};
    assert(mx490_escl_http_response(&r, out, sizeof(out), &n) == 0);
    assert(n > 0 && strstr((const char *)out, "HTTP/1.1 201 Created") != 0);
    assert(strstr((const char *)out, "Content-Length: 3") != 0);
    assert(out[n - 3] == 'a' && out[n - 1] == 'c'); return 0;
}
