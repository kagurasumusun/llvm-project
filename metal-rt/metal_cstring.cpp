// metal_cstring.cpp - Metal String Functions

extern "C" {

int ___metal_strlen(const char* s) { int n = 0; while (*s++) n++; return n; }
int ___metal_strcmp(const char* a, const char* b) { while (*a && *a == *b) { a++; b++; } return *(unsigned char*)a - *(unsigned char*)b; }
int ___metal_strncmp(const char* a, const char* b, int n) { while (n && *a && *a == *b) { a++; b++; n--; } return n ? *(unsigned char*)a - *(unsigned char*)b : 0; }
char* ___metal_strcpy(char* dst, const char* src) { char* r = dst; while ((*dst++ = *src++)); return r; }
char* ___metal_strncpy(char* dst, const char* src, int n) { char* r = dst; while (n && (*dst++ = *src++)) n--; while (n--) *dst++ = 0; return r; }
char* ___metal_strcat(char* dst, const char* src) { char* r = dst; while (*dst) dst++; while ((*dst++ = *src++)); return r; }
char* ___metal_strncat(char* dst, const char* src, int n) { char* r = dst; while (*dst) dst++; while (n-- && (*dst++ = *src++)); *dst = 0; return r; }
const char* ___metal_strchr(const char* s, int c) { while (*s && *s != c) s++; return *s == c ? s : (const char*)0; }
const char* ___metal_strrchr(const char* s, int c) { const char* r = (const char*)0; while (*s) { if (*s == c) r = s; s++; } return r; }
void* ___metal_memcpy(void* dst, const void* src, int n) { char* d = (char*)dst; const char* s = (const char*)src; while (n--) *d++ = *s++; return dst; }
void* ___metal_memmove(void* dst, const void* src, int n) {
    char* d = (char*)dst; const char* s = (const char*)src;
    if (d < s) { while (n--) *d++ = *s++; }
    else { d += n; s += n; while (n--) *--d = *--s; }
    return dst;
}
void* ___metal_memset(void* dst, int c, int n) { char* d = (char*)dst; while (n--) *d++ = (char)c; return dst; }
int ___metal_memcmp(const void* a, const void* b, int n) {
    const unsigned char *p = (const unsigned char*)a, *q = (const unsigned char*)b;
    while (n--) { if (*p != *q) return *p - *q; p++; q++; }
    return 0;
}

} // extern C
