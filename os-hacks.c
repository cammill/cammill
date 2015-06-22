#include "os-hacks.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

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


size_t get_executable_path (char* buffer, size_t len) {
	char *path_end;

	getcwd(buffer, sizeof(buffer));

#ifndef __MINGW32__
	if (readlink("/proc/self/exe", buffer, len) <= 0) {
		return -1;
	}
#else
//	HMODULE hModule = GetModuleHandle(NULL);
//	if (hModule != NULL) {
		// When passing NULL to GetModuleHandle, it returns handle of exe itself
//		GetModuleFileName(hModule, buffer, (sizeof(buffer)));
//	}
	buffer[0] = 0;
#endif

	/* Find the last occurence of a forward slash, the path separator.  */
#ifndef __MINGW32__
	path_end = strrchr (buffer, '/');
#else
	path_end = strrchr (buffer, '\\');
#endif        
	if (path_end == NULL) {
		return -1;
	}

	/* Advance to the character past the last slash.  */
	++path_end;

	/* Obtain the directory containing the program by truncating the
	path after the last slash.  */

	*path_end = '\0';
	/* The length of the path is the number of characters up through the
	last slash.  */
	return (size_t)(path_end - buffer);
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
