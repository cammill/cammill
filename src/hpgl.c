/*

 Copyright by Oliver Dippel <oliver@multixmedia.org>

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

 This program is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation; either version 2 of the License, or (at
 your option) any later version.

 On Debian GNU/Linux systems, the complete text of the GNU General
 Public License can be found in `/usr/share/common-licenses/GPL'.

*/

#include <stdlib.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#ifdef __linux__
#include <linux/limits.h> // for PATH_MAX
#elif _WIN32
#include <windows.h>
#else
#include <limits.h>
#endif
#include <setup.h>
#include <font.h>
#include <dxf.h>
#include <hpgl.h>
#ifdef __APPLE__
#include <malloc/malloc.h>
#endif
#include <locale.h>

#include "os-hacks.h" // for getline()

#ifdef __gnu_linux__
#define PYTHON_SPLINE 1
#endif


void hpgl_read (char *file) {
	FILE *fp;
	char *line = NULL;
	char line2[100000];
	char layer_str[100];
	char cmd_str[10];
	size_t len = 0;
	ssize_t read;
	int n = 0;
	int p = 0;
	double x = 0;
	double y = 0;
	double last_x = 0;
	double last_y = 0;
	int pn = 0;
	char pos_str[1024];
	setlocale(LC_NUMERIC, "C");
	fp = fopen(file, "r");
	if (fp == NULL) {
		return;
	}
	int num = 0;
	for (num = 0; num < line_last; num++) {
		LayerNames[num][0] = 0;
	}
	while ((read = getline(&line, &len, fp)) != -1) {
		trimline(line2, 100000 - 1, line);
//		printf("##hpgl: %s\n", line2);
		cmd_str[0] = line2[0];
		cmd_str[1] = line2[1];
		cmd_str[2] = 0;
//		printf("hpgl: %s\n", cmd_str);
		int pen = 0;
		int pen_n = 0;
		if (cmd_str[0] == 'P') {
			if (cmd_str[1] == 'U') {
//				printf("hpgl: Pen UP\n");
				pen = 0;
			} else if (cmd_str[1] == 'D') {
//				printf("hpgl: Pen Down\n");
				pen = 1;
			} else if (cmd_str[1] == 'R') {
//				printf("hpgl: relative moves\n");
			} else if (cmd_str[1] == 'A') {
//				printf("hpgl: absolute moves\n");
			}
		} else if (cmd_str[0] == 'S') {
			if (cmd_str[1] == 'P') {
				pen_n = atoi(line2 + 2);
//				printf("hpgl: select Pen (%i)\n", pen_n);
			}
		} else if (cmd_str[0] == 'I') {
		}
		p = 0;
		pn = 0;
		for (n = 2; n < strlen(line2); n++) {
			if (line2[n] == ' ') {
			} else if (line2[n] == ',' || line2[n] == ';') {
//				printf("POS%i: %s\n", p, pos_str);
				if (p == 0) {
					last_x = x;
					x = atof(pos_str) / 40.0;
				} else {
					last_y = y;
					y = atof(pos_str) / 40.0;
					if (pen == 1) {
//						printf("LINE %f,%f -> %f,%f\n", last_x, last_y, x, y);
						sprintf(layer_str, "pen%i", pen_n);
						add_line(TYPE_LINE, layer_str, last_x, last_y, x, y, 0.0, 0.0, 0.0);
					}
				}
				pn = 0;
				p = 1 - p;
			} else {
				pos_str[pn++] = line2[n];
				pos_str[pn] = 0;
			}
		}
	}
	fclose(fp);
}


