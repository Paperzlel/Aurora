#ifndef _UNISTD_H
#define _UNISTD_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef signed int *intptr_t;

int execv(const char *path, char *const argv[]);
int execve(const char *path, char *const argv[], char *const envp[]);
int execvp(const char *path, char *const argv[]);

pid_t getpid(void);

pid_t fork(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _UNISTD_H