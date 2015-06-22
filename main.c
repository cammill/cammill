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

#include "os-hacks.h"

#include <libintl.h>
#define _(String) gettext(String)
#define gettext_noop(String) String
#define N_(String) gettext_noop(String)


void texture_init (void);

// path to cammill executable
char program_path[PATH_MAX];

char *about1 = "CAMmill 2D";
char *author1 = "Oliver Dippel <oliver@multixmedia.org>\nMac-Port by McUles <mcules@fpv-club.de>";
char *author2 = "improvements by Jakob Flierl <koppi> and Carlo <onekk>";
char *website = "Website: https://github.com/cammill/cammill\nIRC: #cammill (freenode) ";

int select_object_flag = 0;
int select_object_x = 0;
int select_object_y = 0;

int winw = 1600;
int winh = 1200;
float size_x = 0.0;
float size_y = 0.0;
double min_x = 99999.0;
double min_y = 99999.0;
double max_x = 0.0;
double max_y = 0.0;
double tooltbl_diameters[100];
FILE *fd_out = NULL;
int object_last = 0;
int save_gcode = 0;
char tool_descr[100][1024];
int tools_max = 0;
char postcam_plugins[100][1024];
int postcam_plugin = -1;
int update_post = 1;
char *output_buffer = NULL;
char output_extension[128];
char output_info[1024];
char output_error[1024];
int loading = 0;

int last_mouse_x = 0;
int last_mouse_y = 0;
int last_mouse_button = -1;
int last_mouse_state = 0;

void ParameterUpdate (void);
void ParameterChanged (GtkWidget *widget, gpointer data);

#ifdef USE_VNC
GtkWidget *VncView;
#endif
#ifdef USE_WEBKIT
GtkWidget *WebKit;
#endif
GtkWidget *gCodeViewLabel;
GtkWidget *gCodeViewLabelLua;
GtkWidget *OutputInfoLabel;
GtkWidget *OutputErrorLabel;
GtkWidget *SizeInfoLabel;
GtkWidget *StatusBar;
GtkTreeStore *treestore;
GtkListStore *ListStore[P_LAST];
GtkWidget *ParamValue[P_LAST];
GtkWidget *ParamButton[P_LAST];
GtkWidget *glCanvas;
GtkWidget *gCodeView;
GtkWidget *gCodeViewLua;
GtkWidget *hbox;
GtkWidget *ViewExpander;
GtkWidget *ToolExpander;
GtkWidget *MillingExpander;
GtkWidget *TabsExpander;
GtkWidget *RotaryExpander;
GtkWidget *TangencialExpander;
GtkWidget *MachineExpander;
GtkWidget *MaterialExpander;
GtkWidget *ObjectsExpander;
GtkWidget *MiscExpander;
int PannedStat;
int ExpanderStat[20];

int width = 800;
int height = 600;
int need_init = 1;

double mill_distance_xy = 0.0;
double mill_distance_z = 0.0;
double move_distance_xy = 0.0;
double move_distance_z = 0.0;

GtkWidget *window;
GtkWidget *dialog;



void postcam_load_source (char *plugin) {
	char tmp_str[PATH_MAX];
	snprintf(tmp_str, PATH_MAX, "%sposts/%s.scpost", program_path, plugin);
	GtkTextBuffer *bufferLua = gtk_text_view_get_buffer(GTK_TEXT_VIEW(gCodeViewLua));
	gchar *file_buffer;
	GError *error;
	gboolean read_file_status = g_file_get_contents(tmp_str, &file_buffer, NULL, &error);
	if (read_file_status == FALSE) {
		g_error("error opening file: %s\n",error && error->message ? error->message : "No Detail");
		return;
	}

	gtk_text_buffer_set_text(bufferLua, file_buffer, -1);
	free(file_buffer);
}

void postcam_save_source (const char* path, char *plugin) {
	char tmp_str[PATH_MAX];
	snprintf(tmp_str, PATH_MAX, "%sposts/%s.scpost", path, plugin);
	FILE *fp = fopen(tmp_str, "w");
	if (fp != NULL) {
		GtkTextIter start, end;
		GtkTextBuffer *bufferLua;
		bufferLua = gtk_text_view_get_buffer(GTK_TEXT_VIEW(gCodeViewLua));
		gtk_text_buffer_get_bounds(bufferLua, &start, &end);
		char *luacode = gtk_text_buffer_get_text(bufferLua, &start, &end, TRUE);
		fprintf(fp, "%s", luacode);
		fclose(fp);
		free(luacode);
	}
}

void SetQuaternionFromAxisAngle (const float *axis, float angle, float *quat) {
	float sina2, norm;
	sina2 = (float)sin(0.5f * angle);
	norm = (float)sqrt(axis[0]*axis[0] + axis[1]*axis[1] + axis[2]*axis[2]);
	quat[0] = sina2 * axis[0] / norm;
	quat[1] = sina2 * axis[1] / norm;
	quat[2] = sina2 * axis[2] / norm;
	quat[3] = (float)cos(0.5f * angle);
}

void ConvertQuaternionToMatrix (const float *quat, float *mat) {
	float yy2 = 2.0f * quat[1] * quat[1];
	float xy2 = 2.0f * quat[0] * quat[1];
	float xz2 = 2.0f * quat[0] * quat[2];
	float yz2 = 2.0f * quat[1] * quat[2];
	float zz2 = 2.0f * quat[2] * quat[2];
	float wz2 = 2.0f * quat[3] * quat[2];
	float wy2 = 2.0f * quat[3] * quat[1];
	float wx2 = 2.0f * quat[3] * quat[0];
	float xx2 = 2.0f * quat[0] * quat[0];
	mat[0*4+0] = - yy2 - zz2 + 1.0f;
	mat[0*4+1] = xy2 + wz2;
	mat[0*4+2] = xz2 - wy2;
	mat[0*4+3] = 0;
	mat[1*4+0] = xy2 - wz2;
	mat[1*4+1] = - xx2 - zz2 + 1.0f;
	mat[1*4+2] = yz2 + wx2;
	mat[1*4+3] = 0;
	mat[2*4+0] = xz2 + wy2;
	mat[2*4+1] = yz2 - wx2;
	mat[2*4+2] = - xx2 - yy2 + 1.0f;
	mat[2*4+3] = 0;
	mat[3*4+0] = mat[3*4+1] = mat[3*4+2] = 0;
	mat[3*4+3] = 1;
}

void MultiplyQuaternions (const float *q1, const float *q2, float *qout) {
	float qr[4];
	qr[0] = q1[3]*q2[0] + q1[0]*q2[3] + q1[1]*q2[2] - q1[2]*q2[1];
	qr[1] = q1[3]*q2[1] + q1[1]*q2[3] + q1[2]*q2[0] - q1[0]*q2[2];
	qr[2] = q1[3]*q2[2] + q1[2]*q2[3] + q1[0]*q2[1] - q1[1]*q2[0];
	qr[3]  = q1[3]*q2[3] - (q1[0]*q2[0] + q1[1]*q2[1] + q1[2]*q2[2]);
	qout[0] = qr[0]; qout[1] = qr[1]; qout[2] = qr[2]; qout[3] = qr[3];
}

void onExit (void) {
}

void draw_grid (void) {
	if (PARAMETER[P_M_ROTARYMODE].vint == 0 && PARAMETER[P_V_GRID].vint == 1) {
		float gridXYZ = PARAMETER[P_V_HELP_GRID].vfloat * 10.0;
		float gridXYZmin = PARAMETER[P_V_HELP_GRID].vfloat;
		float lenY = size_y;
		float lenX = size_x;
		int pos_n = 0;
		glColor4f(1.0, 1.0, 1.0, 0.3);
		for (pos_n = 0; pos_n <= lenY; pos_n += gridXYZ) {
			glBegin(GL_LINES);
			glVertex3f(0.0, pos_n, PARAMETER[P_M_DEPTH].vdouble - 0.1);
			glVertex3f(lenX, pos_n, PARAMETER[P_M_DEPTH].vdouble - 0.1);
			glEnd();
		}
		for (pos_n = 0; pos_n <= lenX; pos_n += gridXYZ) {
			glBegin(GL_LINES);
			glVertex3f(pos_n, 0.0, PARAMETER[P_M_DEPTH].vdouble - 0.1);
			glVertex3f(pos_n, lenY, PARAMETER[P_M_DEPTH].vdouble - 0.1);
			glEnd();
		}
		glColor4f(1.0, 1.0, 1.0, 0.2);
		for (pos_n = 0; pos_n <= lenY; pos_n += gridXYZmin) {
			glBegin(GL_LINES);
			glVertex3f(0.0, pos_n, PARAMETER[P_M_DEPTH].vdouble - 0.1);
			glVertex3f(lenX, pos_n, PARAMETER[P_M_DEPTH].vdouble - 0.1);
			glEnd();
		}
		for (pos_n = 0; pos_n <= lenX; pos_n += gridXYZmin) {
			glBegin(GL_LINES);
			glVertex3f(pos_n, 0.0, PARAMETER[P_M_DEPTH].vdouble - 0.1);
			glVertex3f(pos_n, lenY, PARAMETER[P_M_DEPTH].vdouble - 0.1);
			glEnd();
		}
	}
}

