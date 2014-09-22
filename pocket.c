/*

 Copyright 2014 by Oliver Dippel <oliver@multixmedia.org>

 MacOSX - Changes by McUles <mcules@fpv-club.de>
	Yosemite (OSX 10.10)

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

#include <GL/gl.h>
#include <GL/glu.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <gtksourceview/gtksourceview.h>
#include <gtksourceview/gtksourcelanguagemanager.h>
#ifdef USE_VNC
#include <gtk-vnc.h>
#endif
#include <libgen.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#ifdef __APPLE__
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <pwd.h>
#include <dxf.h>
#include <font.h>
#include <setup.h>
#include <postprocessor.h>
#include <calc.h>
#include <pocket.h>

typedef struct{
	double x1;
	double y1;
	double x2;
	double y2;
	int used;
	int visited;
} _POCKETLINE;

_POCKETLINE myPOCKETLINES[10000];


void mill_pocketline (int object_num, double *next_x, double *next_y) {
	double last_x = 0.0;
	double last_y = 0.0;
	int n = 0;
	int n2 = 0;
	double dist = 0.0;

	double x_prio = 5.0;

//	last_x = *next_x;
//	last_y = *next_y;

	mill_move_in(myPOCKETLINES[0].x1, myPOCKETLINES[0].y1, myOBJECTS[object_num].depth, 0, object_num);
	mill_z(1, myOBJECTS[object_num].depth);
	mill_xy(1, myPOCKETLINES[0].x2, myPOCKETLINES[0].y2, myOBJECTS[object_num].depth, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
	last_x = myPOCKETLINES[0].x2;
	last_y = myPOCKETLINES[0].y2;
	myPOCKETLINES[0].visited = 1;


	for (n = 0; n < 10000; n++) {
		double shortest_dist = 999999.0;
		int shortest_line = -1;
		int shortest_side = -1;
		for (n2 = 0; n2 < 10000; n2++) {
			if (myPOCKETLINES[n2].used == 1 && myPOCKETLINES[n2].visited == 0) {
				dist = get_len(last_x, last_y * x_prio, myPOCKETLINES[n2].x1, myPOCKETLINES[n2].y1 * x_prio);
				if (shortest_dist > dist) {
					shortest_dist = dist;
					shortest_line = n2;
					shortest_side = 0;
				}
				dist = get_len(last_x, last_y * x_prio, myPOCKETLINES[n2].x2, myPOCKETLINES[n2].y2 * x_prio);
				if (shortest_dist > dist) {
					shortest_dist = dist;
					shortest_line = n2;
					shortest_side = 1;
				}
			}
		}
		if (shortest_side == 0) {
			dist = get_len(last_x, last_y, myPOCKETLINES[shortest_line].x1, myPOCKETLINES[shortest_line].y1);
			myPOCKETLINES[shortest_line].visited = 1;
			if (dist > PARAMETER[P_TOOL_DIAMETER].vdouble) {
				mill_z(0, PARAMETER[P_CUT_SAVE].vdouble);
				mill_xy(0, myPOCKETLINES[shortest_line].x1, myPOCKETLINES[shortest_line].y1, myOBJECTS[object_num].depth, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
				mill_z(1, myOBJECTS[object_num].depth);
			} else {
				mill_xy(1, myPOCKETLINES[shortest_line].x1, myPOCKETLINES[shortest_line].y1, myOBJECTS[object_num].depth, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
			}
			mill_xy(1, myPOCKETLINES[shortest_line].x2, myPOCKETLINES[shortest_line].y2, myOBJECTS[object_num].depth, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
			last_x = myPOCKETLINES[shortest_line].x2;
			last_y = myPOCKETLINES[shortest_line].y2;
		} else if (shortest_side == 1) {
			dist = get_len(last_x, last_y, myPOCKETLINES[shortest_line].x2, myPOCKETLINES[shortest_line].y2);
			myPOCKETLINES[shortest_line].visited = 1;
			if (dist > PARAMETER[P_TOOL_DIAMETER].vdouble) {
				mill_z(0, PARAMETER[P_CUT_SAVE].vdouble);
				mill_xy(0, myPOCKETLINES[shortest_line].x2, myPOCKETLINES[shortest_line].y2, myOBJECTS[object_num].depth, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
				mill_z(1, myOBJECTS[object_num].depth);
			} else {
				mill_xy(1, myPOCKETLINES[shortest_line].x2, myPOCKETLINES[shortest_line].y2, myOBJECTS[object_num].depth, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
			}
			mill_xy(1, myPOCKETLINES[shortest_line].x1, myPOCKETLINES[shortest_line].y1, myOBJECTS[object_num].depth, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
			last_x = myPOCKETLINES[shortest_line].x1;
			last_y = myPOCKETLINES[shortest_line].y1;
		} else {
			break;
		}
	}
	*next_x = last_x;
	*next_y = last_y;
}

void mill_pocket (int object_num, double *next_x, double *next_y) {
	int n = 0;
	int plnum = 0;
	for (n = 0; n < 10000; n++) {
		myPOCKETLINES[n].used = 0;
		myPOCKETLINES[n].visited = 0;
		myPOCKETLINES[n].x1 = 0.0;
		myPOCKETLINES[n].y1 = 0.0;
		myPOCKETLINES[n].x2 = 0.0;
		myPOCKETLINES[n].y2 = 0.0;
	}
	if (myOBJECTS[object_num].closed == 1 && myOBJECTS[object_num].inside == 1) {
		double pmx = 0.0;
		double pmy = 0.0;
		double last_x = 0.0;
		double last_y = 0.0;
		double in_x = 0.0;
		double in_y = 0.0;
		int in_flag = 0;
		if (myLINES[myOBJECTS[object_num].line[0]].type == TYPE_CIRCLE) {
			double rs = 0.0;
			double cx = myLINES[myOBJECTS[object_num].line[0]].cx;
			double cy = myLINES[myOBJECTS[object_num].line[0]].cy;
			double r = myLINES[myOBJECTS[object_num].line[0]].opt;
			mill_move_in(cx, cy, 0.0, 0, object_num);
			mill_z(1, myOBJECTS[object_num].depth);
			for (rs = PARAMETER[P_TOOL_DIAMETER].vdouble * (double)PARAMETER[P_M_POCKETSTEP].vint / 100.0; rs < r - PARAMETER[P_TOOL_DIAMETER].vdouble * (double)PARAMETER[P_M_POCKETSTEP].vint / 100.0; rs += PARAMETER[P_TOOL_DIAMETER].vdouble * (double)PARAMETER[P_M_POCKETSTEP].vint / 100.0) {
				mill_xy(1, cx - rs, cy, myOBJECTS[object_num].depth, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
				if (myOBJECTS[object_num].climb == 0) {
					mill_circle(2, cx, cy, rs, myOBJECTS[object_num].depth, PARAMETER[P_M_FEEDRATE].vint, myOBJECTS[object_num].inside, object_num, "");
				} else {
					mill_circle(3, cx, cy, rs, myOBJECTS[object_num].depth, PARAMETER[P_M_FEEDRATE].vint, myOBJECTS[object_num].inside, object_num, "");
				}
			}
			*next_x = cx - rs;
			*next_y = cy;
		} else {
			int pipret = 0;
			float ystep = PARAMETER[P_TOOL_DIAMETER].vdouble * (double)PARAMETER[P_M_POCKETSTEP].vint / 100.0;
			float xstep = PARAMETER[P_TOOL_DIAMETER].vdouble / 10.0;
			for (pmy = myOBJECTS[object_num].min_y - ystep; pmy <= myOBJECTS[object_num].max_y + ystep; pmy += ystep) {
				for (pmx = myOBJECTS[object_num].min_x - xstep; pmx <= myOBJECTS[object_num].max_x + xstep; pmx += xstep) {
					pipret = point_in_object(object_num, -1, pmx, pmy);
					if (pipret == 1) {
						float an = 0;
						for (an = 0.0; an < 360.0; an += 3.6) {
							float rangle = toRad(an);
							float rx = (PARAMETER[P_TOOL_DIAMETER].vdouble / 3.0 * 2.0) * cos(rangle);
							float ry = (PARAMETER[P_TOOL_DIAMETER].vdouble / 3.0 * 2.0) * sin(rangle);
							if (point_in_object(object_num, -1, pmx + rx, pmy + ry) == 0) {
								pipret = 0;
							}
						}
						if (pipret == 1) {
							if (in_flag == 0) {
								in_flag = 1;
								in_x = pmx;
								in_y = pmy;
							} else {
								last_x = pmx;
								last_y = pmy;
							}
	
						} else if (in_flag == 1) {
							in_flag = 0;
							myPOCKETLINES[plnum].used = 1;
							myPOCKETLINES[plnum].x1 = in_x;
							myPOCKETLINES[plnum].y1 = in_y;
							myPOCKETLINES[plnum].x2 = last_x;
							myPOCKETLINES[plnum].y2 = last_y;
							plnum++;
						}
					} else {
						if (in_flag == 1) {
							in_flag = 0;
							myPOCKETLINES[plnum].used = 1;
							myPOCKETLINES[plnum].x1 = in_x;
							myPOCKETLINES[plnum].y1 = in_y;
							myPOCKETLINES[plnum].x2 = last_x;
							myPOCKETLINES[plnum].y2 = last_y;
							plnum++;
						}
					}
				}
			}
			mill_pocketline(object_num, next_x, next_y);
		}
	}
}
