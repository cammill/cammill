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

#endif