void draw_helplines (void) {
	char tmp_str[128];
	if (PARAMETER[P_M_ROTARYMODE].vint == 1) {
		GLUquadricObj *quadratic = gluNewQuadric();
		float radius = (PARAMETER[P_MAT_DIAMETER].vdouble / 2.0) + PARAMETER[P_M_DEPTH].vdouble;
		float radius2 = (PARAMETER[P_MAT_DIAMETER].vdouble / 2.0);
		glPushMatrix();
		glTranslatef(0.0, -radius2 - 10.0, 0.0);
		float lenX = size_x;
		float offXYZ = 10.0;
		float arrow_d = 1.0;
		float arrow_l = 6.0;
		glColor4f(0.0, 1.0, 0.0, 1.0);
		glBegin(GL_LINES);
		glVertex3f(0.0, -offXYZ, 0.0);
		glVertex3f(0.0, 0.0, 0.0);
		glEnd();
		glBegin(GL_LINES);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(lenX, 0.0, 0.0);
		glEnd();
		glBegin(GL_LINES);
		glVertex3f(lenX, -offXYZ, 0.0);
		glVertex3f(lenX, 0.0, 0.0);
		glEnd();
		glPushMatrix();
		glTranslatef(lenX, -offXYZ, 0.0);
		glPushMatrix();
		glTranslatef(-lenX / 2.0, -arrow_d * 2.0 - 11.0, 0.0);
		snprintf(tmp_str, sizeof(tmp_str), "%0.2fmm", lenX);
		output_text_gl_center(tmp_str, 0.0, 0.0, 0.0, 0.2);
		glPopMatrix();
		glRotatef(-90.0, 0.0, 1.0, 0.0);
		gluCylinder(quadratic, 0.0, (arrow_d * 3), arrow_l ,32, 1);
		glTranslatef(0.0, 0.0, arrow_l);
		gluCylinder(quadratic, arrow_d, arrow_d, lenX - arrow_l * 2.0 ,32, 1);
		glTranslatef(0.0, 0.0, lenX - arrow_l * 2.0);
		gluCylinder(quadratic, (arrow_d * 3), 0.0, arrow_l ,32, 1);
		glPopMatrix();
		glPopMatrix();

		glColor4f(0.2, 0.2, 0.2, 0.5);
		glPushMatrix();
		glRotatef(90.0, 0.0, 1.0, 0.0);
		gluCylinder(quadratic, radius, radius, size_x ,64, 1);
		glTranslatef(0.0, 0.0, -size_x);
		gluCylinder(quadratic, radius2, radius2, size_x ,64, 1);
		glTranslatef(0.0, 0.0, size_x * 2);
		gluCylinder(quadratic, radius2, radius2, size_x ,64, 1);
		glPopMatrix();

		return;
	}

	/* Scale-Arrow's */
	float lenY = size_y;
	float lenX = size_x;
	float lenZ = PARAMETER[P_M_DEPTH].vdouble * -1;
	float offXYZ = 10.0;
	float arrow_d = 1.0 * PARAMETER[P_V_HELP_ARROW].vfloat;
	float arrow_l = 6.0 * PARAMETER[P_V_HELP_ARROW].vfloat;
	GLUquadricObj *quadratic = gluNewQuadric();

	glColor4f(1.0, 0.0, 0.0, 1.0);
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(-offXYZ, 0.0, 0.0);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, lenY, 0.0);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(0.0, lenY, 0.0);
	glVertex3f(-offXYZ, lenY, 0.0);
	glEnd();
	glPushMatrix();
	glTranslatef(0.0 - offXYZ, -0.0, 0.0);
	glPushMatrix();
	glTranslatef(arrow_d * 2.0 + 1.0, lenY / 2.0, 0.0);
	glRotatef(90.0, 0.0, 0.0, 1.0);
	snprintf(tmp_str, sizeof(tmp_str), "%0.2fmm", lenY);
	glPushMatrix();
	glTranslatef(0.0, 4.0, 0.0);
	output_text_gl_center(tmp_str, 0.0, 0.0, 0.0, 0.2);
	glPopMatrix();
	glPopMatrix();
	glRotatef(-90.0, 1.0, 0.0, 0.0);
	gluCylinder(quadratic, 0.0, (arrow_d * 3), arrow_l ,32, 1);
	glTranslatef(0.0, 0.0, arrow_l);
	gluCylinder(quadratic, arrow_d, arrow_d, lenY - arrow_l * 2.0 ,32, 1);
	glTranslatef(0.0, 0.0, lenY - arrow_l * 2.0);
	gluCylinder(quadratic, (arrow_d * 3), 0.0, arrow_l ,32, 1);
	glPopMatrix();

	glColor4f(0.0, 1.0, 0.0, 1.0);
	glBegin(GL_LINES);
	glVertex3f(0.0, -offXYZ, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(lenX, 0.0, 0.0);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(lenX, -offXYZ, 0.0);
	glVertex3f(lenX, 0.0, 0.0);
	glEnd();
	glPushMatrix();
	glTranslatef(lenX, -offXYZ, 0.0);
	glPushMatrix();
	glTranslatef(-lenX / 2.0, -arrow_d * 2.0 - 1.0, 0.0);
	snprintf(tmp_str, sizeof(tmp_str), "%0.2fmm", lenX);
	glPushMatrix();
	glTranslatef(0.0, -4.0, 0.0);
	output_text_gl_center(tmp_str, 0.0, 0.0, 0.0, 0.2);
	glPopMatrix();
	glPopMatrix();
	glRotatef(-90.0, 0.0, 1.0, 0.0);
	gluCylinder(quadratic, 0.0, (arrow_d * 3), arrow_l ,32, 1);
	glTranslatef(0.0, 0.0, arrow_l);
	gluCylinder(quadratic, arrow_d, arrow_d, lenX - arrow_l * 2.0 ,32, 1);
	glTranslatef(0.0, 0.0, lenX - arrow_l * 2.0);
	gluCylinder(quadratic, (arrow_d * 3), 0.0, arrow_l ,32, 1);
	glPopMatrix();

	glColor4f(0.0, 0.0, 1.0, 1.0);
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(-offXYZ, -offXYZ, 0.0);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, -lenZ);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, -lenZ);
	glVertex3f(-offXYZ, -offXYZ, -lenZ);
	glEnd();
	glPushMatrix();
	glTranslatef(-offXYZ, -offXYZ, -lenZ);
	glPushMatrix();
	glTranslatef(arrow_d * 2.0 - 1.0, -arrow_d * 2.0 - 1.0, lenZ / 2.0);
	glRotatef(90.0, 0.0, 1.0, 0.0);
	snprintf(tmp_str, sizeof(tmp_str), "%0.2fmm", lenZ);
	glPushMatrix();
	glTranslatef(0.0, -4.0, 0.0);
	output_text_gl_center(tmp_str, 0.0, 0.0, 0.0, 0.2);
	glPopMatrix();
	glPopMatrix();
	glRotatef(-90.0, 0.0, 0.0, 1.0);
	gluCylinder(quadratic, 0.0, (arrow_d * 3), arrow_l ,32, 1);
	glTranslatef(0.0, 0.0, arrow_l);
	gluCylinder(quadratic, arrow_d, arrow_d, lenZ - arrow_l * 2.0 ,32, 1);
	glTranslatef(0.0, 0.0, lenZ - arrow_l * 2.0);
	gluCylinder(quadratic, (arrow_d * 3), 0.0, arrow_l ,32, 1);
	glPopMatrix();
}

void select_object (GLint hits, GLuint buffer[]) {
	unsigned int i, j;
	GLuint names, *ptr;
	ptr = (GLuint *) buffer;
//	printf("hits = %d\n", hits);
	for (i = 0; i < hits; i++) {  /* for each hit  */
		names = *ptr;
//		printf(" number of names for hit = %d\n", names);
		ptr++;
//		printf("  z1 is %g;", (float) *ptr/0x7fffffff);
		ptr++;
//		printf(" z2 is %g\n", (float) *ptr/0x7fffffff);
		ptr++;
//		printf("   the name is ");
		for (j = 0; j < names; j++) {  /* for each name */
			PARAMETER[P_O_SELECT].vint = (int)*ptr;
//			printf("%d ", *ptr);
			ptr++;
		}
//		printf("\n");
	}
}

void mainloop (void) {
	char tmp_str[1024];
	size_x = (max_x - min_x);
	size_y = (max_y - min_y);
	float scale = (4.0 / size_x);
	if (scale > (4.0 / size_y)) {
		scale = (4.0 / size_y);
	}

	/* get diameter from tooltable by number */
	if (PARAMETER[P_TOOL_SELECT].vint > 0) {
		PARAMETER[P_TOOL_NUM].vint = PARAMETER[P_TOOL_SELECT].vint;
		PARAMETER[P_TOOL_DIAMETER].vdouble = tooltbl_diameters[PARAMETER[P_TOOL_NUM].vint];
	}
	PARAMETER[P_TOOL_SPEED_MAX].vint = (int)(((float)Material[PARAMETER[P_MAT_SELECT].vint].vc * 1000.0) / (PI * (PARAMETER[P_TOOL_DIAMETER].vdouble)));
	if ((PARAMETER[P_TOOL_DIAMETER].vdouble) < 4.0) {
		PARAMETER[P_M_FEEDRATE_MAX].vint = (int)((float)PARAMETER[P_TOOL_SPEED].vint * Material[PARAMETER[P_MAT_SELECT].vint].fz[FZ_FEEDFLUTE4] * (float)PARAMETER[P_TOOL_W].vint);
	} else if ((PARAMETER[P_TOOL_DIAMETER].vdouble) < 8.0) {
		PARAMETER[P_M_FEEDRATE_MAX].vint = (int)((float)PARAMETER[P_TOOL_SPEED].vint * Material[PARAMETER[P_MAT_SELECT].vint].fz[FZ_FEEDFLUTE8] * (float)PARAMETER[P_TOOL_W].vint);
	} else if ((PARAMETER[P_TOOL_DIAMETER].vdouble) < 12.0) {
		PARAMETER[P_M_FEEDRATE_MAX].vint = (int)((float)PARAMETER[P_TOOL_SPEED].vint * Material[PARAMETER[P_MAT_SELECT].vint].fz[FZ_FEEDFLUTE12] * (float)PARAMETER[P_TOOL_W].vint);
	}

	if (update_post == 1) {
		glDeleteLists(1, 1);
		glNewList(1, GL_COMPILE);

		draw_grid();
		if (PARAMETER[P_V_HELPLINES].vint == 1) {
			if (PARAMETER[P_M_ROTARYMODE].vint == 0) {
				draw_helplines();
			}
		}

		mill_begin(program_path);
		mill_objects();
		mill_end();

		// update GUI
		GtkTextIter startLua, endLua;
		GtkTextBuffer *bufferLua;
		bufferLua = gtk_text_view_get_buffer(GTK_TEXT_VIEW(gCodeViewLua));
		gtk_text_buffer_get_bounds(bufferLua, &startLua, &endLua);
		gtk_label_set_text(GTK_LABEL(OutputErrorLabel), output_error);
		update_post = 0;
		GtkTextIter start, end;
		GtkTextBuffer *buffer;
		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(gCodeView));
		gtk_text_buffer_get_bounds(buffer, &start, &end);
		char *gcode_check = gtk_text_buffer_get_text(buffer, &start, &end, TRUE);
		if (gcode_check != NULL) {
			if (output_buffer != NULL) {
				if (strcmp(gcode_check, output_buffer) != 0) {
					gtk_text_buffer_set_text(buffer, output_buffer, -1);
				}
			} else {
				gtk_text_buffer_set_text(buffer, "", -1);
			}
			free(gcode_check);
		} else {
			gtk_text_buffer_set_text(buffer, "", -1);
		}
		double milltime = mill_distance_xy / PARAMETER[P_M_FEEDRATE].vint;
		milltime += mill_distance_z / PARAMETER[P_M_PLUNGE_SPEED].vint;
		milltime += (move_distance_xy + move_distance_z) / PARAMETER[P_H_FEEDRATE_FAST].vint;
		snprintf(tmp_str, sizeof(tmp_str), _("Distance: Mill-XY=%0.2fmm/Z=%0.2fmm / Move-XY=%0.2fmm/Z=%0.2fmm / Time>%0.1fmin"), mill_distance_xy, mill_distance_z, move_distance_xy, move_distance_z, milltime);
		gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), tmp_str), tmp_str);
		snprintf(tmp_str, sizeof(tmp_str), "Width=%0.1fmm / Height=%0.1fmm", size_x, size_y);
		gtk_label_set_text(GTK_LABEL(SizeInfoLabel), tmp_str);
		glEndList();
	}

	// save output
	if (save_gcode == 1) {
		if (strcmp(PARAMETER[P_MFILE].vstr, "-") == 0) {
			fd_out = stdout;
		} else {
			fd_out = fopen(PARAMETER[P_MFILE].vstr, "w");
		}
		if (fd_out == NULL) {
			fprintf(stderr, "Can not open file: %s\n", PARAMETER[P_MFILE].vstr);
			exit(0);
		}
		fprintf(fd_out, "%s", output_buffer);
		fclose(fd_out);
		if (PARAMETER[P_POST_CMD].vstr[0] != 0) {
			char cmd_str[PATH_MAX];
			snprintf(cmd_str, PATH_MAX, "%s %s", PARAMETER[P_POST_CMD].vstr, PARAMETER[P_MFILE].vstr);
			system(cmd_str);
		}
		gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "saving g-code...done"), "saving g-code...done");
		save_gcode = 0;
	}

	if (PARAMETER[P_O_BATCHMODE].vint == 1) {
		onExit();
		exit(0);
	} else {

		#define BUFSIZE 512
		GLuint selectBuf[BUFSIZE];
		GLint hits;
		GLint viewport[4];
		int x = select_object_x;
		int y = select_object_y;

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		if (select_object_flag == 1) {
			glGetIntegerv(GL_VIEWPORT, viewport);
			glSelectBuffer(BUFSIZE, selectBuf);
			glRenderMode(GL_SELECT);
			glInitNames();
			glPushName(0);
			gluPickMatrix((GLdouble)(x), (GLdouble)(viewport[3] - y), 5.0, 5.0, viewport);
		}

		gluPerspective((M_PI / 4) / M_PI * 180, (float)width / (float)height, 0.1, 1000.0);
		gluLookAt(0, 0, 6,  0, 0, 0,  0, 1, 0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearDepth(1.0);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glEnable(GL_NORMALIZE);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_TRUE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glPushMatrix();

		glScalef(PARAMETER[P_V_ZOOM].vfloat, PARAMETER[P_V_ZOOM].vfloat, PARAMETER[P_V_ZOOM].vfloat);
		glScalef(scale, scale, scale);
		glTranslatef(PARAMETER[P_V_TRANSX].vint, PARAMETER[P_V_TRANSY].vint, 0.0);
		glRotatef(PARAMETER[P_V_ROTZ].vfloat, 0.0, 0.0, 1.0);
		glRotatef(PARAMETER[P_V_ROTY].vfloat, 0.0, 1.0, 0.0);
		glRotatef(PARAMETER[P_V_ROTX].vfloat, 1.0, 0.0, 0.0);
		glTranslatef(-size_x / 2.0, 0.0, 0.0);
		if (PARAMETER[P_M_ROTARYMODE].vint == 0) {
			glTranslatef(0.0, -size_y / 2.0, 0.0);
		}
		glCallList(1);
		if (select_object_flag == 0) {
			glCallList(2);
		}
		glPopMatrix();

		if (select_object_flag == 1) {
			glFlush();
//			printf("## %i %i \n", x , y);
			hits = glRenderMode(GL_RENDER);
			select_object(hits, selectBuf);
			select_object_flag = 0;
			mainloop();
		}
	}
	return;
}

