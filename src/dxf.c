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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <dxf.h>
#ifdef __linux__
#include <linux/limits.h> // for PATH_MAX
#elif _WIN32
#include <windows.h>
#else
#include <limits.h>
#endif
#include <setup.h>
#include <font.h>
#ifdef __APPLE__
#include <malloc/malloc.h>
#endif
#include <locale.h>

#include "os-hacks.h" // for getline()

int block = 0;
double block_x = 0.0;
double block_y = 0.0;
char block_name[1024];

char LayerNames[MAX_OBJECTS][256];

double vector_angle (double x1, double y1, double x2, double y2);
double get_len (double x1, double y1, double x2, double y2);

int mtext_n = 0;
char dxf_options[256][256];
_OBJECT *myOBJECTS = NULL;
_LINE *myLINES = NULL;

char dxf_typename[TYPE_LAST][16];

int line_n = 1;
int line_last = 0;

void add_line (int type, char *layer, double x1, double y1, double x2, double y2, double opt, double cx, double cy) {
//	printf("## ADD_LINE (%i %i): %f,%f -> %f,%f (%s / %f)\n", line_n, line_last, x1, y1, x2, y2, layer, opt);
	if (x1 > 10000.0 || y1 > 10000.0 || x2 > 10000.0 || y2 > 10000.0) {
		fprintf(stderr, "dxf: ###### LINE TOO BIG; %f %f -> %f %f ######\n", x1, y1, x2, y2);
		return;
	}
	if (line_n < MAX_LINES) {
		// add layer name if not exist
		int num = 0;
		for (num = 0; num < line_last; num++) {
			if (strcmp(LayerNames[num], layer) == 0) {
				break;
			} else if (LayerNames[num][0] == 0) {
				strncpy(LayerNames[num], layer, sizeof(LayerNames[num]));
				break;
			}
		}
		// check if line allready exist
		for (num = 0; num < line_last; num++) {
			if (myLINES[num].blockdata != 1 && myLINES[num].cx == cx && myLINES[num].cy == cy && myLINES[num].opt == opt) {
				if (myLINES[num].x1 == x1 && myLINES[num].y1 == y1 && myLINES[num].x2 == x2 && myLINES[num].y2 == y2) {
					printf("## DOUBLE_LINE (%i %i): %f,%f -> %f,%f (%s / %f)\n", line_n, line_last, x1, y1, x2, y2, layer, opt);
					if (PARAMETER[P_M_DELETE_DOUBLE].vint == 1 || strcmp(myLINES[num].layer, layer) == 0) {
						fprintf(stderr, "## DROPED\n");
						return;
					} else {
						fprintf(stderr, "## OTHER LAYER, WARNING ONLY\n");
					}
				} else if (myLINES[num].x1 == x2 && myLINES[num].y1 == y2 && myLINES[num].x2 == x1 && myLINES[num].y2 == y1) {
					printf("## DOUBLE_LINE (%i %i): %f,%f -> %f,%f (%s / %f)\n", line_n, line_last, x1, y1, x2, y2, layer, opt);
					if (PARAMETER[P_M_DELETE_DOUBLE].vint == 1 || strcmp(myLINES[num].layer, layer) == 0) {
						fprintf(stderr, "## DROPED\n");
						return;
					} else {
						fprintf(stderr, "## OTHER LAYER, WARNING ONLY\n");
					}
				}
			}
		}

		if (myLINES == NULL) {
			myLINES = (_LINE *)malloc(sizeof(_LINE) * 5);
			if (myLINES == NULL) {
				fprintf(stderr, "Not enough memory\n");
				exit(1);
			}
		} else {
			_LINE *myLINES_new = NULL;
			myLINES_new = (_LINE *)realloc((void *)myLINES, sizeof(_LINE) * (line_last + 5));
			if (myLINES_new == NULL) {
				fprintf(stderr, "Not enough memory\n");
				exit(1);
			} else {
				myLINES = myLINES_new;
			}
				}
		if (strcmp(layer, "holding-tabs") == 0) {
			myLINES[line_n].istab = 1;
			myLINES[line_n].used = 0;
		} else if (block == 1) {
			myLINES[line_n].istab = 0;
			myLINES[line_n].used = 0;
		} else {
			myLINES[line_n].istab = 0;
			myLINES[line_n].used = 1;
		}
		myLINES[line_n].type = type;
		strncpy(myLINES[line_n].layer, layer, sizeof(myLINES[line_n].layer));
		myLINES[line_n].x1 = x1;
		myLINES[line_n].y1 = y1;
		myLINES[line_n].x2 = x2;
		myLINES[line_n].y2 = y2;
		myLINES[line_n].cx = cx;
		myLINES[line_n].cy = cy;
		myLINES[line_n].opt = opt;
		myLINES[line_n].marked = 0;
		myLINES[line_n].in_object = -1;
		strncpy(myLINES[line_n].block, block_name, sizeof(myLINES[line_n].block));
		myLINES[line_n].blockdata = block;
		line_n++;

		myLINES[line_n].used = 0;
		myLINES[line_n + 1].used = 0;
		myLINES[line_n].type = -1;
		myLINES[line_n + 1].type = -1;

		line_last = line_n;
	} else {
		fprintf(stderr, "dxf: ### TOO MANY LINES ##\n");
//		exit(1);
	}
}

