#include "os-hacks.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <libgen.h>

#ifdef __MINGW32__
#include <windows.h>
#include <shfolder.h>
#else
#include <pwd.h>
#endif

int get_home_dir(char* buffer) {
#ifndef __MINGW32__
        struct passwd *pw = getpwuid(getuid());
        strcpy(buffer, pw->pw_dir);
        return 0;
#else
        //char buf[PATH_MAX];
        //SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, buf);
        //printf("get_home_dir() : '%s'\n", buf);
        return (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, buffer)));
#endif
}

size_t get_executable_path (char *argv, char* buffer, size_t len) {
#ifdef __MINGW32__
	GetModuleFileName(NULL, buffer, len);
#else
	char *res = realpath(argv, NULL);
	if (res == NULL) {
			res = realpath("/proc/self/exe", NULL);
			if (res == NULL) {
				fprintf(stderr, "realpath() failed\n");
				return -1;
			} else {
				snprintf(buffer, len, "%s", res);
			}
	} else {
			snprintf(buffer, len, "%s", res);
	}
	free(res);
#endif
	fprintf(stderr, "%s - %s\n", argv, buffer);
	dirname(buffer);
	strcat(buffer, "/");
	fprintf(stderr, "%s - %s\n", argv, buffer);
	return (size_t)strlen(buffer);
}

ssize_t getdelim(char **linep, size_t *n, int delim, FILE *fp){
        int ch;
        size_t i = 0;
        if(!linep || !n || !fp){
                errno = EINVAL;
                return -1;
        }
        if(*linep == NULL){
                if(NULL==(*linep = malloc(*n=128))){
                        *n = 0;
                        errno = ENOMEM;
                        return -1;
                }
        }
        while((ch = fgetc(fp)) != EOF){
                if(i + 1 >= *n){
                        char *temp = realloc(*linep, *n + 128);
                        if(!temp){
                                errno = ENOMEM;
                                return -1;
                        }
                        *n += 128;
                        *linep = temp;
                }
                (*linep)[i++] = ch;
                if(ch == delim)
                        break;
        }
        (*linep)[i] = '\0';
        return !i && ch == EOF ? -1 : i;
}

ssize_t getline(char **linep, size_t *n, FILE *fp) {
        return getdelim(linep, n, '\n', fp);
}
