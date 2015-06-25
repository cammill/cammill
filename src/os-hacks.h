#ifndef OS_HACKS_H
#define OS_HACKS_H

#include <stdio.h>
#include <stddef.h>

int get_home_dir(char* buffer);
size_t get_executable_path (char* buffer, size_t len);

#ifdef __MINGW32__
ssize_t getline(char **linep, size_t *n, FILE *fp);
#define DIR_SEP "\\"
#else
#define DIR_SEP "/"
#endif

#ifdef __MINGW32__
#ifndef WIFEXITED
#define WIFEXITED(S) 1
#endif

#ifndef WEXITSTATUS
#define WEXITSTATUS(S) (S)
#endif
#endif

#define WIFSIGNALED(x) 0

#define WTERMSIG(x) SIGTERM

#endif
