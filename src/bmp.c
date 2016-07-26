/*

 Copyright 2014-2016 by Oliver Dippel <oliver@multixmedia.org>

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

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkgl.h>
#include <gtksourceview/gtksourceview.h>
#include <gtksourceview/gtksourcelanguagemanager.h>
#ifdef USE_VNC
#include <gtk-vnc.h>
#endif
#ifdef USE_WEBKIT
#include <webkit/webkitwebview.h>
#endif
#include <libgen.h>
#include <math.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#ifndef __MINGW32__
#include <sys/wait.h>
#endif
#ifdef __APPLE__
#include <malloc/malloc.h>
#endif
#ifdef USE_G3D
#include <g3d/g3d.h>
void slice_3d (char *file, float z);
#endif
#include <locale.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <dxf.h>
#include <font.h>
#include <setup.h>
#include <postprocessor.h>
#include <calc.h>
#include <bmp.h>

#include "os-hacks.h"

#include <libintl.h>
#define _(String) gettext(String)
#define gettext_noop(String) String
#define N_(String) gettext_noop(String)

GtkWidget *imgCanvas;
extern float size_x;
extern float size_y;
extern double min_x;
extern double min_y;
extern double max_x;
extern double max_y;
extern void update_gui (void);

static int moves[10000][5];
static int bmp_width = 0;
static int bmp_height = 0;
static int ml = 0;

void get_pixel (GdkPixbuf *pixbuf, int x, int y, guchar *red, guchar *green, guchar *blue, guchar *alpha) {
	int width, height, rowstride, n_channels;
	guchar *pixels, *p;
	n_channels = gdk_pixbuf_get_n_channels (pixbuf);
	g_assert (gdk_pixbuf_get_colorspace (pixbuf) == GDK_COLORSPACE_RGB);
	g_assert (gdk_pixbuf_get_bits_per_sample (pixbuf) == 8);
	g_assert (gdk_pixbuf_get_has_alpha (pixbuf));
	g_assert (n_channels == 4);
	width = gdk_pixbuf_get_width (pixbuf);
	height = gdk_pixbuf_get_height (pixbuf);
	g_assert (x >= 0 && x < width);
	g_assert (y >= 0 && y < height);
	rowstride = gdk_pixbuf_get_rowstride (pixbuf);
	pixels = gdk_pixbuf_get_pixels (pixbuf);
	p = pixels + y * rowstride + x * n_channels;
	*red = p[0];
	*green = p[1];
	*blue = p[2];
	*alpha = p[3];
}

void put_pixel (GdkPixbuf *pixbuf, int x, int y, guchar red, guchar green, guchar blue, guchar alpha) {
	int width, height, rowstride, n_channels;
	guchar *pixels, *p;
	n_channels = gdk_pixbuf_get_n_channels (pixbuf);
	g_assert (gdk_pixbuf_get_colorspace (pixbuf) == GDK_COLORSPACE_RGB);
	g_assert (gdk_pixbuf_get_bits_per_sample (pixbuf) == 8);
	g_assert (gdk_pixbuf_get_has_alpha (pixbuf));
	g_assert (n_channels == 4);
	width = gdk_pixbuf_get_width (pixbuf);
	height = gdk_pixbuf_get_height (pixbuf);
	g_assert (x >= 0 && x < width);
	g_assert (y >= 0 && y < height);
	rowstride = gdk_pixbuf_get_rowstride (pixbuf);
	pixels = gdk_pixbuf_get_pixels (pixbuf);
	p = pixels + y * rowstride + x * n_channels;
	p[0] = red;
	p[1] = green;
	p[2] = blue;
	p[3] = alpha;
}

int get_dist (int x1, int y1, int x2, int y2) {
	float dx = (float)(x2 - x1);
	float dy = (float)(y2 - y1);
	int len = (int)sqrt(dx * dx + dy * dy);
	return len;
}

void bitmap_load (char *filename) {
	imgCanvas = gtk_image_new_from_file (filename);
}

void bitmap_pre (void) {
	int x = 0;
	int y = 0;
	int x_last = 0;
	int y_last = 0;
	int x_first = 0;
	int y_first = 0;
	guchar red;
	guchar green;
	guchar blue;
	guchar alpha;
	int xstep = 1;
	int ystep = 1;
	int lon = 0;
	float scale = PARAMETER[P_B_SCALE].vdouble;
	int mn = 0;
	GdkPixbuf *surface_pixbuf = gtk_image_get_pixbuf((GtkImage*)imgCanvas);
	bmp_width = gdk_pixbuf_get_width(surface_pixbuf);
	bmp_height = gdk_pixbuf_get_height(surface_pixbuf);
	min_x = 99999.0;
	min_y = 99999.0;
	max_x = -99999.0;
	max_y = -99999.0;
	for (mn = 0; mn < 10000; mn++) {
		moves[mn][0] = 0;
		moves[mn][1] = 0;
		moves[mn][2] = 0;
		moves[mn][3] = 0;
		moves[mn][4] = 0; // flag
	}
	for (y = 0; y < bmp_height; y += ystep) {
		for (x = 0; x < bmp_width; x += xstep) {
			get_pixel(surface_pixbuf, x, y, &red, &green, &blue, &alpha);
			if (alpha > 0 && red >= PARAMETER[P_B_R].vint && green >= PARAMETER[P_B_G].vint && blue >= PARAMETER[P_B_B].vint) {
				if (lon == 0) {
					lon = 1;
					x_first = x;
					y_first = y;
				}
				x_last = x;
				y_last = y;
				if (min_x > (double)x * scale) {
					min_x = (double)x * scale;
				}
				if (min_y > (double)y * scale) {
					min_y = (double)y * scale;
				}
				if (max_x < (double)x * scale) {
					max_x = (double)x * scale;
				}
				if (max_y < (double)y * scale) {
					max_y = (double)y * scale;
				}
//				put_pixel(surface_pixbuf, x, y, 0, 0, 255, 255);
			} else {
				if (lon == 1) {
					lon = 0;
					moves[ml][0] = x_first;
					moves[ml][1] = y_first;
					moves[ml][2] = x_last;
					moves[ml][3] = y_last;
					moves[ml][4] = 1;
					ml++;
				}
			}
		}
		if (lon == 1) {
			lon = 0;
			moves[ml][0] = x_first;
			moves[ml][1] = y_first;
			moves[ml][2] = x_last;
			moves[ml][3] = y_last;
			moves[ml][4] = 1;
			ml++;
		}
		y += ystep;
		if (y < bmp_height) {
			for (x = bmp_width - 1; x >= 0; x -= xstep) {
				get_pixel(surface_pixbuf, x, y, &red, &green, &blue, &alpha);
				if (alpha > 0 && red >= PARAMETER[P_B_R].vint && green >= PARAMETER[P_B_G].vint && blue >= PARAMETER[P_B_B].vint) {
					if (lon == 0) {
						lon = 1;
						x_first = x;
						y_first = y;
					}
					x_last = x;
					y_last = y;
					if (min_x > (double)x * scale) {
						min_x = (double)x * scale;
					}
					if (min_y > (double)y * scale) {
						min_y = (double)y * scale;
					}
					if (max_x < (double)x * scale) {
						max_x = (double)x * scale;
					}
					if (max_y < (double)y * scale) {
						max_y = (double)y * scale;
					}
//					put_pixel(surface_pixbuf, x, y, 0, 0, 255, 255);
				} else {
					if (lon == 1) {
						lon = 0;
						moves[ml][0] = x_first;
						moves[ml][1] = y_first;
						moves[ml][2] = x_last;
						moves[ml][3] = y_last;
						moves[ml][4] = 1;
						ml++;
					}
				}
			}
			if (lon == 1) {
				lon = 0;
				moves[ml][0] = x_first;
				moves[ml][1] = y_first;
				moves[ml][2] = x_last;
				moves[ml][3] = y_last;
				moves[ml][4] = 1;
				ml++;
			}
		}
	}
	size_x = (float)max_x * scale;
	size_y = (float)max_y * scale;
}

void bitmap2cnc (void) {
	float scale = PARAMETER[P_B_SCALE].vdouble;
	int mn = 0;
	double depth = 0.0;
	int object_num = 0;
	myOBJECTS[object_num].order = 1;
	myOBJECTS[object_num].tabs = PARAMETER[P_T_USE].vint;
	if (PARAMETER[P_M_LASERMODE].vint == 1) {
		myOBJECTS[object_num].depth = 0.0;
	} else {
		myOBJECTS[object_num].depth = PARAMETER[P_M_DEPTH].vdouble;
	}
	myOBJECTS[object_num].overcut = PARAMETER[P_M_OVERCUT].vint;
	myOBJECTS[object_num].laser = PARAMETER[P_M_LASERMODE].vint;
	myOBJECTS[object_num].climb = PARAMETER[P_M_CLIMB].vint;
	myOBJECTS[object_num].helix = PARAMETER[P_M_HELIX].vint;
	myOBJECTS[object_num].tool_num = PARAMETER[P_TOOL_NUM].vint;
	myOBJECTS[object_num].tool_dia = PARAMETER[P_TOOL_DIAMETER].vdouble;
	myOBJECTS[object_num].tool_speed = PARAMETER[P_TOOL_SPEED].vint;
	for (depth = PARAMETER[P_M_Z_STEP].vdouble; depth > PARAMETER[P_M_DEPTH].vdouble + PARAMETER[P_M_Z_STEP].vdouble; depth += PARAMETER[P_M_Z_STEP].vdouble) {
		if (PARAMETER[P_M_LASERMODE].vint == 0) {
			mill_z(0, PARAMETER[P_CUT_SAVE].vdouble);
		} else {
			mill_z(0, 0.0);
		}
		int xx = 0;
		int epx = 0;
		int epy = 0;
		for (xx = 0; xx < 10000; xx++) {
			int pn = -1;
			int pd = 9999;
			int ps = 0;
			int dist = 0;
			for (mn = 0; mn < ml; mn++) {
				if (moves[mn][4] == 1) {
					dist = get_dist (epx, epy, moves[mn][0], moves[mn][1]);
					if (dist < pd) {
						pd = dist;
						pn = mn;
						ps = 0;
					}
					dist = get_dist (epx, epy, moves[mn][2], moves[mn][3]);
					if (dist < pd) {
						pd = dist;
						pn = mn;
						ps = 1;
					}
				}
			}
			if (pn != -1) {
				moves[pn][4] = 2;
				if (ps == 0) {
					mill_move_in((double)moves[pn][0] * scale, (double)(bmp_height - moves[pn][1]) * scale, depth, PARAMETER[P_M_LASERMODE].vint, object_num);
					if (PARAMETER[P_M_LASERMODE].vint == 0) {
						mill_z(1, depth);
					}
					mill_xy(1, (double)moves[pn][2] * scale, (double)(bmp_height - moves[pn][3]) * scale, 0.0, PARAMETER[P_M_FEEDRATE].vint, 0, "");
					mill_move_out(PARAMETER[P_M_LASERMODE].vint, object_num);
					epx = moves[pn][2];
					epy = moves[pn][3];
				} else {
					mill_move_in((double)moves[pn][2] * scale, (double)(bmp_height - moves[pn][3]) * scale, depth, PARAMETER[P_M_LASERMODE].vint, object_num);
					if (PARAMETER[P_M_LASERMODE].vint == 0) {
						mill_z(1, depth);
					}
					mill_xy(1, (double)moves[pn][0] * scale, (double)(bmp_height - moves[pn][1]) * scale, 0.0, PARAMETER[P_M_FEEDRATE].vint, 0, "");
					mill_move_out(PARAMETER[P_M_LASERMODE].vint, object_num);
					epx = moves[pn][0];
					epy = moves[pn][1];
				}
			} else {
				break;
			}
		}
		if (PARAMETER[P_M_LASERMODE].vint == 1) {
			break;
		}

		for (mn = 0; mn < 10000; mn++) {
			if (moves[mn][4] == 2) {
				moves[mn][4] = 1; // flag
			}
		}
	}
}