void ToolLoadTable (void) {
	/* import Tool-Diameter from Tooltable */
	int n = 0;
	char tmp_str[1024];
	tools_max = 0;
	if (PARAMETER[P_TOOL_TABLE].vstr[0] != 0) {
		FILE *tt_fp;
		char *line = NULL;
		size_t len = 0;
		ssize_t read;
		tt_fp = fopen(PARAMETER[P_TOOL_TABLE].vstr, "r");
		if (tt_fp == NULL) {
			fprintf(stderr, "Can not open Tooltable-File: %s\n", PARAMETER[P_TOOL_TABLE].vstr);
			exit(EXIT_FAILURE);
		}
		tooltbl_diameters[0] = 1;
		n = 0;
		gtk_list_store_clear(ListStore[P_TOOL_SELECT]);
		snprintf(tmp_str, sizeof(tmp_str), "FREE");
		gtk_list_store_insert_with_values(ListStore[P_TOOL_SELECT], NULL, -1, 0, NULL, 1, tmp_str, -1);
		n++;
		while ((read = getline(&line, &len, tt_fp)) != -1) {
			if (strncmp(line, "T", 1) == 0) {
				char line2[2048];
				trimline(line2, 1024, line);
				int tooln = atoi(line2 + 1);
				double toold = atof(strstr(line2, " D") + 2);
				if (tooln > 0 && tooln < 100 && toold > 0.001) {
					tooltbl_diameters[tooln] = toold;
					tool_descr[tooln][0] = 0;
					if (strstr(line2, ";") > 0) {
						strcpy(tool_descr[tooln], strstr(line2, ";") + 1);
					}
					snprintf(tmp_str, sizeof(tmp_str), "#%i D%0.2fmm (%s)", tooln, tooltbl_diameters[tooln], tool_descr[tooln]);
					gtk_list_store_insert_with_values(ListStore[P_TOOL_SELECT], NULL, -1, 0, NULL, 1, tmp_str, -1);
					n++;
					tools_max++;
				}
			}
		}
		fclose(tt_fp);
	}
}

void LayerLoadList (void) {
}

void ArgsRead (int argc, char **argv) {
	int num = 0;
	if (argc < 2) {
//		SetupShowHelp();
//		exit(1);
	}
	PARAMETER[P_V_DXF].vstr[0] = 0;
	strcpy(PARAMETER[P_MFILE].vstr, "-");
	for (num = 1; num < argc; num++) {
		if (SetupArgCheck(argv[num], argv[num + 1]) == 1) {
			num++;
		} else if (strcmp(argv[num], "-h") == 0) {
			SetupShowHelp();
			exit(0);
		} else if (num != argc - 1) {
			fprintf(stderr, "### unknown argument: %s ###\n", argv[num]);
			SetupShowHelp();
			exit(1);
		} else {
			strcpy(PARAMETER[P_V_DXF].vstr, argv[argc - 1]);
		}
	}
}

void view_init_gl(void) {
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(glCanvas);
	GdkGLContext *glcontext = gtk_widget_get_gl_context(glCanvas);
	if (gdk_gl_drawable_gl_begin(gldrawable, glcontext)) {
		glViewport(0, 0, width, height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective((M_PI / 4) / M_PI * 180, (float)width / (float)height, 0.1, 1000.0);
		gluLookAt(0, 0, 6,  0, 0, 0,  0, 1, 0);
		glMatrixMode(GL_MODELVIEW);
		glEnable(GL_NORMALIZE);
		gdk_gl_drawable_gl_end(gldrawable);
	}
}

void view_draw (void) {
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(glCanvas);
	GdkGLContext *glcontext = gtk_widget_get_gl_context(glCanvas);
	if (gdk_gl_drawable_gl_begin(gldrawable, glcontext)) {
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ParameterUpdate();
		mainloop();
		if (gdk_gl_drawable_is_double_buffered (gldrawable)) {
			gdk_gl_drawable_swap_buffers(gldrawable);
		} else {
			glFlush();
		}
		gdk_gl_drawable_gl_end(gldrawable);
	}
}


void handler_destroy (GtkWidget *widget, gpointer data) {
	if (PARAMETER[P_O_AUTOSAVE].vint == 1) {
		SetupSave();
	}
	gtk_main_quit();
}

void handler_rotate_drawing (GtkWidget *widget, gpointer data) {
	int num;
	loading = 1;
	for (num = 0; num < line_last; num++) {
		if (myLINES[num].used == 1) {
			double tmp = myLINES[num].x1;
			myLINES[num].x1 = myLINES[num].y1;
			myLINES[num].y1 = size_x - tmp;
			tmp = myLINES[num].x2;
			myLINES[num].x2 = myLINES[num].y2;
			myLINES[num].y2 = size_x - tmp;
			tmp = myLINES[num].cx;
			myLINES[num].cx = myLINES[num].cy;
			myLINES[num].cy = size_x - tmp;
		}
	}
	init_objects();
	loading = 0;
}

void handler_flip_x_drawing (GtkWidget *widget, gpointer data) {
	int num;
	loading = 1;
	for (num = 0; num < line_last; num++) {
		if (myLINES[num].used == 1) {
			myLINES[num].x1 = size_x - myLINES[num].x1;
			myLINES[num].x2 = size_x - myLINES[num].x2;
			myLINES[num].cx = size_x - myLINES[num].cx;
		}
	}
	init_objects();
	loading = 0;
}

void handler_flip_y_drawing (GtkWidget *widget, gpointer data) {
	int num;
	loading = 1;
	for (num = 0; num < line_last; num++) {
		if (myLINES[num].used == 1) {
			myLINES[num].y1 = size_y - myLINES[num].y1;
			myLINES[num].y2 = size_y - myLINES[num].y2;
			myLINES[num].cy = size_y - myLINES[num].cy;
		}
	}
	init_objects();
	loading = 0;
}

void handler_reload_dxf (GtkWidget *widget, gpointer data) {
		gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "reloading dxf..."), "reloading dxf...");
		loading = 1;
#ifdef USE_G3D
		if (strstr(PARAMETER[P_V_DXF].vstr, ".dxf") > 0 || strstr(PARAMETER[P_V_DXF].vstr, ".DXF") > 0) {
			dxf_read(PARAMETER[P_V_DXF].vstr);
		} else {
			slice_3d(PARAMETER[P_V_DXF].vstr, 0.0);
		}
#else
		dxf_read(PARAMETER[P_V_DXF].vstr);
#endif
		init_objects();
		loading = 0;
		gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "reloading dxf...done"), "reloading dxf...done");
}

void handler_load_dxf (GtkWidget *widget, gpointer data) {
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new(_("Load Drawing"),
		GTK_WINDOW(window),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
	NULL);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

	GtkFileFilter *ffilter;
	ffilter = gtk_file_filter_new();
	gtk_file_filter_set_name(ffilter, _("DXF-Drawings"));
	gtk_file_filter_add_pattern(ffilter, "*.dxf");
	gtk_file_filter_add_pattern(ffilter, "*.DXF");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), ffilter);

#ifdef USE_G3D
	ffilter = gtk_file_filter_new();
	gtk_file_filter_set_name(ffilter, _("3D-Objects"));
	gtk_file_filter_add_pattern(ffilter, "*.obj");
	gtk_file_filter_add_pattern(ffilter, "*.OBJ");
	gtk_file_filter_add_pattern(ffilter, "*.lwo");
	gtk_file_filter_add_pattern(ffilter, "*.LWO");
	gtk_file_filter_add_pattern(ffilter, "*.ac3d");
	gtk_file_filter_add_pattern(ffilter, "*.AC3D");
	gtk_file_filter_add_pattern(ffilter, "*.stl");
	gtk_file_filter_add_pattern(ffilter, "*.STL");
	gtk_file_filter_add_pattern(ffilter, "*.3ds");
	gtk_file_filter_add_pattern(ffilter, "*.3DS");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), ffilter);
#endif

	if (PARAMETER[P_TOOL_TABLE].vstr[0] == 0) {
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), "./");
	} else {
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), PARAMETER[P_V_DXF].vstr);
	}
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		strcpy(PARAMETER[P_V_DXF].vstr, filename);
		gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "reading dxf..."), "reading dxf...");
		loading = 1;
#ifdef USE_G3D
		if (strstr(PARAMETER[P_V_DXF].vstr, ".dxf") > 0 || strstr(PARAMETER[P_V_DXF].vstr, ".DXF") > 0) {
			dxf_read(PARAMETER[P_V_DXF].vstr);
		} else {
			slice_3d(PARAMETER[P_V_DXF].vstr, 0.0);
		}
#else
		dxf_read(PARAMETER[P_V_DXF].vstr);
#endif
		init_objects();
		loading = 0;
		gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "reading dxf...done"), "reading dxf...done");
		g_free(filename);
	}
	gtk_widget_destroy(dialog);
}

void handler_load_preset (GtkWidget *widget, gpointer data) {
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new ("Load Preset",
		GTK_WINDOW(window),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
	NULL);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
	GtkFileFilter *ffilter;
	ffilter = gtk_file_filter_new();
	gtk_file_filter_set_name(ffilter, "Preset");
	gtk_file_filter_add_pattern(ffilter, "*.preset");
	gtk_file_filter_add_pattern(ffilter, "*.PRESET");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), ffilter);
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), "./");
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "reading preset..."), "reading preset...");
		SetupLoadPreset(filename);
		gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "reading preset...done"), "reading preset...done");
		g_free(filename);
	}
	gtk_widget_destroy(dialog);
}

void handler_save_preset (GtkWidget *widget, gpointer data) {
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new ("Save Preset",
		GTK_WINDOW(window),
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
	NULL);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

	GtkFileFilter *ffilter;
	ffilter = gtk_file_filter_new();
	gtk_file_filter_set_name(ffilter, "Preset");
	gtk_file_filter_add_pattern(ffilter, "*.preset");
	gtk_file_filter_add_pattern(ffilter, "*.PRESET");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), ffilter);
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), "./");
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "saving Preset..."), "saving Preset...");
		SetupSavePreset(filename);
		gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "saving Preset...done"), "saving Preset...done");
		g_free(filename);
	}
	gtk_widget_destroy(dialog);
}

char *suffix_remove (char *mystr) {
	char *retstr;
	char *lastdot;
	if (mystr == NULL) {
		return NULL;
	}
	if ((retstr = malloc (strlen (mystr) + 1)) == NULL) {
        	return NULL;
	}
	strcpy(retstr, mystr);
	lastdot = strrchr(retstr, '.');
	if (lastdot != NULL) {
		*lastdot = '\0';
	}
	return retstr;
}

void handler_save_lua (GtkWidget *widget, gpointer data) {
	gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "saving lua..."), "saving lua...");
	postcam_save_source(program_path, postcam_plugins[PARAMETER[P_H_POST].vint]);
	postcam_plugin = -1;
	gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "saving lua...done"), "saving lua...done");
}

void handler_save_gcode_as (GtkWidget *widget, gpointer data) {
	char ext_str[1024];
	GtkWidget *dialog;
	snprintf(ext_str, 1024, "%s (.%s)", _("Save Output As.."), output_extension);
	dialog = gtk_file_chooser_dialog_new (ext_str,
		GTK_WINDOW(window),
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
	NULL);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER (dialog), TRUE);

	GtkFileFilter *ffilter;
	ffilter = gtk_file_filter_new();
	gtk_file_filter_set_name(ffilter, output_extension);
	snprintf(ext_str, 1024, "*.%s", output_extension);
	gtk_file_filter_add_pattern(ffilter, ext_str);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), ffilter);
	if (PARAMETER[P_MFILE].vstr[0] == 0) {
		char dir[2048];
		strcpy(dir, PARAMETER[P_V_DXF].vstr);
		dirname(dir);
		char file[2048];
		strcpy(file, basename(PARAMETER[P_V_DXF].vstr));
		char *file_nosuffix = suffix_remove(file);
		file_nosuffix = realloc(file_nosuffix, strlen(file_nosuffix) + 5);
		strcat(file_nosuffix, ".");
		strcat(file_nosuffix, output_extension);
		if (strstr(PARAMETER[P_V_DXF].vstr, "/") > 0) {
			gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), dir);
		} else {
			gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), "./");
		}

		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), file_nosuffix);
		free(file_nosuffix);
	} else {
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER (dialog), PARAMETER[P_MFILE].vstr);
	}

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		strcpy(PARAMETER[P_MFILE].vstr, filename);
		g_free(filename);
		save_gcode = 1;
		gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "saving g-code..."), "saving g-code...");
	}
	gtk_widget_destroy(dialog);
}

void handler_save_gcode (GtkWidget *widget, gpointer data) {
	if (PARAMETER[P_MFILE].vstr[0] == 0) {
                handler_save_gcode_as(widget, data);
        } else {
                save_gcode = 1;
                gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "saving g-code..."), "saving g-code...");
        }
}

void handler_load_tooltable (GtkWidget *widget, gpointer data) {
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new (_("Load Tooltable"),
		GTK_WINDOW(window),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
	NULL);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

	GtkFileFilter *ffilter;
	ffilter = gtk_file_filter_new();
	gtk_file_filter_set_name(ffilter, _("Tooltable"));
	gtk_file_filter_add_pattern(ffilter, "*.tbl");
	gtk_file_filter_add_pattern(ffilter, "*.TBL");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), ffilter);

	if (PARAMETER[P_TOOL_TABLE].vstr[0] == 0) {
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), "./");
	} else {
		gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), PARAMETER[P_TOOL_TABLE].vstr);
	}
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		strcpy(PARAMETER[P_TOOL_TABLE].vstr, filename);
		gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "loading tooltable..."), "loading tooltable...");
		ToolLoadTable();
		gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "loading tooltable...done"), "loading tooltable...done");
		g_free(filename);
	}
	gtk_widget_destroy(dialog);
}

