/* Minimal libc replacements used by the offline ARM payload-size probe.
 * The printer port should replace these with native routines if available. */
#include <stdarg.h>
#include <stddef.h>

size_t strlen(const char *s) { const char *p = s; while (*p) ++p; return (size_t)(p - s); }
int strcmp(const char *a, const char *b) { while (*a && *a == *b) { ++a; ++b; } return (unsigned char)*a - (unsigned char)*b; }
int strncmp(const char *a, const char *b, size_t n) { while (n && *a && *a == *b) { ++a; ++b; --n; } return n ? (unsigned char)*a - (unsigned char)*b : 0; }
size_t strcspn(const char *s, const char *reject) { size_t n = 0; for (; s[n]; ++n) for (const char *r = reject; *r; ++r) if (s[n] == *r) return n; return n; }

static int emit(char *out, size_t cap, size_t *pos, char c) { if (*pos + 1 < cap) out[*pos] = c; ++*pos; return 0; }
static int emit_text(char *out, size_t cap, size_t *pos, const char *s) { while (*s) emit(out, cap, pos, *s++); return 0; }
static int emit_decimal12(char *out, size_t cap, size_t *pos, unsigned long value) {
    static const unsigned long place[] = {1000000000UL,100000000UL,10000000UL,1000000UL,100000UL,10000UL,1000UL,100UL,10UL,1UL};
    size_t i; unsigned long digit;
    for (i = 0; i < sizeof(place) / sizeof(place[0]); ++i) {
        digit = 0;
        while (value >= place[i]) { value -= place[i]; ++digit; }
        emit(out, cap, pos, (char)('0' + digit));
    }
    emit(out, cap, pos, '0'); emit(out, cap, pos, '0');
    return 0;
}

int snprintf(char *out, size_t cap, const char *fmt, ...) {
    va_list ap; size_t pos = 0; va_start(ap, fmt);
    if (fmt[0] == '0' && fmt[1] == '0') {
        emit_text(out, cap, &pos, "00000000-0000-4000-8000-");
        emit_decimal12(out, cap, &pos, va_arg(ap, unsigned long));
    } else if (fmt[0] == '/' && fmt[1] == 'e') {
        emit_text(out, cap, &pos, "/eSCL/ScanJobs/");
        emit_text(out, cap, &pos, va_arg(ap, const char *));
    }
    va_end(ap);
    if (cap) out[pos < cap - 1 ? pos : cap - 1] = '\0';
    return (int)pos;
}