size_t trimline (char *out, size_t len, const char *str) {
	if(len == 0) {
		return 0;
	}
	const char *end;
	size_t out_size;
	while (*str == ' ' || *str == '\r' || *str == '\n') {
		str++;
	}
	if (*str == 0) {
		*out = 0;
		return 1;
	}
	end = str + strlen(str) - 1;
	while(end > str && (*end == ' ' || *end == '\r' || *end == '\n')) {
		end--;
	}
	end++;
	out_size = (end - str) < len-1 ? (end - str) : len-1;
	memcpy(out, str, out_size);
	out[out_size] = 0;
	return out_size;
}

void clear_dxfoptions (void) {
	int num = 0;
	for (num = 0; num < 256; num++) {
		dxf_options[num][0] = 0;
	}
}

void dxf_read (char *file) {
	FILE *fp;
	char *line = NULL;
	char line2[1024];
	size_t len = 0;
	ssize_t read;

	setlocale(LC_NUMERIC, "C");

	line_last = 0;
	line_n = 1;
	if (myLINES != NULL) {
		free(myLINES);
		myLINES = NULL;
	}
	// set default to mm
	PARAMETER[P_O_UNIT].vint = 1;
	strcpy(dxf_typename[TYPE_NONE], "None");
	strcpy(dxf_typename[TYPE_LINE], "Line");
	strcpy(dxf_typename[TYPE_ARC], "Arc");
	strcpy(dxf_typename[TYPE_CIRCLE], "Circle");
	strcpy(dxf_typename[TYPE_MTEXT], "Text");
	strcpy(dxf_typename[TYPE_POINT], "Point");
	strcpy(dxf_typename[TYPE_POLYLINE], "Polyline");
	strcpy(dxf_typename[TYPE_VERTEX], "Vertex");

	fp = fopen(file, "r");
	if (fp == NULL) {
		return;
	}

	int num = 0;
	for (num = 0; num < line_last; num++) {
		LayerNames[num][0] = 0;
	}

	clear_dxfoptions();

	char last_0[1024];
	strcpy(last_0, "");

	int lwpl_flag = 0;
	int pl_flag = 0;
	int pl_closed = 0;
	double pl_first_x = 0.0;
	double pl_first_y = 0.0;
	double pl_last_x = 0.0;
	double pl_last_y = 0.0;

	while ((read = getline(&line, &len, fp)) != -1) {
		trimline(line2, 1024, line);
		int dxfoption = atoi(line2);
		if ((read = getline(&line, &len, fp)) != -1) {
			trimline(line2, 1024, line);
			if (dxfoption == 0) {
				if (last_0[0] != 0) {
					if (strcmp(last_0, "BLOCK") == 0) {
						block = 1;
						block_x = atof(dxf_options[OPTION_POINT_X]);
						block_y = atof(dxf_options[OPTION_POINT_Y]);
						strncpy(block_name, dxf_options[OPTION_BLOCKNAME], sizeof(block_name));
					} else if (strcmp(last_0, "ENDBLK") == 0) {
						block = 0;
						block_name[0] = 0;
					} else if (strcmp(last_0, "INSERT") == 0) {
						block = 0;
						block_name[0] = 0;
						block_x = atof(dxf_options[OPTION_POINT_X]);
						block_y = atof(dxf_options[OPTION_POINT_Y]);
						float scale_x = atof(dxf_options[41]);
						float scale_y = atof(dxf_options[42]);
						if (scale_x == 0.0) {
							scale_x = 1.0;
						}
						if (scale_y == 0.0) {
							scale_y = 1.0;
						}
						strncpy(block_name, dxf_options[2], sizeof(block_name));
						int num = 0;
						int last = line_last;
						for (num = 0; num < last - 1; num++) {
							if (myLINES[num].blockdata == 1 && myLINES[num].block[0] != 0 && strcmp(myLINES[num].block, block_name) == 0) {
								add_line(myLINES[num].type, dxf_options[OPTION_LAYERNAME], myLINES[num].x1 * scale_x + block_x, myLINES[num].y1 * scale_y + block_y, myLINES[num].x2 * scale_x + block_x, myLINES[num].y2 * scale_y + block_y, myLINES[num].opt, myLINES[num].cx * scale_x + block_x, myLINES[num].cy * scale_y + block_y);
							}
						}
					} else if (strcmp(last_0, "LINE") == 0) {
						double p_x1 = atof(dxf_options[OPTION_LINE_X1]);
						double p_y1 = atof(dxf_options[OPTION_LINE_Y1]);
						double p_x2 = atof(dxf_options[OPTION_LINE_X2]);
						double p_y2 = atof(dxf_options[OPTION_LINE_Y2]);
						add_line(TYPE_LINE, dxf_options[8], p_x1, p_y1, p_x2, p_y2, 0.0, 0.0, 0.0);
					} else if (strcmp(last_0, "VERTEX") == 0) {
						double p_x1 = atof(dxf_options[OPTION_LINE_X1]);
						double p_y1 = atof(dxf_options[OPTION_LINE_Y1]);
						double p_r1 = atof(dxf_options[42]);
						if (pl_flag == 0) {
							pl_first_x = p_x1;
							pl_first_y = p_y1;
						} else {
							if (p_r1 != 0.0) {
								double chord = sqrt(pow(fabs(pl_last_x - p_x1), 2.0) + pow(fabs(pl_last_y - p_y1), 2.0));
								double s = chord / 2.0 * p_r1;
								double radius = (pow(chord / 2.0, 2.0) + pow(s, 2.0)) / (2.0 * s);
								double len = get_len(pl_last_x, pl_last_y, p_x1, p_y1);
								if (radius * 2 < len) {
									radius *= 2.0;
								}
								add_line(TYPE_ARC, dxf_options[8], pl_last_x, pl_last_y, p_x1, p_y1, radius, 0.0, 0.0);
							} else {
								add_line(TYPE_LINE, dxf_options[8], pl_last_x, pl_last_y, p_x1, p_y1, 0.0, 0.0, 0.0);
							}
						}
						pl_last_x = p_x1;
						pl_last_y = p_y1;
						pl_flag = 1;
					} else if (strcmp(last_0, "SEQEND") == 0) {
						if (pl_closed == 1) {
							add_line(TYPE_LINE, dxf_options[8], pl_last_x, pl_last_y, pl_first_x, pl_first_y, 0.0, 0.0, 0.0);
						}
						pl_flag = 0;
						pl_closed = 0;
					} else if (strcmp(last_0, "POLYLINE") == 0) {
						pl_closed = atoi(dxf_options[70]);
						pl_flag = 0;
					} else if (strcmp(last_0, "LWPOLYLINE") == 0) {
						pl_closed = atoi(dxf_options[70]);
						double p_x1 = atof(dxf_options[OPTION_LINE_X1]);
						double p_y1 = atof(dxf_options[OPTION_LINE_Y1]);
						double p_r1 = atof(dxf_options[42]);
						pl_last_x = p_x1;
						pl_last_y = p_y1;
						dxf_options[42][0] = 0;
						if (pl_closed == 1) {
							add_line(TYPE_LINE, dxf_options[8], pl_last_x, pl_last_y, pl_first_x, pl_first_y, p_r1, 0.0, 0.0);
						}
						lwpl_flag = 0;
						pl_closed = 0;

					} else if (strcmp(last_0, "POINT") == 0) {
						double p_x1 = atof(dxf_options[OPTION_POINT_X]);
						double p_y1 = atof(dxf_options[OPTION_POINT_Y]);
						double p_x2 = atof(dxf_options[OPTION_POINT_X]);
						double p_y2 = atof(dxf_options[OPTION_POINT_Y]);
						add_line(TYPE_POINT, dxf_options[8], p_x1, p_y1, p_x2, p_y2, 0.0, 0.0, 0.0);
					} else if (strcmp(last_0, "SPLINE") == 0) {
/*					} else if (strcmp(last_0, "CIRCLE") == 0) {
						double cx = atof(dxf_options[OPTION_ARC_X]);
						double cy = atof(dxf_options[OPTION_ARC_Y]);
						double r = atof(dxf_options[OPTION_ARC_RADIUS]);
						add_line(TYPE_CIRCLE, dxf_options[8], cx - r, cy, cx + r, cy, r, cx, cy);
						add_line(TYPE_CIRCLE, dxf_options[8], cx + r, cy, cx - r, cy, r, cx, cy);
*/
					} else if (strcmp(last_0, "ARC") == 0 || strcmp(last_0, "CIRCLE") == 0) {
						double p_x1 = atof(dxf_options[OPTION_ARC_X]);
						double p_y1 = atof(dxf_options[OPTION_ARC_Y]);
						double p_y2 = atof(dxf_options[OPTION_ARC_RADIUS]);
						double p_a1 = atof(dxf_options[OPTION_ARC_BEGIN]);
						double p_a2 = atof(dxf_options[OPTION_ARC_END]);
						if (strcmp(last_0, "CIRCLE") == 0) {
							p_a1 = 0.0;
							p_a2 = 360.0;
						}
						if (p_a1 > p_a2) {
							p_a2 += 360.0;
						}
						double r = p_y2;
						double angle2 = toRad(p_a1);
						double x2 = r * cos(angle2);
						double y2 = r * sin(angle2);
						double last_x = (p_x1 + x2);
						double last_y = (p_y1 + y2);
						double an = 0;
						double p_rast = (p_a2 - p_a1) / 9.0;
						for (an = p_a1 + p_rast; an <= p_a2 - (p_rast / 2.0); an += p_rast) {
							double angle1 = toRad(an);
							double x1 = r * cos(angle1);
							double y1 = r * sin(angle1);
							if (strcmp(last_0, "CIRCLE") == 0) {
								add_line(TYPE_CIRCLE, dxf_options[8], last_x, last_y, p_x1 + x1, p_y1 + y1, r, p_x1, p_y1);
							} else {
								add_line(TYPE_ARC, dxf_options[8], last_x, last_y, p_x1 + x1, p_y1 + y1, r, p_x1, p_y1);
							}
							last_x = p_x1 + x1;
							last_y = p_y1 + y1;
						}
						double angle3 = toRad(p_a2);
						double x3 = r * cos(angle3);
						double y3 = r * sin(angle3);
						if (strcmp(last_0, "CIRCLE") == 0) {
							add_line(TYPE_CIRCLE, dxf_options[8], last_x, last_y, p_x1 + x3, p_y1 + y3, r, p_x1, p_y1);
						} else {
							add_line(TYPE_ARC, dxf_options[8], last_x, last_y, p_x1 + x3, p_y1 + y3, r, p_x1, p_y1);
						}
					} else if (strcmp(last_0, "ELLIPSE") == 0) {
						double p_x1 = atof(dxf_options[10]);
						double p_y1 = atof(dxf_options[20]);
						double e_x = atof(dxf_options[11]);
						double e_y = atof(dxf_options[21]);
						double p_a1 = 0.0;
						double p_a2 = 360.0;
						double ratio = atof(dxf_options[40]);
						double r = e_x;
						if (e_x < 0) {
							e_x *= -1;
						}
						if (e_y < 0) {
							e_y *= -1;
						}
						if (e_y > e_x) {
							r = e_y * ratio;
							ratio = 1.0 / ratio;
						}
						if (p_a1 > p_a2) {
							p_a2 += 360.0;
						}
						double angle2 = toRad(p_a1);
						double x2 = r * cos(angle2);
						double y2 = r * ratio * sin(angle2);
						double last_x = (p_x1 + x2);
						double last_y = (p_y1 + y2);
						double first_x = last_x;
						double first_y = last_y;
						double an = 0;
						double p_rast = (p_a2 - p_a1) / 18.0;
						for (an = p_a1 + p_rast; an <= p_a2 - (p_rast / 2.0); an += p_rast) {
							double angle1 = toRad(an);
							double x1 = r * cos(angle1);
							double y1 = r * ratio * sin(angle1);
							add_line(TYPE_ELLIPSE, dxf_options[8], last_x, last_y, p_x1 + x1, p_y1 + y1, 0.0, 0.0, 0.0);
							last_x = p_x1 + x1;
							last_y = p_y1 + y1;
						}
						add_line(TYPE_ELLIPSE, dxf_options[8], last_x, last_y, first_x, first_y, 0.0, 0.0, 0.0);
					} else if (strcmp(last_0, "MTEXT") == 0) {
						double p_x1 = atof(dxf_options[OPTION_MTEXT_X]);
						double p_y1 = atof(dxf_options[OPTION_MTEXT_Y]);
						double p_s = atof(dxf_options[OPTION_MTEXT_SIZE]);
						if (output_text_dxf(dxf_options[OPTION_MTEXT_TEXT], dxf_options[8], p_x1, p_y1, 0.0, p_s, PARAMETER[P_M_TEXT_SCALE_WIDTH].vdouble, PARAMETER[P_M_TEXT_SCALE_HEIGHT].vdouble, PARAMETER[P_M_TEXT_FIXED_WIDTH].vint, PARAMETER[P_M_TEXT_FONT].vstr) == 0) {
							mtext_n++;
						}
					} else if (strcmp(last_0, "$MEASUREMENT") == 0) {
						int mesurement = atoi(dxf_options[OPTION_MEASUREMENT]);
						if (mesurement == 1) {
							PARAMETER[P_O_UNIT].vint = 1;
							strcpy(PARAMETER[P_O_UNIT].vstr, "mm");
						} else {
							PARAMETER[P_O_UNIT].vint = 0;
							strcpy(PARAMETER[P_O_UNIT].vstr, "inch");
						}
					} else {
						pl_flag = 0;
						lwpl_flag = 0;
					}
					clear_dxfoptions();
				}
				strncpy(last_0, line2, sizeof(last_0));
			}
//			printf("## %i: %s\n", dxfoption, line2);
			if (dxfoption < 256) {
				strncpy(dxf_options[dxfoption], line2, sizeof(dxf_options[dxfoption]));
				if (strcmp(last_0, "LWPOLYLINE") == 0) {
					if (dxfoption == 10) {
					} else if (dxfoption == 20) {
						double p_x1 = atof(dxf_options[OPTION_LINE_X1]);
						double p_y1 = atof(dxf_options[OPTION_LINE_Y1]);
						double p_r1 = atof(dxf_options[42]);
						if (lwpl_flag > 0) {
							if (dxf_options[42][0] == 0) {
								add_line(TYPE_LINE, dxf_options[8], pl_last_x, pl_last_y, p_x1, p_y1, 0.0, 0.0, 0.0);
							} else {
								double chord = sqrt(pow(fabs(pl_last_x - p_x1), 2.0) + pow(fabs(pl_last_y - p_y1), 2.0));
								double s = chord / 2.0 * p_r1;
								double radius = (pow(chord / 2.0, 2.0) + pow(s, 2.0)) / (2.0 * s);

								// calc center-point
								double d = sqrt((pl_last_x - p_x1)*(pl_last_x - p_x1) + (pl_last_y - p_y1)*(pl_last_y - p_y1));
								double a = (radius * radius - radius * radius + d * d) / (2 * d);
								double h = sqrt(radius * radius - a * a);
								double tx = (p_x1 - pl_last_x) * (a / d) + pl_last_x;
								double ty = (p_y1 - pl_last_y) * (a / d) + pl_last_y;
								double cx = tx + h * (p_y1 - pl_last_y) / d;
								double cy = ty - h * (p_x1 - pl_last_x) / d;
								double x4 = tx - h * (p_y1 - pl_last_y) / d;
								double y4 = ty + h * (p_x1 - pl_last_x) / d;

//								add_line(TYPE_LINE, dxf_options[8], cx, cy, cx, cy, 0.0, 0.0, 0.0);

								if (p_r1 > 0.0) {
									cx = x4;
									cy = y4;
								}

//								add_line(TYPE_LINE, dxf_options[8], x4, y4, x4, y4, 0.0, 0.0, 0.0);
//								add_line(TYPE_ARC, dxf_options[8], pl_last_x, pl_last_y, p_x1, p_y1, radius, 0.0, 0.0);

								// calc start/end angle
								double a1 = vector_angle(cx, cy, pl_last_x, pl_last_y);
								double a2 = vector_angle(cx, cy, p_x1, p_y1);

								//printf("## %f %f \n", a1 , a1);

								// split arcs
								double p_x1 = cx;
								double p_y1 = cy;
								double p_y2 = fabs(radius);
								double p_a1 = a2;
								double p_a2 = a1;
								if (p_r1 > 0.0) {
									p_a1 = a1;
									p_a2 = a2;
								}
								if (p_a1 > p_a2) {
									p_a2 += 360.0;
								}
								double r = p_y2;
								double angle2 = toRad(p_a1);
								double x2 = r * cos(angle2);
								double y2 = r * sin(angle2);
								double last_x = (p_x1 + x2);
								double last_y = (p_y1 + y2);
								double an = 0;
								double p_rast = (p_a2 - p_a1) / 18.0;
								for (an = p_a1 + p_rast; an <= p_a2 - (p_rast / 2.0); an += p_rast) {
									double angle1 = toRad(an);
									double x1 = r * cos(angle1);
									double y1 = r * sin(angle1);
									add_line(TYPE_ARC, dxf_options[8], last_x, last_y, p_x1 + x1, p_y1 + y1, r, p_x1, p_y1);
									last_x = p_x1 + x1;
									last_y = p_y1 + y1;
								}
								double angle3 = toRad(p_a2);
								double x3 = r * cos(angle3);
								double y3 = r * sin(angle3);
								add_line(TYPE_ARC, dxf_options[8], last_x, last_y, p_x1 + x3, p_y1 + y3, r, p_x1, p_y1);

							}
						} else {
							pl_first_x = p_x1;
							pl_first_y = p_y1;
						}
						dxf_options[42][0] = 0;
						pl_last_x = p_x1;
						pl_last_y = p_y1;
						lwpl_flag++;
					}
				}
			}
		}
	}

//exit(0);
	fclose(fp);
}