void handler_save_setup (GtkWidget *widget, gpointer data) {
	gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "saving setup..."), "saving setup...");
	SetupSave();
	gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "saving setup...done"), "saving setup...done");
}

void handler_about (GtkWidget *widget, gpointer data) {
	GtkWidget *dialog = gtk_dialog_new();
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(window));
	gtk_window_set_title(GTK_WINDOW(dialog), _("About"));
	gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_QUIT, 1);
	char tmp_str[2048];
	snprintf(tmp_str, sizeof(tmp_str), "%s\n\nCopyright by %s\n%s\n\n%s\n\nVersion: %s\n",about1, author1, author2, website, VERSION);
	GtkWidget *label = gtk_label_new(tmp_str);
	gtk_widget_modify_font(label, pango_font_description_from_string("Tahoma 18"));

	char iconfile[PATH_MAX];
	snprintf(iconfile, PATH_MAX, "%s%s", program_path, "icons/logo.png");
	GtkWidget *image = gtk_image_new_from_file(iconfile);
	GtkWidget *box = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	gtk_box_pack_start(GTK_BOX(box), image, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 0);
	gtk_widget_show_all(dialog);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(dialog);
}

void handler_draw (GtkWidget *w, GdkEventExpose* e, void *v) {
}

void handler_scrollwheel(GtkWidget * w, GdkEvent* e, GtkWidget *l) {
	if (e->scroll.direction == GDK_SCROLL_UP) {
		PARAMETER[P_V_ZOOM].vfloat += 0.1;
	} else if (e->scroll.direction == GDK_SCROLL_DOWN) {
		PARAMETER[P_V_ZOOM].vfloat -= 0.1;
	}
}

void handler_button_press (GtkWidget *w, GdkEventButton* e, void *v) {
//	printf("button_press x=%g y=%g b=%d state=%d\n", e->x, e->y, e->button, e->state);
	int mouseX = e->x;
	int mouseY = e->y;
	int state = e->state;
	int button = e->button;;
	if (button == 4 && state == 0) {
		PARAMETER[P_V_ZOOM].vfloat += 0.05;
	} else if (button == 5 && state == 0) {
		PARAMETER[P_V_ZOOM].vfloat -= 0.05;
	} else if (button == 1) {
		if (state == 0) {
			select_object_x = mouseX;
			select_object_y = mouseY;
			select_object_flag = 2;
			last_mouse_x = mouseX - PARAMETER[P_V_TRANSX].vint * 2;
			last_mouse_y = mouseY - PARAMETER[P_V_TRANSY].vint * -2;
			last_mouse_button = button;
			last_mouse_state = state;
		} else {
			last_mouse_button = button;
			last_mouse_state = state;
		}
	} else if (button == 2) {
		if (state == 0) {
			last_mouse_x = mouseX - (int)(PARAMETER[P_V_ROTY].vfloat * 5.0);
			last_mouse_y = mouseY - (int)(PARAMETER[P_V_ROTX].vfloat * 5.0);
			last_mouse_button = button;
			last_mouse_state = state;
		} else {
			last_mouse_button = button;
			last_mouse_state = state;
		}
	} else if (button == 3) {
		if (state == 0) {
			last_mouse_x = mouseX - (int)(PARAMETER[P_V_ROTZ].vfloat * 5.0);;
			last_mouse_y = mouseY - (int)(PARAMETER[P_V_ZOOM].vfloat * 100.0);
			last_mouse_button = button;
			last_mouse_state = state;
		} else {
			last_mouse_button = button;
			last_mouse_state = state;
		}
	}
}

void handler_button_release (GtkWidget *w, GdkEventButton* e, void *v) {
//	printf("button_release x=%g y=%g b=%d state=%d\n", e->x, e->y, e->button, e->state);
	last_mouse_button = -1;	
	if (select_object_flag == 2) {
		select_object_flag = 1;
	} else {
		select_object_flag = 0;
	}
}

void handler_motion (GtkWidget *w, GdkEventMotion* e, void *v) {
//	printf("button_motion x=%g y=%g state=%d\n", e->x, e->y, e->state);
	int mouseX = e->x;
	int mouseY = e->y;
	select_object_flag = 0;
	if (last_mouse_button == 1) {
		PARAMETER[P_V_TRANSX].vint = (mouseX - last_mouse_x) / 2;
		PARAMETER[P_V_TRANSY].vint = (mouseY - last_mouse_y) / -2;
	} else if (last_mouse_button == 2) {
		PARAMETER[P_V_ROTY].vfloat = (float)(mouseX - last_mouse_x) / 5.0;
		PARAMETER[P_V_ROTX].vfloat = (float)(mouseY - last_mouse_y) / 5.0;
	} else if (last_mouse_button == 3) {
		PARAMETER[P_V_ROTZ].vfloat = (float)(mouseX - last_mouse_x) / 5.0;
		PARAMETER[P_V_ZOOM].vfloat = (float)(mouseY - last_mouse_y) / 100;
	} else {
		last_mouse_x = mouseX;
		last_mouse_y = mouseY;
	}
}

void handler_keypress (GtkWidget *w, GdkEventKey* e, void *v) {
//	printf("key_press state=%d key=%s\n", e->state, e->string);
}

void handler_configure (GtkWidget *w, GdkEventConfigure* e, void *v) {
//	printf("configure width=%d height=%d ratio=%g\n", e->width, e->height, e->width/(float)e->height);
	width = e->width;
	height = e->height;
	need_init = 1;
}

int handler_periodic_action (gpointer d) {
	while ( gtk_events_pending() ) {
		gtk_main_iteration();
	}
	if (need_init == 1) {
		need_init = 0;
		view_init_gl();
	}
	view_draw();
	return(1);
}

GtkWidget *create_gl () {
	static GdkGLConfig *glconfig = NULL;
	static GdkGLContext *glcontext = NULL;
	GtkWidget *area;
	if (glconfig == NULL) {
		glconfig = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE);
		if (glconfig == NULL) {
			glconfig = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH);
			if (glconfig == NULL) {
				exit(1);
			}
		}
	}
	area = gtk_drawing_area_new();
	gtk_widget_set_gl_capability(area, glconfig, glcontext, TRUE, GDK_GL_RGBA_TYPE); 
	gtk_widget_set_events(GTK_WIDGET(area)
		,GDK_POINTER_MOTION_MASK 
		|GDK_BUTTON_PRESS_MASK 
		|GDK_BUTTON_RELEASE_MASK
		|GDK_ENTER_NOTIFY_MASK
		|GDK_KEY_PRESS_MASK
		|GDK_KEY_RELEASE_MASK
		|GDK_EXPOSURE_MASK
	);
	gtk_signal_connect(GTK_OBJECT(area), "enter-notify-event", GTK_SIGNAL_FUNC(gtk_widget_grab_focus), NULL);
	return(area);
}

void ParameterChanged (GtkWidget *widget, gpointer data) {
	if (loading == 1) {
		return;
	}
	int n = (int)data;
//	printf("UPDATED(%i): %s\n", n, PARAMETER[n].name);
	if (PARAMETER[n].type == T_FLOAT) {
		PARAMETER[n].vfloat = gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(widget));
	} else if (PARAMETER[n].type == T_DOUBLE) {
		PARAMETER[n].vdouble = (double)gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(widget));
	} else if (PARAMETER[n].type == T_INT) {
		PARAMETER[n].vint = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	} else if (PARAMETER[n].type == T_SELECT) {
		PARAMETER[n].vint = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
	} else if (PARAMETER[n].type == T_BOOL) {
		PARAMETER[n].vint = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	} else if (PARAMETER[n].type == T_STRING) {
		strcpy(PARAMETER[n].vstr, (char *)gtk_entry_get_text(GTK_ENTRY(widget)));
	} else if (PARAMETER[n].type == T_FILE) {
		strcpy(PARAMETER[n].vstr, (char *)gtk_entry_get_text(GTK_ENTRY(widget)));
	}

	if (n == P_O_SELECT && PARAMETER[P_O_SELECT].vint != -1) {
		int object_num = PARAMETER[P_O_SELECT].vint;
		PARAMETER[P_O_USE].vint = myOBJECTS[object_num].use;
		PARAMETER[P_O_FORCE].vint = myOBJECTS[object_num].force;
		PARAMETER[P_O_CLIMB].vint = myOBJECTS[object_num].climb;
		PARAMETER[P_O_OFFSET].vint = myOBJECTS[object_num].offset;
		PARAMETER[P_O_OVERCUT].vint = myOBJECTS[object_num].overcut;
		PARAMETER[P_O_POCKET].vint = myOBJECTS[object_num].pocket;
		PARAMETER[P_O_LASER].vint = myOBJECTS[object_num].laser;
		PARAMETER[P_O_DEPTH].vdouble = myOBJECTS[object_num].depth;
		PARAMETER[P_O_ORDER].vint = myOBJECTS[object_num].order;
		PARAMETER[P_O_TABS].vint = myOBJECTS[object_num].tabs;
	} else if (n > P_O_SELECT && PARAMETER[P_O_SELECT].vint != -1) {
		int object_num = PARAMETER[P_O_SELECT].vint;
		myOBJECTS[object_num].use = PARAMETER[P_O_USE].vint;
		myOBJECTS[object_num].force = PARAMETER[P_O_FORCE].vint;
		myOBJECTS[object_num].climb = PARAMETER[P_O_CLIMB].vint;
		myOBJECTS[object_num].offset = PARAMETER[P_O_OFFSET].vint;
		myOBJECTS[object_num].overcut = PARAMETER[P_O_OVERCUT].vint;
		myOBJECTS[object_num].pocket = PARAMETER[P_O_POCKET].vint;
		myOBJECTS[object_num].laser = PARAMETER[P_O_LASER].vint;
		myOBJECTS[object_num].depth = PARAMETER[P_O_DEPTH].vdouble;
		myOBJECTS[object_num].order = PARAMETER[P_O_ORDER].vint;
		myOBJECTS[object_num].tabs = PARAMETER[P_O_TABS].vint;
	}

	if (n == P_MAT_SELECT) {
		int mat_num = PARAMETER[P_MAT_SELECT].vint;
		PARAMETER[P_MAT_CUTSPEED].vint = Material[mat_num].vc;
		PARAMETER[P_MAT_FEEDFLUTE4].vdouble = Material[mat_num].fz[FZ_FEEDFLUTE4];
		PARAMETER[P_MAT_FEEDFLUTE8].vdouble = Material[mat_num].fz[FZ_FEEDFLUTE8];
		PARAMETER[P_MAT_FEEDFLUTE12].vdouble = Material[mat_num].fz[FZ_FEEDFLUTE12];
		strcpy(PARAMETER[P_MAT_TEXTURE].vstr, Material[mat_num].texture);
	}

	if (n == P_O_TOLERANCE) {
		loading = 1;
		init_objects();
		loading = 0;
	}
	if (n != P_O_PARAVIEW && strncmp(PARAMETER[n].name, "Translate", 9) != 0 && strncmp(PARAMETER[n].name, "Rotate", 6) != 0 && strncmp(PARAMETER[n].name, "Zoom", 4) != 0) {
		update_post = 1;
	}

	if (n == P_O_PARAVIEW) {
		gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "please save setup and restart cammill !"), "please save setup and restart cammill !");
	}

}

