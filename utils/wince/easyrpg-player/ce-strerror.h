#ifndef CE_STRERROR_H
#define CE_STRERROR_H
#ifdef __cplusplus
extern "C" {
#endif
char *strerror(int errnum);
int remove(const char *path);
#ifdef __cplusplus
}
#endif
#endif
