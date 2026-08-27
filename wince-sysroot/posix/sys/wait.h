/*
 * sys/wait.h for Windows CE - waitpid over the posix runtime's child
 * table (see wince-sysroot/posix/process.c).
 */
#ifndef _WINCE_SYS_WAIT_H
#define _WINCE_SYS_WAIT_H

#include <stddef.h>
#include <sys/types.h>   /* pid_t (mingwrt: _pid_t) */

/* waitpid status accessors (msvcrt-compatible encoding) */
#define WEXITSTATUS(s)  (((s) >> 8) & 0xFF)
#define WTERMSIG(s)     ((s) & 0x7F)
#define WIFEXITED(s)    (WTERMSIG(s) == 0)
#define WIFSIGNALED(s)  (WTERMSIG(s) != 0 && WEXITSTATUS(s) == 0)
#define WIFSTOPPED(s)   (0)
#define WIFCONTINUED(s) (0)
#define WNOHANG         1
#define WUNTRACED       2

#ifdef __cplusplus
extern "C" {
#endif

pid_t waitpid(pid_t pid, int *status, int options);

#ifdef __cplusplus
}
#endif

#endif /* _WINCE_SYS_WAIT_H */