void ParameterUpdate (void) {
	char path[1024];
	char value2[1024];
	int n = 0;
	for (n = 0; n < P_LAST; n++) {
		if (PARAMETER[n].type == T_FLOAT) {
			if (PARAMETER[n].vfloat != gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(ParamValue[n]))) {
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(ParamValue[n]), PARAMETER[n].vfloat);
			}
		} else if (PARAMETER[n].type == T_DOUBLE) {
			if (PARAMETER[n].vdouble != gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(ParamValue[n]))) {
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(ParamValue[n]), PARAMETER[n].vdouble);
			}
		} else if (PARAMETER[n].type == T_INT) {
			if (PARAMETER[n].vint != gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ParamValue[n]))) {
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(ParamValue[n]), PARAMETER[n].vint);
			}
		} else if (PARAMETER[n].type == T_SELECT) {
			if (PARAMETER[n].vint != gtk_combo_box_get_active(GTK_COMBO_BOX(ParamValue[n]))) {
				gtk_combo_box_set_active(GTK_COMBO_BOX(ParamValue[n]), PARAMETER[n].vint);
			}
		} else if (PARAMETER[n].type == T_BOOL) {
			if (PARAMETER[n].vint != gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ParamValue[n]))) {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ParamValue[n]), PARAMETER[n].vint);
			}
		} else if (PARAMETER[n].type == T_STRING) {
			gtk_entry_set_text(GTK_ENTRY(ParamValue[n]), PARAMETER[n].vstr);
		} else if (PARAMETER[n].type == T_FILE) {
			gtk_entry_set_text(GTK_ENTRY(ParamValue[n]), PARAMETER[n].vstr);
		} else {
			continue;
		}
	}
	for (n = 0; n < P_LAST; n++) {
		snprintf(path, sizeof(path), "0:%i:%i", PARAMETER[n].l1, PARAMETER[n].l2);
		if (PARAMETER[n].type == T_FLOAT) {
			snprintf(value2, sizeof(value2), "%f", PARAMETER[n].vfloat);
		} else if (PARAMETER[n].type == T_DOUBLE) {
			snprintf(value2, sizeof(value2), "%f", PARAMETER[n].vdouble);
		} else if (PARAMETER[n].type == T_INT) {
			snprintf(value2, sizeof(value2), "%i", PARAMETER[n].vint);
		} else if (PARAMETER[n].type == T_SELECT) {
			snprintf(value2, sizeof(value2), "%i", PARAMETER[n].vint);
		} else if (PARAMETER[n].type == T_BOOL) {
			snprintf(value2, sizeof(value2), "%i", PARAMETER[n].vint);
		} else if (PARAMETER[n].type == T_STRING) {
			snprintf(value2, sizeof(value2), "%s", PARAMETER[n].vstr);
		} else if (PARAMETER[n].type == T_FILE) {
			snprintf(value2, sizeof(value2), "%s", PARAMETER[n].vstr);
		} else {
			continue;
		}
	}
}

GdkPixbuf *create_pixbuf(const gchar * filename) {
	GdkPixbuf *pixbuf;
	GError *error = NULL;
	pixbuf = gdk_pixbuf_new_from_file(filename, &error);
	if(!pixbuf) {
		fprintf(stderr, "%s\n", error->message);
		g_error_free(error);
	}
	return pixbuf;
}

#ifdef USE_WEBKIT
void handler_webkit_back (GtkWidget *widget, gpointer data) {
	webkit_web_view_go_back(WEBKIT_WEB_VIEW(WebKit));
}

void handler_webkit_home (GtkWidget *widget, gpointer data) {
	webkit_web_view_open(WEBKIT_WEB_VIEW(WebKit), "file:///usr/src/cammill/index.html");
}

void handler_webkit_forward (GtkWidget *widget, gpointer data) {
	webkit_web_view_go_forward(WEBKIT_WEB_VIEW(WebKit));
}
#endif

