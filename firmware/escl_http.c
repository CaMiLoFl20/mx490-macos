#include "escl_http.h"

static size_t put(unsigned char *out, size_t cap, size_t p, const char *s)
{
    while (*s) { if (p < cap) out[p] = (unsigned char)*s; ++p; ++s; }
    return p;
}
static size_t put_uint(unsigned char *out, size_t cap, size_t p, size_t v)
{
    char d[20]; size_t n = 0, i;
    do { d[n++] = (char)('0' + (v % 10)); v /= 10; } while (v != 0);
    for (i = n; i != 0; --i) { if (p < cap) out[p] = (unsigned char)d[i - 1]; ++p; }
    return p;
}
int mx490_escl_http_response(const struct mx490_escl_response *r,
                             unsigned char *out, size_t cap, size_t *written)
{
    const char *reason; size_t p = 0, i;
    if (r == 0 || out == 0 || written == 0) return -1;
    if (r->status == 200) reason = "OK";
    else if (r->status == 201) reason = "Created";
    else if (r->status == 404) reason = "Not Found";
    else if (r->status == 500) reason = "Internal Server Error";
    else return -1;
    p = put(out, cap, p, "HTTP/1.1 "); p = put_uint(out, cap, p, (size_t)r->status);
    p = put(out, cap, p, " "); p = put(out, cap, p, reason);
    p = put(out, cap, p, "\r\nContent-Type: ");
    p = put(out, cap, p, r->content_type ? r->content_type : "text/plain");
    p = put(out, cap, p, "\r\nContent-Length: "); p = put_uint(out, cap, p, r->body_len);
    if (r->location) { p = put(out, cap, p, "\r\nLocation: "); p = put(out, cap, p, r->location); }
    p = put(out, cap, p, "\r\nConnection: close\r\n\r\n");
    if (p + r->body_len > cap) return -1;
    for (i = 0; i < r->body_len; ++i) out[p + i] = r->body[i];
    p += r->body_len; *written = p; return 0;
}