void create_gui () {
	GtkWidget *vbox;

	glCanvas = create_gl();
	gtk_widget_set_usize(GTK_WIDGET(glCanvas), 800, 600);
	gtk_signal_connect(GTK_OBJECT(glCanvas), "expose_event", GTK_SIGNAL_FUNC(handler_draw), NULL);  
	gtk_signal_connect(GTK_OBJECT(glCanvas), "button_press_event", GTK_SIGNAL_FUNC(handler_button_press), NULL);  
	gtk_signal_connect(GTK_OBJECT(glCanvas), "button_release_event", GTK_SIGNAL_FUNC(handler_button_release), NULL);  
	gtk_signal_connect(GTK_OBJECT(glCanvas), "configure_event", GTK_SIGNAL_FUNC(handler_configure), NULL);  
	gtk_signal_connect(GTK_OBJECT(glCanvas), "motion_notify_event", GTK_SIGNAL_FUNC(handler_motion), NULL);  
	gtk_signal_connect(GTK_OBJECT(glCanvas), "key_press_event", GTK_SIGNAL_FUNC(handler_keypress), NULL);  
	gtk_signal_connect(GTK_OBJECT(glCanvas), "scroll-event", GTK_SIGNAL_FUNC(handler_scrollwheel), NULL);  

	// top-menu
	GtkWidget *MenuBar = gtk_menu_bar_new();
	GtkWidget *MenuItem;
	GtkWidget *FileMenu = gtk_menu_item_new_with_mnemonic(_("_File"));
	GtkWidget *FileMenuList = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(FileMenu), FileMenuList);
	gtk_menu_bar_append(GTK_MENU_BAR(MenuBar), FileMenu);

        GtkAccelGroup *accel_group = gtk_accel_group_new();

		MenuItem = gtk_menu_item_new_with_mnemonic(_("_Load DXF"));
		gtk_menu_append(GTK_MENU(FileMenuList), MenuItem);
		gtk_signal_connect(GTK_OBJECT(MenuItem), "activate", GTK_SIGNAL_FUNC(handler_load_dxf), NULL);
                gtk_widget_add_accelerator(MenuItem, "activate", accel_group,
                                           GDK_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
                
		MenuItem = gtk_menu_item_new_with_mnemonic(_("_Reload DXF"));
		gtk_menu_append(GTK_MENU(FileMenuList), MenuItem);
		gtk_signal_connect(GTK_OBJECT(MenuItem), "activate", GTK_SIGNAL_FUNC(handler_reload_dxf), NULL);
                gtk_widget_add_accelerator(MenuItem, "activate", accel_group,
                                           GDK_r, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
                
		MenuItem = gtk_menu_item_new_with_mnemonic(_("_Save Output As.."));
		gtk_menu_append(GTK_MENU(FileMenuList), MenuItem);
		gtk_signal_connect(GTK_OBJECT(MenuItem), "activate", GTK_SIGNAL_FUNC(handler_save_gcode_as), NULL);
                gtk_widget_add_accelerator(MenuItem, "activate", accel_group,
                                           GDK_s, GDK_CONTROL_MASK | GDK_SHIFT_MASK, GTK_ACCEL_VISIBLE);

		MenuItem = gtk_menu_item_new_with_mnemonic(_("_Save Output"));
		gtk_menu_append(GTK_MENU(FileMenuList), MenuItem);
		gtk_signal_connect(GTK_OBJECT(MenuItem), "activate", GTK_SIGNAL_FUNC(handler_save_gcode), NULL);
                gtk_widget_add_accelerator(MenuItem, "activate", accel_group,
                                           GDK_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

		MenuItem = gtk_menu_item_new_with_label(_("Load Tooltable"));
		gtk_menu_append(GTK_MENU(FileMenuList), MenuItem);
		gtk_signal_connect(GTK_OBJECT(MenuItem), "activate", GTK_SIGNAL_FUNC(handler_load_tooltable), NULL);

		MenuItem = gtk_menu_item_new_with_label(_("Load Preset"));
		gtk_menu_append(GTK_MENU(FileMenuList), MenuItem);
		gtk_signal_connect(GTK_OBJECT(MenuItem), "activate", GTK_SIGNAL_FUNC(handler_load_preset), NULL);

		MenuItem = gtk_menu_item_new_with_label(_("Save Preset"));
		gtk_menu_append(GTK_MENU(FileMenuList), MenuItem);
		gtk_signal_connect(GTK_OBJECT(MenuItem), "activate", GTK_SIGNAL_FUNC(handler_save_preset), NULL);

		MenuItem = gtk_menu_item_new_with_label(_("Save Setup"));
		gtk_menu_append(GTK_MENU(FileMenuList), MenuItem);
		gtk_signal_connect(GTK_OBJECT(MenuItem), "activate", GTK_SIGNAL_FUNC(handler_save_setup), NULL);

		MenuItem = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, accel_group);
		gtk_menu_append(GTK_MENU(FileMenuList), MenuItem);
		gtk_signal_connect(GTK_OBJECT(MenuItem), "activate", GTK_SIGNAL_FUNC(handler_destroy), NULL);

	gtk_widget_add_accelerator(MenuItem, "activate", accel_group, GDK_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
                

	GtkWidget *HelpMenu = gtk_menu_item_new_with_mnemonic(_("_Help"));
	GtkWidget *HelpMenuList = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(HelpMenu), HelpMenuList);
	gtk_menu_bar_append(GTK_MENU_BAR(MenuBar), HelpMenu);

	MenuItem = gtk_menu_item_new_with_label(_("About"));
	gtk_menu_append(GTK_MENU(HelpMenuList), MenuItem);
	gtk_signal_connect(GTK_OBJECT(MenuItem), "activate", GTK_SIGNAL_FUNC(handler_about), NULL);

	GtkWidget *ToolBar = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(ToolBar), GTK_TOOLBAR_ICONS);

	GtkToolItem *ToolItemLoadDXF = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
	gtk_tool_item_set_tooltip_text(ToolItemLoadDXF, _("Load DXF"));
	gtk_toolbar_insert(GTK_TOOLBAR(ToolBar), ToolItemLoadDXF, -1);
	g_signal_connect(G_OBJECT(ToolItemLoadDXF), "clicked", GTK_SIGNAL_FUNC(handler_load_dxf), NULL);

	GtkToolItem *ToolItemReloadDXF = gtk_tool_button_new_from_stock(GTK_STOCK_REFRESH);
	gtk_tool_item_set_tooltip_text(ToolItemReloadDXF, _("Reload DXF"));
	gtk_toolbar_insert(GTK_TOOLBAR(ToolBar), ToolItemReloadDXF, -1);
	g_signal_connect(G_OBJECT(ToolItemReloadDXF), "clicked", GTK_SIGNAL_FUNC(handler_reload_dxf), NULL);

	GtkToolItem *ToolItemSaveAsGcode = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE_AS);
	gtk_tool_item_set_tooltip_text(ToolItemSaveAsGcode, _("Save Output As.."));
	gtk_toolbar_insert(GTK_TOOLBAR(ToolBar), ToolItemSaveAsGcode, -1);
	g_signal_connect(G_OBJECT(ToolItemSaveAsGcode), "clicked", GTK_SIGNAL_FUNC(handler_save_gcode_as), NULL);

	GtkToolItem *ToolItemSaveGcode = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE);
	gtk_tool_item_set_tooltip_text(ToolItemSaveGcode, _("Save Output"));
	gtk_toolbar_insert(GTK_TOOLBAR(ToolBar), ToolItemSaveGcode, -1);
	g_signal_connect(G_OBJECT(ToolItemSaveGcode), "clicked", GTK_SIGNAL_FUNC(handler_save_gcode), NULL);

	GtkToolItem *ToolItemSaveSetup = gtk_tool_button_new_from_stock(GTK_STOCK_PROPERTIES);
	gtk_tool_item_set_tooltip_text(ToolItemSaveSetup, _("Save Setup"));
	gtk_toolbar_insert(GTK_TOOLBAR(ToolBar), ToolItemSaveSetup, -1);
	g_signal_connect(G_OBJECT(ToolItemSaveSetup), "clicked", GTK_SIGNAL_FUNC(handler_save_setup), NULL);

	GtkToolItem *ToolItemSep = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(ToolBar), ToolItemSep, -1); 

	GtkToolItem *ToolItemLoadPreset = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
	gtk_tool_item_set_tooltip_text(ToolItemLoadPreset, _("Load Preset"));
	gtk_toolbar_insert(GTK_TOOLBAR(ToolBar), ToolItemLoadPreset, -1);
	g_signal_connect(G_OBJECT(ToolItemLoadPreset), "clicked", GTK_SIGNAL_FUNC(handler_load_preset), NULL);

	GtkToolItem *ToolItemSavePreset = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE_AS);
	gtk_tool_item_set_tooltip_text(ToolItemSavePreset, _("Save Preset"));
	gtk_toolbar_insert(GTK_TOOLBAR(ToolBar), ToolItemSavePreset, -1);
	g_signal_connect(G_OBJECT(ToolItemSavePreset), "clicked", GTK_SIGNAL_FUNC(handler_save_preset), NULL);


	GtkToolItem *ToolItemSep1 = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(ToolBar), ToolItemSep1, -1); 

	GtkToolItem *TB_Rotate;
	GtkIconInfo *icon_Rotate = gtk_icon_theme_lookup_icon(gtk_icon_theme_get_default(), "object-rotate-right", 24, 0);
	TB_Rotate = gtk_tool_button_new(gtk_image_new_from_file(gtk_icon_info_get_filename(icon_Rotate)), "Rotate");
	gtk_tool_item_set_tooltip_text(TB_Rotate, "Rotate 90°");
	gtk_toolbar_insert(GTK_TOOLBAR(ToolBar), TB_Rotate, -1);
	g_signal_connect(G_OBJECT(TB_Rotate), "clicked", GTK_SIGNAL_FUNC(handler_rotate_drawing), NULL);

	GtkToolItem *TB_FlipX;
	GtkIconInfo *icon_FlipX = gtk_icon_theme_lookup_icon(gtk_icon_theme_get_default(), "object-flip-horizontal", 24, 0);
	TB_FlipX = gtk_tool_button_new(gtk_image_new_from_file(gtk_icon_info_get_filename(icon_FlipX)), "FlipX");
	gtk_tool_item_set_tooltip_text(TB_FlipX, "Flip X");
	gtk_toolbar_insert(GTK_TOOLBAR(ToolBar), TB_FlipX, -1);
	g_signal_connect(G_OBJECT(TB_FlipX), "clicked", GTK_SIGNAL_FUNC(handler_flip_x_drawing), NULL);

	GtkToolItem *TB_FlipY;
	GtkIconInfo *icon_FlipY = gtk_icon_theme_lookup_icon(gtk_icon_theme_get_default(), "object-flip-vertical", 24, 0);
	TB_FlipY = gtk_tool_button_new(gtk_image_new_from_file(gtk_icon_info_get_filename(icon_FlipY)), "FlipY");
	gtk_tool_item_set_tooltip_text(TB_FlipY, "Flip Y");
	gtk_toolbar_insert(GTK_TOOLBAR(ToolBar), TB_FlipY, -1);
	g_signal_connect(G_OBJECT(TB_FlipY), "clicked", GTK_SIGNAL_FUNC(handler_flip_y_drawing), NULL);

	GtkToolItem *ToolItemSep2 = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(ToolBar), ToolItemSep2, -1); 


	GtkWidget *NbBox;
	int ViewNum = 0;
	int ToolNum = 0;
	int MillingNum = 0;
	int HoldingNum = 0;
	int RotaryNum = 0;
	int TangencialNum = 0;
	int MachineNum = 0;
	int MaterialNum = 0;
	int ObjectsNum = 0;
	int MiscNum = 0;
	GtkWidget *ViewBox = gtk_vbox_new(0, 0);
	GtkWidget *ToolBox = gtk_vbox_new(0, 0);
	GtkWidget *MillingBox = gtk_vbox_new(0, 0);
	GtkWidget *HoldingBox = gtk_vbox_new(0, 0);
	GtkWidget *RotaryBox = gtk_vbox_new(0, 0);
	GtkWidget *TangencialBox = gtk_vbox_new(0, 0);
	GtkWidget *MachineBox = gtk_vbox_new(0, 0);
	GtkWidget *MaterialBox = gtk_vbox_new(0, 0);
	GtkWidget *ObjectsBox = gtk_vbox_new(0, 0);
	GtkWidget *MiscBox = gtk_vbox_new(0, 0);
	if (PARAMETER[P_O_PARAVIEW].vint == 1) {
		NbBox = gtk_table_new(2, 2, FALSE);
		GtkWidget *notebook = gtk_notebook_new();
		gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);
		gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
		gtk_table_attach_defaults(GTK_TABLE(NbBox), notebook, 0, 1, 0, 1);
		GtkWidget *ViewLabel = gtk_label_new(_("View"));
		GtkWidget *ToolLabel = gtk_label_new(_("Tool"));
		GtkWidget *MillingLabel = gtk_label_new(_("Milling"));
		GtkWidget *HoldingLabel = gtk_label_new(_("Tabs"));
		GtkWidget *RotaryLabel = gtk_label_new(_("Rotary"));
		GtkWidget *TangencialLabel = gtk_label_new(_("Tangencial"));
		GtkWidget *MachineLabel = gtk_label_new(_("Machine"));
		GtkWidget *MaterialLabel = gtk_label_new(_("Material"));
		GtkWidget *ObjectsLabel = gtk_label_new(_("Objects"));
		GtkWidget *MiscLabel = gtk_label_new(_("Misc"));
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook), ViewBox, ViewLabel);
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook), ToolBox, ToolLabel);
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook), MillingBox, MillingLabel);
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook), HoldingBox, HoldingLabel);
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook), RotaryBox, RotaryLabel);
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook), TangencialBox, TangencialLabel);
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook), MachineBox, MachineLabel);
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook), MaterialBox, MaterialLabel);
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook), ObjectsBox, ObjectsLabel);
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook), MiscBox, MiscLabel);
	} else {
		GtkWidget *ExpanderBox = gtk_vbox_new(0, 0);
		ViewExpander = gtk_expander_new(_("View"));
		ToolExpander = gtk_expander_new(_("Tool"));
		MillingExpander = gtk_expander_new(_("Milling"));
		TabsExpander = gtk_expander_new(_("Tabs"));
		RotaryExpander = gtk_expander_new(_("Rotary"));
		TangencialExpander = gtk_expander_new(_("Tangencial"));
		MachineExpander = gtk_expander_new(_("Machine"));
		MaterialExpander = gtk_expander_new(_("Material"));
		ObjectsExpander = gtk_expander_new(_("Objects"));
		MiscExpander = gtk_expander_new(_("Misc"));
		gtk_expander_set_expanded(GTK_EXPANDER(ViewExpander), ExpanderStat[0]);
		gtk_expander_set_expanded(GTK_EXPANDER(ToolExpander), ExpanderStat[1]);
		gtk_expander_set_expanded(GTK_EXPANDER(MillingExpander), ExpanderStat[2]);
		gtk_expander_set_expanded(GTK_EXPANDER(TabsExpander), ExpanderStat[3]);
		gtk_expander_set_expanded(GTK_EXPANDER(RotaryExpander), ExpanderStat[4]);
		gtk_expander_set_expanded(GTK_EXPANDER(TangencialExpander), ExpanderStat[5]);
		gtk_expander_set_expanded(GTK_EXPANDER(MachineExpander), ExpanderStat[6]);
		gtk_expander_set_expanded(GTK_EXPANDER(MaterialExpander), ExpanderStat[7]);
		gtk_expander_set_expanded(GTK_EXPANDER(ObjectsExpander), ExpanderStat[8]);
		gtk_expander_set_expanded(GTK_EXPANDER(MiscExpander), ExpanderStat[9]);
		GtkWidget *ViewBoxFrame = gtk_frame_new("");
		GtkWidget *ToolBoxFrame = gtk_frame_new("");
		GtkWidget *MillingBoxFrame = gtk_frame_new("");
		GtkWidget *TabsBoxFrame = gtk_frame_new("");
		GtkWidget *RotaryBoxFrame = gtk_frame_new("");
		GtkWidget *TangencialBoxFrame = gtk_frame_new("");
		GtkWidget *MachineBoxFrame = gtk_frame_new("");
		GtkWidget *MaterialBoxFrame = gtk_frame_new("");
		GtkWidget *ObjectsBoxFrame = gtk_frame_new("");
		GtkWidget *MiscBoxFrame = gtk_frame_new("");
		gtk_container_add(GTK_CONTAINER(ViewBoxFrame), ViewExpander);
		gtk_container_add(GTK_CONTAINER(ToolBoxFrame), ToolExpander);
		gtk_container_add(GTK_CONTAINER(MillingBoxFrame), MillingExpander);
		gtk_container_add(GTK_CONTAINER(TabsBoxFrame), TabsExpander);
		gtk_container_add(GTK_CONTAINER(RotaryBoxFrame), RotaryExpander);
		gtk_container_add(GTK_CONTAINER(TangencialBoxFrame), TangencialExpander);
		gtk_container_add(GTK_CONTAINER(MachineBoxFrame), MachineExpander);
		gtk_container_add(GTK_CONTAINER(MaterialBoxFrame), MaterialExpander);
		gtk_container_add(GTK_CONTAINER(ObjectsBoxFrame), ObjectsExpander);
		gtk_container_add(GTK_CONTAINER(MiscBoxFrame), MiscExpander);
		gtk_box_pack_start(GTK_BOX(ExpanderBox), ViewBoxFrame, 0, 1, 0);
		gtk_box_pack_start(GTK_BOX(ExpanderBox), ToolBoxFrame, 0, 1, 0);
		gtk_box_pack_start(GTK_BOX(ExpanderBox), MillingBoxFrame, 0, 1, 0);
		gtk_box_pack_start(GTK_BOX(ExpanderBox), TabsBoxFrame, 0, 1, 0);
		gtk_box_pack_start(GTK_BOX(ExpanderBox), RotaryBoxFrame, 0, 1, 0);
		gtk_box_pack_start(GTK_BOX(ExpanderBox), TangencialBoxFrame, 0, 1, 0);
		gtk_box_pack_start(GTK_BOX(ExpanderBox), MachineBoxFrame, 0, 1, 0);
		gtk_box_pack_start(GTK_BOX(ExpanderBox), MaterialBoxFrame, 0, 1, 0);
		gtk_box_pack_start(GTK_BOX(ExpanderBox), ObjectsBoxFrame, 0, 1, 0);
		gtk_box_pack_start(GTK_BOX(ExpanderBox), MiscBoxFrame, 0, 1, 0);
		NbBox = gtk_scrolled_window_new(NULL, NULL);
		gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(NbBox), ExpanderBox);
		gtk_container_add(GTK_CONTAINER(ViewExpander), ViewBox);
		gtk_container_add(GTK_CONTAINER(ToolExpander), ToolBox);
		gtk_container_add(GTK_CONTAINER(MillingExpander), MillingBox);
		gtk_container_add(GTK_CONTAINER(TabsExpander), HoldingBox);
		gtk_container_add(GTK_CONTAINER(RotaryExpander), RotaryBox);
		gtk_container_add(GTK_CONTAINER(TangencialExpander), TangencialBox);
		gtk_container_add(GTK_CONTAINER(MachineExpander), MachineBox);
		gtk_container_add(GTK_CONTAINER(MaterialExpander), MaterialBox);
		gtk_container_add(GTK_CONTAINER(ObjectsExpander), ObjectsBox);
		gtk_container_add(GTK_CONTAINER(MiscExpander), MiscBox);
	}
	gtk_widget_set_size_request(NbBox, 300, -1);

	int n = 0;
	for (n = 0; n < P_LAST; n++) {
		GtkWidget *Label;
		GtkTooltips *tooltips = gtk_tooltips_new();
		GtkWidget *Box = gtk_hbox_new(0, 0);
		Label = gtk_label_new(_(PARAMETER[n].name));
		if (PARAMETER[n].type == T_FLOAT) {
			GtkWidget *Align = gtk_alignment_new(0.0, 0.5, 0.0, 0.0);
			gtk_container_add(GTK_CONTAINER(Align), Label);
			GtkAdjustment *Adj = (GtkAdjustment *)gtk_adjustment_new(PARAMETER[n].vdouble, PARAMETER[n].min, PARAMETER[n].max, PARAMETER[n].step, PARAMETER[n].step * 10.0, 0.0);
			ParamValue[n] = gtk_spin_button_new(Adj, PARAMETER[n].step, 3);
			gtk_box_pack_start(GTK_BOX(Box), Align, 1, 1, 0);
			gtk_box_pack_start(GTK_BOX(Box), ParamValue[n], 0, 0, 0);
		} else if (PARAMETER[n].type == T_DOUBLE) {
			GtkWidget *Align = gtk_alignment_new(0.0, 0.5, 0.0, 0.0);
			gtk_container_add(GTK_CONTAINER(Align), Label);
			GtkAdjustment *Adj = (GtkAdjustment *)gtk_adjustment_new(PARAMETER[n].vdouble, PARAMETER[n].min, PARAMETER[n].max, PARAMETER[n].step, PARAMETER[n].step * 10.0, 0.0);
			ParamValue[n] = gtk_spin_button_new(Adj, PARAMETER[n].step, 4);
			gtk_box_pack_start(GTK_BOX(Box), Align, 1, 1, 0);
			gtk_box_pack_start(GTK_BOX(Box), ParamValue[n], 0, 0, 0);
		} else if (PARAMETER[n].type == T_INT) {
			GtkWidget *Align = gtk_alignment_new(0.0, 0.5, 0.0, 0.0);
			gtk_container_add(GTK_CONTAINER(Align), Label);
			GtkAdjustment *Adj = (GtkAdjustment *)gtk_adjustment_new(PARAMETER[n].vdouble, PARAMETER[n].min, PARAMETER[n].max, PARAMETER[n].step, PARAMETER[n].step * 10.0, 0.0);
			ParamValue[n] = gtk_spin_button_new(Adj, PARAMETER[n].step, 1);
			gtk_box_pack_start(GTK_BOX(Box), Align, 1, 1, 0);
			gtk_box_pack_start(GTK_BOX(Box), ParamValue[n], 0, 0, 0);
		} else if (PARAMETER[n].type == T_SELECT) {
			GtkWidget *Align = gtk_alignment_new(0.0, 0.5, 0.0, 0.0);
			gtk_container_add(GTK_CONTAINER(Align), Label);
			ListStore[n] = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
			ParamValue[n] = gtk_combo_box_new_with_model(GTK_TREE_MODEL(ListStore[n]));
			g_object_unref(ListStore[n]);
			GtkCellRenderer *column;
			column = gtk_cell_renderer_text_new();
			gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(ParamValue[n]), column, TRUE);
			gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(ParamValue[n]), column, "cell-background", 0, "text", 1, NULL);
			gtk_combo_box_set_active(GTK_COMBO_BOX(ParamValue[n]), 1);
			gtk_box_pack_start(GTK_BOX(Box), Align, 1, 1, 0);
			gtk_box_pack_start(GTK_BOX(Box), ParamValue[n], 0, 0, 0);
		} else if (PARAMETER[n].type == T_BOOL) {
			GtkWidget *Align = gtk_alignment_new(0.0, 0.5, 0.0, 0.0);
			gtk_container_add(GTK_CONTAINER(Align), Label);
			ParamValue[n] = gtk_check_button_new_with_label(_("On/Off"));
			gtk_box_pack_start(GTK_BOX(Box), Align, 1, 1, 0);
			gtk_box_pack_start(GTK_BOX(Box), ParamValue[n], 0, 0, 0);
		} else if (PARAMETER[n].type == T_STRING) {
			GtkWidget *Align = gtk_alignment_new(0.0, 0.5, 0.0, 0.0);
			gtk_container_add(GTK_CONTAINER(Align), Label);
			ParamValue[n] = gtk_entry_new();
			gtk_entry_set_text(GTK_ENTRY(ParamValue[n]), PARAMETER[n].vstr);
			gtk_box_pack_start(GTK_BOX(Box), Align, 1, 1, 0);
			gtk_box_pack_start(GTK_BOX(Box), ParamValue[n], 0, 0, 0);
		} else if (PARAMETER[n].type == T_FILE) {
			GtkWidget *Align = gtk_alignment_new(0.0, 0.5, 0.0, 0.0);
			gtk_container_add(GTK_CONTAINER(Align), Label);
			ParamValue[n] = gtk_entry_new();
			gtk_entry_set_text(GTK_ENTRY(ParamValue[n]), PARAMETER[n].vstr);
			ParamButton[n] = gtk_button_new();
			GtkWidget *image = gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_BUTTON);
			gtk_button_set_image(GTK_BUTTON(ParamButton[n]), image);
			gtk_box_pack_start(GTK_BOX(Box), Align, 1, 1, 0);
			gtk_box_pack_start(GTK_BOX(Box), ParamValue[n], 0, 0, 0);
			gtk_box_pack_start(GTK_BOX(Box), ParamButton[n], 0, 0, 0);
		} else {
			continue;
		}
		gtk_tooltips_set_tip(tooltips, Box, _(PARAMETER[n].help), NULL);
		if (strcmp(PARAMETER[n].group, "View") == 0) {
			gtk_box_pack_start(GTK_BOX(ViewBox), Box, 0, 0, 0);
			PARAMETER[n].l1 = 0;
			PARAMETER[n].l2 = ViewNum;
			ViewNum++;
		} else if (strcmp(PARAMETER[n].group, "Tool") == 0) {
			gtk_box_pack_start(GTK_BOX(ToolBox), Box, 0, 0, 0);
			PARAMETER[n].l1 = 1;
			PARAMETER[n].l2 = ToolNum;
			ToolNum++;
		} else if (strcmp(PARAMETER[n].group, "Milling") == 0) {
			gtk_box_pack_start(GTK_BOX(MillingBox), Box, 0, 0, 0);
			PARAMETER[n].l1 = 2;
			PARAMETER[n].l2 = MillingNum;
			MillingNum++;
		} else if (strcmp(PARAMETER[n].group, "Holding-Tabs") == 0) {
			gtk_box_pack_start(GTK_BOX(HoldingBox), Box, 0, 0, 0);
			PARAMETER[n].l1 = 3;
			PARAMETER[n].l2 = HoldingNum;
			HoldingNum++;
		} else if (strcmp(PARAMETER[n].group, "Rotary") == 0) {
			gtk_box_pack_start(GTK_BOX(RotaryBox), Box, 0, 0, 0);
			PARAMETER[n].l1 = 3;
			PARAMETER[n].l2 = RotaryNum;
			RotaryNum++;
		} else if (strcmp(PARAMETER[n].group, "Misc") == 0) {
			gtk_box_pack_start(GTK_BOX(MiscBox), Box, 0, 0, 0);
			PARAMETER[n].l1 = 3;
			PARAMETER[n].l2 = MiscNum;
			MiscNum++;
		} else if (strcmp(PARAMETER[n].group, "Tangencial") == 0) {
			gtk_box_pack_start(GTK_BOX(TangencialBox), Box, 0, 0, 0);
			PARAMETER[n].l1 = 3;
			PARAMETER[n].l2 = TangencialNum;
			TangencialNum++;
		} else if (strcmp(PARAMETER[n].group, "Machine") == 0) {
			gtk_box_pack_start(GTK_BOX(MachineBox), Box, 0, 0, 0);
			PARAMETER[n].l1 = 4;
			PARAMETER[n].l2 = MachineNum;
			MachineNum++;
		} else if (strcmp(PARAMETER[n].group, "Material") == 0) {
			gtk_box_pack_start(GTK_BOX(MaterialBox), Box, 0, 0, 0);
			PARAMETER[n].l1 = 5;
			PARAMETER[n].l2 = MaterialNum;
			MaterialNum++;
		} else if (strcmp(PARAMETER[n].group, "Objects") == 0) {
			gtk_box_pack_start(GTK_BOX(ObjectsBox), Box, 0, 0, 0);
			PARAMETER[n].l1 = 6;
			PARAMETER[n].l2 = ObjectsNum;
			ObjectsNum++;
		}
	}

	OutputInfoLabel = gtk_label_new("-- OutputInfo --");
	gtk_box_pack_start(GTK_BOX(MachineBox), OutputInfoLabel, 0, 0, 0);

	/* import DXF */
	loading = 1;
	if (PARAMETER[P_V_DXF].vstr[0] != 0) {
#ifdef USE_G3D
		if (strstr(PARAMETER[P_V_DXF].vstr, ".dxf") > 0 || strstr(PARAMETER[P_V_DXF].vstr, ".DXF") > 0) {
			dxf_read(PARAMETER[P_V_DXF].vstr);
		} else {
			slice_3d(PARAMETER[P_V_DXF].vstr, 0.0);
		}
#else
		dxf_read(PARAMETER[P_V_DXF].vstr);
#endif
	}
	init_objects();
//	LayerLoadList();
	loading = 0;

	gtk_list_store_insert_with_values(ListStore[P_O_OFFSET], NULL, -1, 0, NULL, 1, _("None"), -1);
	gtk_list_store_insert_with_values(ListStore[P_O_OFFSET], NULL, -1, 0, NULL, 1, _("Inside"), -1);
	gtk_list_store_insert_with_values(ListStore[P_O_OFFSET], NULL, -1, 0, NULL, 1, _("Outside"), -1);

	gtk_list_store_insert_with_values(ListStore[P_O_ORDER], NULL, -1, 0, NULL, 1, _("-5 First"), -1);
	gtk_list_store_insert_with_values(ListStore[P_O_ORDER], NULL, -1, 0, NULL, 1, "-4", -1);
	gtk_list_store_insert_with_values(ListStore[P_O_ORDER], NULL, -1, 0, NULL, 1, "-3", -1);
	gtk_list_store_insert_with_values(ListStore[P_O_ORDER], NULL, -1, 0, NULL, 1, "-2", -1);
	gtk_list_store_insert_with_values(ListStore[P_O_ORDER], NULL, -1, 0, NULL, 1, "-1", -1);
	gtk_list_store_insert_with_values(ListStore[P_O_ORDER], NULL, -1, 0, NULL, 1, _("0 Auto"), -1);
	gtk_list_store_insert_with_values(ListStore[P_O_ORDER], NULL, -1, 0, NULL, 1, "1", -1);
	gtk_list_store_insert_with_values(ListStore[P_O_ORDER], NULL, -1, 0, NULL, 1, "2", -1);
	gtk_list_store_insert_with_values(ListStore[P_O_ORDER], NULL, -1, 0, NULL, 1, "3", -1);
	gtk_list_store_insert_with_values(ListStore[P_O_ORDER], NULL, -1, 0, NULL, 1, "4", -1);
	gtk_list_store_insert_with_values(ListStore[P_O_ORDER], NULL, -1, 0, NULL, 1, _("5 Last"), -1);

	gtk_list_store_insert_with_values(ListStore[P_H_ROTARYAXIS], NULL, -1, 0, NULL, 1, "A", -1);
	gtk_list_store_insert_with_values(ListStore[P_H_ROTARYAXIS], NULL, -1, 0, NULL, 1, "B", -1);
	gtk_list_store_insert_with_values(ListStore[P_H_ROTARYAXIS], NULL, -1, 0, NULL, 1, "C", -1);

	gtk_list_store_insert_with_values(ListStore[P_H_KNIFEAXIS], NULL, -1, 0, NULL, 1, "A", -1);
	gtk_list_store_insert_with_values(ListStore[P_H_KNIFEAXIS], NULL, -1, 0, NULL, 1, "B", -1);
	gtk_list_store_insert_with_values(ListStore[P_H_KNIFEAXIS], NULL, -1, 0, NULL, 1, "C", -1);

	gtk_list_store_insert_with_values(ListStore[P_O_PARAVIEW], NULL, -1, 0, NULL, 1, _("Expander"), -1);
	gtk_list_store_insert_with_values(ListStore[P_O_PARAVIEW], NULL, -1, 0, NULL, 1, _("Notebook-Tabs"), -1);

	DIR *dir;
	n = 0;
	struct dirent *ent;
	char dir_posts[PATH_MAX];
	if (program_path[0] == 0) {
		snprintf(dir_posts, PATH_MAX, "%s", "posts");
	} else {
		snprintf(dir_posts, PATH_MAX, "%s%s%s", program_path, DIR_SEP, "posts");
	}
	// fprintf(stderr, "postprocessor directory: '%s'\n", dir_posts);
	if ((dir = opendir(dir_posts)) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			if (ent->d_name[0] != '.') {
				char *pname = suffix_remove(ent->d_name);
				gtk_list_store_insert_with_values(ListStore[P_H_POST], NULL, -1, 0, NULL, 1, pname, -1);
				strcpy(postcam_plugins[n++], pname);
				postcam_plugins[n][0] = 0;
				free(pname);
				if (PARAMETER[P_H_POST].vint == -1) {
					PARAMETER[P_H_POST].vint = 0;
				}
			}
		}
		closedir (dir);
	} else {
		fprintf(stderr, "postprocessor directory not found: %s\n", dir_posts);
		PARAMETER[P_H_POST].vint = -1;
	}

/*
	if ((dir = opendir("fonts/")) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			if (ent->d_name[0] != '.') {
				char *pname = suffix_remove(ent->d_name);
				gtk_list_store_insert_with_values(ListStore[P_M_FONT], NULL, -1, 0, NULL, 1, pname, -1);
				free(pname);
			}
		}
		closedir (dir);
	} else {
		fprintf(stderr, "fonts directory not found: fonts/\n");
	}
*/

	g_signal_connect(G_OBJECT(ParamButton[P_MFILE]), "clicked", GTK_SIGNAL_FUNC(handler_save_gcode_as), NULL);
	g_signal_connect(G_OBJECT(ParamButton[P_TOOL_TABLE]), "clicked", GTK_SIGNAL_FUNC(handler_load_tooltable), NULL);
	g_signal_connect(G_OBJECT(ParamButton[P_V_DXF]), "clicked", GTK_SIGNAL_FUNC(handler_load_dxf), NULL);

	MaterialLoadList(program_path);
	ToolLoadTable();

	ParameterUpdate();
	for (n = 0; n < P_LAST; n++) {
		if (PARAMETER[n].readonly == 1) {
			gtk_widget_set_sensitive(GTK_WIDGET(ParamValue[n]), FALSE);
		} else if (PARAMETER[n].type == T_FLOAT) {
			gtk_signal_connect(GTK_OBJECT(ParamValue[n]), "changed", GTK_SIGNAL_FUNC(ParameterChanged), (gpointer)n);
		} else if (PARAMETER[n].type == T_DOUBLE) {
			gtk_signal_connect(GTK_OBJECT(ParamValue[n]), "changed", GTK_SIGNAL_FUNC(ParameterChanged), (gpointer)n);
		} else if (PARAMETER[n].type == T_INT) {
			gtk_signal_connect(GTK_OBJECT(ParamValue[n]), "changed", GTK_SIGNAL_FUNC(ParameterChanged), (gpointer)n);
		} else if (PARAMETER[n].type == T_SELECT) {
			gtk_signal_connect(GTK_OBJECT(ParamValue[n]), "changed", GTK_SIGNAL_FUNC(ParameterChanged), (gpointer)n);
		} else if (PARAMETER[n].type == T_BOOL) {
			gtk_signal_connect(GTK_OBJECT(ParamValue[n]), "toggled", GTK_SIGNAL_FUNC(ParameterChanged), (gpointer)n);
		} else if (PARAMETER[n].type == T_STRING) {
			gtk_signal_connect(GTK_OBJECT(ParamValue[n]), "changed", GTK_SIGNAL_FUNC(ParameterChanged), (gpointer)n);
		} else if (PARAMETER[n].type == T_FILE) {
			gtk_signal_connect(GTK_OBJECT(ParamValue[n]), "changed", GTK_SIGNAL_FUNC(ParameterChanged), (gpointer)n);
		}
	}

	int mat_num = PARAMETER[P_MAT_SELECT].vint;
	PARAMETER[P_MAT_CUTSPEED].vint = Material[mat_num].vc;
	PARAMETER[P_MAT_FEEDFLUTE4].vdouble = Material[mat_num].fz[FZ_FEEDFLUTE4];
	PARAMETER[P_MAT_FEEDFLUTE8].vdouble = Material[mat_num].fz[FZ_FEEDFLUTE8];
	PARAMETER[P_MAT_FEEDFLUTE12].vdouble = Material[mat_num].fz[FZ_FEEDFLUTE12];
	strcpy(PARAMETER[P_MAT_TEXTURE].vstr, Material[mat_num].texture);

	StatusBar = gtk_statusbar_new();

	gCodeViewLua = gtk_source_view_new();
	gtk_source_view_set_auto_indent(GTK_SOURCE_VIEW(gCodeViewLua), TRUE);
	gtk_source_view_set_highlight_current_line(GTK_SOURCE_VIEW(gCodeViewLua), TRUE);
	gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(gCodeViewLua), TRUE);
//	gtk_source_view_set_right_margin_position(GTK_SOURCE_VIEW(gCodeViewLua), 80);
//	gtk_source_view_set_show_right_margin(GTK_SOURCE_VIEW(gCodeViewLua), TRUE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(gCodeViewLua), GTK_WRAP_WORD_CHAR);

	GtkWidget *textWidgetLua = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(textWidgetLua), gCodeViewLua);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(textWidgetLua), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);


	GtkWidget *LuaToolBar = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(LuaToolBar), GTK_TOOLBAR_ICONS);

	GtkToolItem *LuaToolItemSaveGcode = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE);
	gtk_tool_item_set_tooltip_text(LuaToolItemSaveGcode, "Save Output");
	gtk_toolbar_insert(GTK_TOOLBAR(LuaToolBar), LuaToolItemSaveGcode, -1);
	g_signal_connect(G_OBJECT(LuaToolItemSaveGcode), "clicked", GTK_SIGNAL_FUNC(handler_save_lua), NULL);

	GtkToolItem *LuaToolItemSep = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(LuaToolBar), LuaToolItemSep, -1); 

	OutputErrorLabel = gtk_label_new("-- OutputErrors --");

	GtkWidget *textWidgetLuaBox = gtk_vbox_new(0, 0);
	gtk_box_pack_start(GTK_BOX(textWidgetLuaBox), LuaToolBar, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(textWidgetLuaBox), textWidgetLua, 1, 1, 0);
	gtk_box_pack_start(GTK_BOX(textWidgetLuaBox), OutputErrorLabel, 0, 0, 0);

	GtkTextBuffer *bufferLua;
	const gchar *textLua = "WARNING: No Postprocessor loaded";
	bufferLua = gtk_text_view_get_buffer(GTK_TEXT_VIEW(gCodeViewLua));
	gtk_text_buffer_set_text(bufferLua, textLua, -1);

	gCodeView = gtk_source_view_new();
	gtk_source_view_set_auto_indent(GTK_SOURCE_VIEW(gCodeView), TRUE);
	gtk_source_view_set_highlight_current_line(GTK_SOURCE_VIEW(gCodeView), TRUE);
	gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(gCodeView), TRUE);
//	gtk_source_view_set_right_margin_position(GTK_SOURCE_VIEW(gCodeView), 80);
//	gtk_source_view_set_show_right_margin(GTK_SOURCE_VIEW(gCodeView), TRUE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(gCodeView), GTK_WRAP_WORD_CHAR);

	GtkWidget *textWidget = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(textWidget), gCodeView);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(textWidget), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

	GtkTextBuffer *buffer;
	const gchar *text = "Hello Text";
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(gCodeView));
	gtk_text_buffer_set_text(buffer, text, -1);

	GtkSourceLanguageManager *manager = gtk_source_language_manager_get_default();
	const gchar * const *current;
	int i;
	current = gtk_source_language_manager_get_search_path(manager);
	for (i = 0; current[i] != NULL; i++) {}
	gchar **lang_dirs;
	lang_dirs = g_new0(gchar *, i + 2);
	for (i = 0; current[i] != NULL; i++) {
		lang_dirs[i] = g_build_filename(current[i], NULL);
	}
	lang_dirs[i] = g_build_filename(".", NULL);
	gtk_source_language_manager_set_search_path(manager, lang_dirs);
	g_strfreev(lang_dirs);
	GtkSourceLanguage *ngclang = gtk_source_language_manager_get_language(manager, ".ngc");
	gtk_source_buffer_set_language(GTK_SOURCE_BUFFER(buffer), ngclang);

	GtkSourceLanguage *langLua = gtk_source_language_manager_get_language(manager, "lua");
	gtk_source_buffer_set_language(GTK_SOURCE_BUFFER(bufferLua), langLua);

	GtkWidget *NbBox2 = gtk_table_new(2, 2, FALSE);
	GtkWidget *notebook2 = gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook2), GTK_POS_TOP);
	gtk_table_attach_defaults(GTK_TABLE(NbBox2), notebook2, 0, 1, 0, 1);

	GtkWidget *glCanvasLabel = gtk_label_new(_("3D-View"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), glCanvas, glCanvasLabel);

	gCodeViewLabel = gtk_label_new(_("Output"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), textWidget, gCodeViewLabel);
	gtk_label_set_text(GTK_LABEL(OutputInfoLabel), output_info);

	gCodeViewLabelLua = gtk_label_new(_("PostProcessor"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), textWidgetLuaBox, gCodeViewLabelLua);

#ifdef USE_VNC
	if (PARAMETER[P_O_VNCSERVER].vstr[0] != 0) {
		char port[128];
		GtkWidget *VncLabel = gtk_label_new(_("VNC"));
		VncView = vnc_display_new();
		GtkWidget *VncWindow = gtk_scrolled_window_new(NULL, NULL);
		gtk_container_add(GTK_CONTAINER(VncWindow), VncView);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(VncWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), VncWindow, VncLabel);
		snprintf(port, sizeof(port), "%i", PARAMETER[P_O_VNCPORT].vint);
		vnc_display_open_host(VNC_DISPLAY(VncView), PARAMETER[P_O_VNCSERVER].vstr, port);
	}
#endif

#ifdef USE_WEBKIT
	GtkWidget *WebKitLabel = gtk_label_new(_("Documentation"));
	GtkWidget *WebKitBox = gtk_vbox_new(0, 0);
	GtkWidget *WebKitWindow = gtk_scrolled_window_new(NULL, NULL);
	WebKit = webkit_web_view_new();
	GtkWidget *WebKitToolBar = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(WebKitToolBar), GTK_TOOLBAR_ICONS);

	GtkToolItem *WebKitBack = gtk_tool_button_new_from_stock(GTK_STOCK_GO_BACK);
	gtk_toolbar_insert(GTK_TOOLBAR(WebKitToolBar), WebKitBack, -1);
	g_signal_connect(G_OBJECT(WebKitBack), "clicked", GTK_SIGNAL_FUNC(handler_webkit_back), NULL);

	GtkToolItem *WebKitHome = gtk_tool_button_new_from_stock(GTK_STOCK_HOME);
	gtk_toolbar_insert(GTK_TOOLBAR(WebKitToolBar), WebKitHome, -1);
	g_signal_connect(G_OBJECT(WebKitHome), "clicked", GTK_SIGNAL_FUNC(handler_webkit_home), NULL);

	GtkToolItem *WebKitForward = gtk_tool_button_new_from_stock(GTK_STOCK_GO_FORWARD);
	gtk_toolbar_insert(GTK_TOOLBAR(WebKitToolBar), WebKitForward, -1);
	g_signal_connect(G_OBJECT(WebKitForward), "clicked", GTK_SIGNAL_FUNC(handler_webkit_forward), NULL);

	gtk_box_pack_start(GTK_BOX(WebKitBox), WebKitToolBar, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(WebKitBox), WebKitWindow, 1, 1, 0);
	gtk_container_add(GTK_CONTAINER(WebKitWindow), WebKit);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(WebKitWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), WebKitBox, WebKitLabel);
	webkit_web_view_open(WEBKIT_WEB_VIEW(WebKit), "file:///usr/src/cammill/index.html");
#endif

/*
	Embedded Programms (-wid)
	GtkWidget *PlugLabel = gtk_label_new(_("PlugIn"));
	GtkWidget *sck = gtk_socket_new();
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), sck, PlugLabel);
*/

	hbox = gtk_hpaned_new();
	gtk_paned_pack1(GTK_PANED(hbox), NbBox, TRUE, FALSE);
	gtk_paned_pack2(GTK_PANED(hbox), NbBox2, TRUE, TRUE);
	gtk_paned_set_position(GTK_PANED(hbox), PannedStat);

	SizeInfoLabel = gtk_label_new("Width=0mm / Height=0mm");
	GtkWidget *SizeInfo = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(SizeInfo), SizeInfoLabel);
	gtk_container_set_border_width(GTK_CONTAINER(SizeInfo), 4);

	char iconfile[PATH_MAX];
	snprintf(iconfile, PATH_MAX, "%s%s", program_path, "icons/logo-top.png");

	GtkWidget *LogoIMG = gtk_image_new_from_file(iconfile);
	GtkWidget *Logo = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(Logo), LogoIMG);
	gtk_signal_connect(GTK_OBJECT(Logo), "button_press_event", GTK_SIGNAL_FUNC(handler_about), NULL);

	GtkWidget *topBox = gtk_hbox_new(0, 0);
	gtk_box_pack_start(GTK_BOX(topBox), ToolBar, 1, 1, 0);
	gtk_box_pack_start(GTK_BOX(topBox), SizeInfo, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(topBox), Logo, 0, 0, 0);

	vbox = gtk_vbox_new(0, 0);
	gtk_box_pack_start(GTK_BOX(vbox), MenuBar, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(vbox), topBox, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, 1, 1, 0);
	gtk_box_pack_start(GTK_BOX(vbox), StatusBar, 0, 0, 0);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "CAMmill 2D");
        gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	snprintf(iconfile, PATH_MAX, "%s%s", program_path, "icons/logo-top.png");
	gtk_window_set_icon(GTK_WINDOW(window), create_pixbuf(iconfile));

	gtk_signal_connect(GTK_OBJECT(window), "destroy_event", GTK_SIGNAL_FUNC (handler_destroy), NULL);
	gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC (handler_destroy), NULL);
	gtk_container_add (GTK_CONTAINER(window), vbox);

	gtk_widget_show_all(window);

/*
	Embedded Programms (-wid)
	GdkNativeWindow nwnd = gtk_socket_get_id(GTK_SOCKET(sck));
	g_print("%i\n", nwnd);
*/
}

int main (int argc, char *argv[]) {

	get_executable_path(program_path, sizeof(program_path));

	bindtextdomain("cammill", "intl");
	textdomain("cammill");

	// force dots in printf
	setlocale(LC_NUMERIC, "C");

	SetupLoad();
	ArgsRead(argc, argv);
//	SetupShow();

	if (PARAMETER[P_O_BATCHMODE].vint == 1 && PARAMETER[P_MFILE].vstr[0] != 0) {
		save_gcode = 1;
	}

	gtk_init(&argc, &argv);
	gtk_gl_init(&argc, &argv);
	create_gui();

	strcpy(output_extension, "ngc");
	strcpy(output_info, "");

	if (PARAMETER[P_H_POST].vint != -1) {
		postcam_init_lua(program_path, postcam_plugins[PARAMETER[P_H_POST].vint]);
	}
	postcam_plugin = PARAMETER[P_H_POST].vint;
	gtk_label_set_text(GTK_LABEL(OutputInfoLabel), output_info);
	char tmp_str[1024];
	snprintf(tmp_str, sizeof(tmp_str), "%s (%s)", _("Output"), output_extension);
	gtk_label_set_text(GTK_LABEL(gCodeViewLabel), tmp_str);
	if (PARAMETER[P_H_POST].vint != -1) {
		postcam_load_source(postcam_plugins[PARAMETER[P_H_POST].vint]);
	}

	gtk_timeout_add(1000/25, handler_periodic_action, NULL);
	gtk_main ();

	postcam_exit_lua();

	return 0;
}


