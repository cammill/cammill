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
#endif
#ifdef USE_G3D
#include <g3d/g3d.h>
#endif
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
#include <pocket.h>

#include "os-hacks.h"

#define dot(ux,uy,uz,vx,vy,vz) (ux * vx + uy * vy + uz * vz)

extern float size_x;
extern float size_y;
extern double min_x;
extern double min_y;
extern double max_x;
extern double max_y;
extern FILE *fd_out;
extern int object_last;
extern char postcam_plugins[100][1024];
extern int postcam_plugin;
extern int update_post;
extern char *output_buffer;
extern char output_extension[128];
extern char output_info[1024];
extern double mill_distance_xy;
extern double mill_distance_z;
extern double move_distance_xy;
extern double move_distance_z;

int MaterialMax = 0;
_MATERIAL Material[100];

int mill_start = 0;
int mill_start_all = 0;
double mill_last_x = 0.0;
double mill_last_y = 0.0;
double mill_last_z = 0.0;
int tool_last = -1;
char *rotary_axis[3] = {"A", "B", "C"};
char cline[1024];


#ifndef CALLBACK
#define CALLBACK
#endif

extern GtkListStore *ListStore[P_LAST];
extern GtkWidget *OutputInfoLabel;
extern GtkWidget *gCodeViewLabel;

void postcam_load_source (char *plugin);
GLuint texture_load (char *filename);

void CALLBACK beginCallback(GLenum which) {
   glBegin(which);
}

void CALLBACK errorCallback(GLenum errorCode) {
	gluErrorString(errorCode);
//	const GLubyte *estring;
//	estring = gluErrorString(errorCode);
//	fprintf(stderr, "Tessellation Error: %s\n", (char *) estring);
//	exit(0);
}

void CALLBACK endCallback(void) {
	glEnd();
}

void CALLBACK vertexCallback(GLvoid *vertex) {
	const GLdouble *pointer;
	pointer = (GLdouble *) vertex;
	glColor3dv(pointer+3);
	glVertex3dv(pointer);
}

void CALLBACK combineCallback(GLdouble coords[3], GLdouble *vertex_data[4], GLfloat weight[4], GLdouble **dataOut ) {
	GLdouble *vertex;
	int i;
	vertex = (GLdouble *)malloc(6 * sizeof(GLdouble));
	vertex[0] = coords[0];
	vertex[1] = coords[1];
	vertex[2] = coords[2];
	for (i = 3; i < 6; i++) {
		vertex[i] = weight[0] * vertex_data[0][i] + weight[1] * vertex_data[1][i] + weight[2] * vertex_data[2][i] + weight[3] * vertex_data[3][i];
	}
	*dataOut = vertex;
}

void object2poly (int object_num, double depth, double depth2, int invert) {
	int num = 0;
	int nverts = 0;
	GLUtesselator *tobj;
	GLdouble rect2[MAX_LINES][3];
	if (PARAMETER[P_V_TEXTURES].vint == 1) {
		glColor4f(1.0, 1.0, 1.0, 1.0);
		texture_load(Material[PARAMETER[P_MAT_SELECT].vint].texture);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glTexGend(GL_S,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
		glTexGend(GL_T,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glScalef(0.002, 0.002, 0.002);
	} else  {
		if (PARAMETER[P_O_SELECT].vint == object_num) {
			glColor4f(1.0, 0.0, 0.0, 0.5);
		} else {
			if (invert == 0) {
				glColor4f(0.0, 0.5, 0.2, 0.5);
			} else {
				glColor4f(0.0, 0.75, 0.3, 0.5);
			}
		}
	}
	tobj = gluNewTess();
	gluTessCallback(tobj, GLU_TESS_VERTEX, (GLvoid (CALLBACK*) ()) &glVertex3dv);
	gluTessCallback(tobj, GLU_TESS_BEGIN, (GLvoid (CALLBACK*) ()) &beginCallback);
	gluTessCallback(tobj, GLU_TESS_END, (GLvoid (CALLBACK*) ()) &endCallback);
	gluTessCallback(tobj, GLU_TESS_ERROR, (GLvoid (CALLBACK*) ()) &errorCallback);
	glShadeModel(GL_FLAT);
	gluTessBeginPolygon(tobj, NULL);
	if (invert == 0) {
		if (myOBJECTS[object_num].climb == 0) {
			gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_POSITIVE);
		} else {
			gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NEGATIVE);
		}
	} else {
		if (myOBJECTS[object_num].climb == 0) {
			gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NEGATIVE);
		} else {
			gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_POSITIVE);
		}
	}
	gluTessNormal(tobj, 0, 0, 1);
	gluTessBeginContour(tobj);
	if (myLINES[myOBJECTS[object_num].line[0]].type == TYPE_CIRCLE) {
		gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_POSITIVE);
		int lnum = myOBJECTS[object_num].line[0];
		float an = 0.0;
		float r = myLINES[lnum].opt;
		float x = myLINES[lnum].cx;
		float y = myLINES[lnum].cy;
		float last_x = x + r;
		float last_y = y;
		for (an = 0.0; an < 360.0; an += 9.0) {
			float angle1 = toRad(an);
			float x1 = r * cos(angle1);
			float y1 = r * sin(angle1);
			rect2[nverts][0] = (GLdouble)x + x1;
			rect2[nverts][1] = (GLdouble)y + y1;
			rect2[nverts][2] = (GLdouble)depth;
			gluTessVertex(tobj, rect2[nverts], rect2[nverts]);
			if (depth != depth2) {
				glBegin(GL_QUADS);
				glVertex3f((float)last_x, (float)last_y, depth);
				glVertex3f((float)x + x1, (float)y + y1, depth);
				glVertex3f((float)x + x1, (float)y + y1, depth2);
				glVertex3f((float)last_x, (float)last_y, depth2);
				glEnd();
			}
			last_x = (float)x + x1;
			last_y = (float)y + y1;
			nverts++;
		}
	} else {
		for (num = 0; num < line_last; num++) {
			if (myOBJECTS[object_num].line[num] != 0) {
				int lnum = myOBJECTS[object_num].line[num];
				rect2[nverts][0] = (GLdouble)myLINES[lnum].x1;
				rect2[nverts][1] = (GLdouble)myLINES[lnum].y1;
				rect2[nverts][2] = (GLdouble)depth;
				gluTessVertex(tobj, rect2[nverts], rect2[nverts]);
				if (depth != depth2) {
					glBegin(GL_QUADS);
					glVertex3f((float)myLINES[lnum].x1, (float)myLINES[lnum].y1, depth);
					glVertex3f((float)myLINES[lnum].x2, (float)myLINES[lnum].y2, depth);
					glVertex3f((float)myLINES[lnum].x2, (float)myLINES[lnum].y2, depth2);
					glVertex3f((float)myLINES[lnum].x1, (float)myLINES[lnum].y1, depth2);
					glEnd();
				}
				nverts++;
			}
		}
	}
	int num5 = 0;
	for (num5 = 0; num5 < object_last; num5++) {
		if (num5 != object_num && myOBJECTS[num5].closed == 1 && myOBJECTS[num5].inside == 1) {
			int lnum = myOBJECTS[num5].line[0];
			int pipret = 0;
			double testx = myLINES[lnum].x1;
			double testy = myLINES[lnum].y1;
			pipret = point_in_object(object_num, -1, testx, testy);
			if (pipret == 1) {
				gluNextContour(tobj, GLU_INTERIOR);
				if (myLINES[lnum].type == TYPE_CIRCLE) {
					float an = 0.0;
					float r = myLINES[lnum].opt;
					float x = myLINES[lnum].cx;
					float y = myLINES[lnum].cy;
					float last_x = x + r;
					float last_y = y;
					if (myOBJECTS[object_num].climb == 1) {
						for (an = 0.0; an < 360.0; an += 9.0) {
							float angle1 = toRad(an);
							float x1 = r * cos(angle1);
							float y1 = r * sin(angle1);
							rect2[nverts][0] = (GLdouble)x + x1;
							rect2[nverts][1] = (GLdouble)y + y1;
							rect2[nverts][2] = (GLdouble)depth;
							gluTessVertex(tobj, rect2[nverts], rect2[nverts]);
							if (depth != depth2) {
								glBegin(GL_QUADS);
								glVertex3f((float)last_x, (float)last_y, depth);
								glVertex3f((float)x + x1, (float)y + y1, depth);
								glVertex3f((float)x + x1, (float)y + y1, depth2);
								glVertex3f((float)last_x, (float)last_y, depth2);
								glEnd();
							}
							last_x = (float)x + x1;
							last_y = (float)y + y1;
							nverts++;
						}
					} else {
						for (an = 360.0; an > 0.0; an -= 9.0) {
							float angle1 = toRad(an);
							float x1 = r * cos(angle1);
							float y1 = r * sin(angle1);
							rect2[nverts][0] = (GLdouble)x + x1;
							rect2[nverts][1] = (GLdouble)y + y1;
							rect2[nverts][2] = (GLdouble)depth;
							gluTessVertex(tobj, rect2[nverts], rect2[nverts]);
							if (depth != depth2) {
								glBegin(GL_QUADS);
								glVertex3f((float)last_x, (float)last_y, depth);
								glVertex3f((float)x + x1, (float)y + y1, depth);
								glVertex3f((float)x + x1, (float)y + y1, depth2);
								glVertex3f((float)last_x, (float)last_y, depth2);
								glEnd();
							}
							last_x = (float)x + x1;
							last_y = (float)y + y1;
							nverts++;
						}
					}
				} else {
					for (num = 0; num < line_last; num++) {
						if (myOBJECTS[num5].line[num] != 0) {
							int lnum = myOBJECTS[num5].line[num];
							rect2[nverts][0] = (GLdouble)myLINES[lnum].x1;
							rect2[nverts][1] = (GLdouble)myLINES[lnum].y1;
							rect2[nverts][2] = (GLdouble)depth;
							gluTessVertex(tobj, rect2[nverts], rect2[nverts]);
							if (depth != depth2) {
								glBegin(GL_QUADS);
								glVertex3f((float)myLINES[lnum].x1, (float)myLINES[lnum].y1, depth);
								glVertex3f((float)myLINES[lnum].x2, (float)myLINES[lnum].y2, depth);
								glVertex3f((float)myLINES[lnum].x2, (float)myLINES[lnum].y2, depth2);
								glVertex3f((float)myLINES[lnum].x1, (float)myLINES[lnum].y1, depth2);
								glEnd();
							}
							nverts++;
						}
					}
				}
			}
		}
	}
	gluTessEndPolygon(tobj);
	gluTessCallback(tobj, GLU_TESS_VERTEX, (GLvoid (CALLBACK*) ()) &vertexCallback);
	gluTessCallback(tobj, GLU_TESS_BEGIN, (GLvoid (CALLBACK*) ()) &beginCallback);
	gluTessCallback(tobj, GLU_TESS_END, (GLvoid (CALLBACK*) ()) &endCallback);
	gluTessCallback(tobj, GLU_TESS_ERROR, (GLvoid (CALLBACK*) ()) &errorCallback);
	gluTessCallback(tobj, GLU_TESS_COMBINE, (GLvoid (CALLBACK*) ()) &combineCallback);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	gluDeleteTess(tobj);
}


void DrawLine (float x1, float y1, float x2, float y2, float z, float w) {
	float angle = atan2(y2 - y1, x2 - x1);
	float t2sina1 = w / 2 * sin(angle);
	float t2cosa1 = w / 2 * cos(angle);
	glBegin(GL_QUADS);
	glVertex3f(x1 + t2sina1, y1 - t2cosa1, z);
	glVertex3f(x2 + t2sina1, y2 - t2cosa1, z);
	glVertex3f(x2 - t2sina1, y2 + t2cosa1, z);
	glVertex3f(x1 - t2sina1, y1 + t2cosa1, z);
	glEnd();
}

void DrawArrow (float x1, float y1, float x2, float y2, float z, float w) {
	float dx = x2 - x1;
	float dy = y2 - y1;
	float len = sqrt(dx * dx + dy * dy);
	float asize = 2.0;
	if (len < asize) {
		return;
	}
	float angle = atan2(dy, dx);
	float off_x = asize * cos(angle + toRad(45.0 + 90.0));
	float off_y = asize * sin(angle + toRad(45.0 + 90.0));
	float off2_x = asize * cos(angle + toRad(-45.0 - 90.0));
	float off2_y = asize * sin(angle + toRad(-45.0 - 90.0));
	float half_x = x1 + (x2 - x1) / 2.0;
	float half_y = y1 + (y2 - y1) / 2.0;
	glBegin(GL_LINES);
	glVertex3f(half_x, half_y, z);
	glVertex3f(half_x + off_x, half_y + off_y, z);
	glVertex3f(half_x, half_y, z);
	glVertex3f(half_x + off2_x, half_y + off2_y, z);
	glEnd();
}

void draw_line_wrap_conn (float x1, float y1, float depth1, float depth2) {
	float ry = 0.0;
	float rz = 0.0;
	glBegin(GL_LINES);
	point_rotate(y1, depth1, &ry, &rz);
	glVertex3f(x1, ry, rz);
	point_rotate(y1, depth2, &ry, &rz);
	glVertex3f(x1, ry, rz);
	glEnd();
}

void draw_line_wrap (float x1, float y1, float x2, float y2, float depth) {
	float radius = (PARAMETER[P_MAT_DIAMETER].vdouble / 2.0) + depth;
	float dX = x2 - x1;
	float dY = y2 - y1;
	float dashes = dY;
	if (dashes < -1.0) {
		dashes *= -1;
	}
	float an = y1 / (PARAMETER[P_MAT_DIAMETER].vdouble * PI) * 360;
	float rangle = toRad(an - 90.0);
	float ry = radius * cos(rangle);
	float rz = radius * sin(rangle);
	glBegin(GL_LINE_STRIP);
	glVertex3f(x1, ry, -rz);
	if (dashes > 1.0) {
		float dashX = dX / dashes;
		float dashY = dY / dashes;
		float q = 0.0;
		while (q++ < dashes) {
			x1 += dashX;
			y1 += dashY;
			an = y1 / (PARAMETER[P_MAT_DIAMETER].vdouble * PI) * 360;
			rangle = toRad(an - 90.0);
			ry = radius * cos(rangle);
			rz = radius * sin(rangle);
			glVertex3f(x1, ry, -rz);
		}
	}
	an = y2 / (PARAMETER[P_MAT_DIAMETER].vdouble * PI) * 360;
	rangle = toRad(an - 90.0);
	ry = radius * cos(rangle);
	rz = radius * sin(rangle);
	glVertex3f(x2, ry, -rz);
	glEnd();
}

void draw_oline (float x1, float y1, float x2, float y2, float depth) {
	if (PARAMETER[P_M_ROTARYMODE].vint == 1) {
		draw_line_wrap(x1, y1, x2, y2, 0.0);
		draw_line_wrap(x1, y1, x2, y2, depth);
		draw_line_wrap_conn(x1, y1, 0.0, depth);
		draw_line_wrap_conn(x2, y2, 0.0, depth);
	} else {
		glBegin(GL_LINES);
		glVertex3f(x1, y1, 0.02);
		glVertex3f(x2, y2, 0.02);
	//	if (PARAMETER[P_V_HELPLINES].vint == 1) {
			glBegin(GL_LINES);
			glVertex3f(x1, y1, depth);
			glVertex3f(x2, y2, depth);
			glVertex3f(x1, y1, depth);
			glVertex3f(x1, y1, 0.02);
			glVertex3f(x2, y2, depth);
			glVertex3f(x2, y2, 0.02);
	//	}
		glEnd();
	}
}

void draw_line2 (float x1, float y1, float z1, float x2, float y2, float z2, float width) {
	if (PARAMETER[P_M_ROTARYMODE].vint == 1) {
		draw_line_wrap(x1, y1, x2, y2, 0.0);
		draw_line_wrap(x1, y1, x2, y2, z1);
		draw_line_wrap_conn(x1, y1, 0.0, z1);
		draw_line_wrap_conn(x2, y2, 0.0, z1);
	} else {
		if (PARAMETER[P_V_HELPLINES].vint == 1) {
			DrawLine(x1, y1, x2, y2, z1, width);
			GLUquadricObj *quadric=gluNewQuadric();
			gluQuadricNormals(quadric, GLU_SMOOTH);
			gluQuadricOrientation(quadric,GLU_OUTSIDE);
			glPushMatrix();
			glTranslatef(x1, y1, z1);
			gluDisk(quadric, 0.0, width / 2.0, 18, 1);
			glPopMatrix();
			glPushMatrix();
			glTranslatef(x2, y2, z1);
			gluDisk(quadric, 0.0, width / 2.0, 18, 1);
			glPopMatrix();
			gluDeleteQuadric(quadric);
		}
		glBegin(GL_LINES);
		glVertex3f(x1, y1, z1);
		glVertex3f(x2, y2, z2);
		glEnd();
	}
}

void draw_line (float x1, float y1, float z1, float x2, float y2, float z2, float width) {
	if (PARAMETER[P_M_ROTARYMODE].vint == 1) {
		glColor4f(1.0, 0.0, 1.0, 1.0);
		draw_line_wrap(x1, y1, x2, y2, 0.0);
		draw_line_wrap(x1, y1, x2, y2, z1);
		draw_line_wrap_conn(x1, y1, 0.0, z1);
		draw_line_wrap_conn(x2, y2, 0.0, z1);
	} else {
		if (PARAMETER[P_V_HELPDIA].vint == 1) {
			glColor4f(1.0, 1.0, 0.0, 1.0);
			DrawLine(x1, y1, x2, y2, z1, width);
			GLUquadricObj *quadric=gluNewQuadric();
			gluQuadricNormals(quadric, GLU_SMOOTH);
			gluQuadricOrientation(quadric,GLU_OUTSIDE);
			glPushMatrix();
			glTranslatef(x1, y1, z1);
			gluDisk(quadric, 0.0, width / 2.0, 18, 1);
			glPopMatrix();
			glPushMatrix();
			glTranslatef(x2, y2, z1);
			gluDisk(quadric, 0.0, width / 2.0, 18, 1);
			glPopMatrix();
			gluDeleteQuadric(quadric);
		}
		glColor4f(1.0, 0.0, 1.0, 1.0);
		glBegin(GL_LINES);
		glVertex3f(x1, y1, z1 + 0.02);
		glVertex3f(x2, y2, z2 + 0.02);
		glEnd();
		DrawArrow(x1, y1, x2, y2, z1 + 0.02, width);
	}
}

void draw_line3 (float x1, float y1, float z1, float x2, float y2, float z2) {
	if (PARAMETER[P_M_ROTARYMODE].vint == 1) {
		glColor4f(1.0, 0.0, 1.0, 1.0);
		draw_line_wrap(x1, y1, x2, y2, z1);
	} else {
		glColor4f(1.0, 0.0, 1.0, 1.0);
		glBegin(GL_LINES);
		glVertex3f(x1, y1, z1 + 0.02);
		glVertex3f(x2, y2, z2 + 0.02);
		glEnd();
	}
}

void line_invert (int num) {
	double tempx = myLINES[num].x2;
	double tempy = myLINES[num].y2;
	myLINES[num].x2 = myLINES[num].x1;
	myLINES[num].y2 = myLINES[num].y1;
	myLINES[num].x1 = tempx;
	myLINES[num].y1 = tempy;
	myLINES[num].opt *= -1;
}

int point_in_object (int object_num, int object_ex, double testx, double testy) {
	int result = 0;
	int num = 0;
	int onum = object_num;
	testx += 0.0001;
	testy += 0.0001;
	if (object_num == -1) {
		for (onum = 0; onum < object_last; onum++) {
			if (onum == object_ex) {
				continue;
			}
			if (myOBJECTS[onum].closed == 0) {
				continue;
			}
			for (num = 0; num < line_last; num++) {
				if (myOBJECTS[onum].line[num] != 0) {
					int lnum = myOBJECTS[onum].line[num];
					if (myLINES[lnum].y2 == testy) {
						if ((myLINES[lnum].x2 == testx) || (myLINES[lnum].y1 == testy && ((myLINES[lnum].x2 > testx) == (myLINES[lnum].x1 < testx)))) {
							fprintf(stderr, "Point on line\n");
							return -1;
						}
					}
					if ((myLINES[lnum].y1 < testy) != (myLINES[lnum].y2 < testy)) {
						if (myLINES[lnum].x1 >= testx) {
							if (myLINES[lnum].x2 > testx) {
								result = 1 - result;
							} else {
								double d = (double)(myLINES[lnum].x1 - testx) * (myLINES[lnum].y2 - testy) - (double)(myLINES[lnum].x2 - testx) * (myLINES[lnum].y1 - testy);
								if (!d) {
									return -1;
								}
								if ((d > 0) == (myLINES[lnum].y2 > myLINES[lnum].y1)) {
									result = 1 - result;
								}
							}
						} else {
							if (myLINES[lnum].x2 > testx) {
								double d = (double)(myLINES[lnum].x1 - testx) * (myLINES[lnum].y2 - testy) - (double)(myLINES[lnum].x2 - testx) * (myLINES[lnum].y1 - testy);
								if (!d) {
									return -1;
								}
								if ((d > 0) == (myLINES[lnum].y2 > myLINES[lnum].y1)) {
									result = 1 - result;
								}
							}
						}
					}
				}
			}
		}
	} else {
		if (myOBJECTS[onum].closed == 0) {
			return 0;
		}
		for (num = 0; num < line_last; num++) {
			if (myOBJECTS[onum].line[num] != 0) {
				int lnum = myOBJECTS[onum].line[num];
				if (myLINES[lnum].y2 == testy) {
					if ((myLINES[lnum].x2 == testx) || (myLINES[lnum].y1 == testy && ((myLINES[lnum].x2 > testx) == (myLINES[lnum].x1 < testx)))) {
						fprintf(stderr, "Point on line\n");
						return -1;
					}
				}
				if ((myLINES[lnum].y1 < testy) != (myLINES[lnum].y2 < testy)) {
					if (myLINES[lnum].x1 >= testx) {
						if (myLINES[lnum].x2 > testx) {
							result = 1 - result;
						} else {
							double d = (double)(myLINES[lnum].x1 - testx) * (myLINES[lnum].y2 - testy) - (double)(myLINES[lnum].x2 - testx) * (myLINES[lnum].y1 - testy);
							if (!d) {
								return -1;
							}
							if ((d > 0) == (myLINES[lnum].y2 > myLINES[lnum].y1)) {
								result = 1 - result;
							}
						}
					} else {
						if (myLINES[lnum].x2 > testx) {
							double d = (double)(myLINES[lnum].x1 - testx) * (myLINES[lnum].y2 - testy) - (double)(myLINES[lnum].x2 - testx) * (myLINES[lnum].y1 - testy);
							if (!d) {
								return -1;
							}
							if ((d > 0) == (myLINES[lnum].y2 > myLINES[lnum].y1)) {
								result = 1 - result;
							}
						}
					}
				}
			}
		}
	}
	return result;
}

void point_rotate (float y, float depth, float *ny, float *nz) {
	float radius = (PARAMETER[P_MAT_DIAMETER].vdouble / 2.0) + depth;
	float an = y / (PARAMETER[P_MAT_DIAMETER].vdouble * PI) * 360;
	float rangle = toRad(an - 90.0);
	*ny = radius * cos(rangle);
	*nz = radius * -sin(rangle);
}

double _X (double x) {
	if (PARAMETER[P_M_ROTARYMODE].vint == 1 && PARAMETER[P_H_ROTARYAXIS].vint == 1) {
		return x / (PARAMETER[P_MAT_DIAMETER].vdouble * PI) * 360;
	}
	return x;
}

double _Y (double y) {
	if (PARAMETER[P_M_ROTARYMODE].vint == 1 && PARAMETER[P_H_ROTARYAXIS].vint == 0) {
		return y / (PARAMETER[P_MAT_DIAMETER].vdouble * PI) * 360;
	}
	return y;
}

double _Z (double z) {
	if (PARAMETER[P_M_ROTARYMODE].vint == 1) {
		return z + (PARAMETER[P_MAT_DIAMETER].vdouble / 2.0);
	}
	return z;
}

void translateAxisX (double x, char *ret_str) {
	if (PARAMETER[P_M_ROTARYMODE].vint == 1 && PARAMETER[P_H_ROTARYAXIS].vint == 1) {
		double an = x / (PARAMETER[P_MAT_DIAMETER].vdouble * PI) * 360;
		sprintf(ret_str, "%s%f", rotary_axis[PARAMETER[P_H_ROTARYAXIS].vint], an);
	} else {
		sprintf(ret_str, "X%f", x);
	}
}

void translateAxisY (double y, char *ret_str) {
	if (PARAMETER[P_M_ROTARYMODE].vint == 1 && PARAMETER[P_H_ROTARYAXIS].vint == 0) {
		double an = y / (PARAMETER[P_MAT_DIAMETER].vdouble * PI) * 360;
		sprintf(ret_str, "%s%f", rotary_axis[PARAMETER[P_H_ROTARYAXIS].vint], an);
	} else {
		sprintf(ret_str, "Y%f", y);
	}
}

void translateAxisZ (double z, char *ret_str) {
	if (PARAMETER[P_M_ROTARYMODE].vint == 1) {
		sprintf(ret_str, "Z%f", z + (PARAMETER[P_MAT_DIAMETER].vdouble / 2.0));
	} else {
		sprintf(ret_str, "Z%f", z);
	}
}

int object_line_last (int object_num) {
	int num = 0;
	int ret = 0;
	for (num = 0; num < line_last; num++) {
		if (myOBJECTS[object_num].line[num] != 0) {
			ret = num;
		}
	}
	return ret;
}

double get_len (double x1, double y1, double x2, double y2) {
	double dx = x2 - x1;
	double dy = y2 - y1;
	double len = sqrt(dx * dx + dy * dy);
	return len;
}

double set_positive (double val) {
	if (val < 0.0) {
		val *= -1;
	}
	return val;
}

/* set new first line in object */
void resort_object (int object_num, int start) {
	int num4 = 0;
	int num5 = 0;
	int OTEMPLINE[MAX_LINES];
	for (num4 = 0; num4 < line_last; num4++) {
		OTEMPLINE[num4] = 0;
	}
	for (num4 = start; num4 < line_last; num4++) {
		if (myOBJECTS[object_num].line[num4] != 0) {
			OTEMPLINE[num5++] = myOBJECTS[object_num].line[num4];
		}
	}
	for (num4 = 0; num4 < start; num4++) {
		if (myOBJECTS[object_num].line[num4] != 0) {
			OTEMPLINE[num5++] = myOBJECTS[object_num].line[num4];
		}
	}
	for (num4 = 0; num4 < num5; num4++) {
		myOBJECTS[object_num].line[num4] = OTEMPLINE[num4];
	}
}

/* reverse lines in object */
void redir_object (int object_num) {
	int num = 0;
	int num5 = 0;
	int OTEMPLINE[MAX_LINES];
	for (num = 0; num < line_last; num++) {
		OTEMPLINE[num] = 0;
	}
	for (num = line_last - 1; num >= 0; num--) {
		if (myOBJECTS[object_num].line[num] != 0) {
			OTEMPLINE[num5++] = myOBJECTS[object_num].line[num];
			int lnum = myOBJECTS[object_num].line[num];
			line_invert(lnum);
		}
	}
	for (num = 0; num < num5; num++) {
		myOBJECTS[object_num].line[num] = OTEMPLINE[num];
	}
}

double line_len (int lnum) {
	double dx = myLINES[lnum].x2 - myLINES[lnum].x1;
	double dy = myLINES[lnum].y2 - myLINES[lnum].y1;
	double len = sqrt(dx * dx + dy * dy);
	return len;
}

double line_angle (int lnum) {
	double dx = myLINES[lnum].x2 - myLINES[lnum].x1;
	double dy = myLINES[lnum].y2 - myLINES[lnum].y1;
	double alpha = toDeg(atan(dy / dx));
	if (dx < 0 && dy >= 0) {
		alpha = alpha + 180;
	} else if (dx < 0 && dy < 0) {
		alpha = alpha - 180;
	}
	return alpha;
}

double vector_angle (double x1, double y1, double x2, double y2) {
	double dx = x2 - x1;
	double dy = y2 - y1;
	double alpha = toDeg(atan(dy / dx));
	if (dx < 0 && dy >= 0) {
		alpha = alpha + 180;
	} else if (dx < 0 && dy < 0) {
		alpha = alpha - 180;
	}
	return alpha;
}

double line_angle2 (int lnum) {
	double dx = myLINES[lnum].x2 - myLINES[lnum].x1;
	double dy = myLINES[lnum].y2 - myLINES[lnum].y1;
	double alpha = toDeg(atan2(dx, dy));
	return alpha;
}

void add_angle_offset (double *check_x, double *check_y, double radius, double alpha) {
	double angle = toRad(alpha);
	*check_x += radius * cos(angle);
	*check_y += radius * sin(angle);
}

/* optimize dir of object / inside=cw, outside=ccw */
void object_optimize_dir (int object_num) {
	int pipret = 0;
	if (myOBJECTS[object_num].line[0] != 0) {
		if (myOBJECTS[object_num].closed == 1) {
			int lnum = myOBJECTS[object_num].line[0];
			double alpha = line_angle(lnum);
			double len = line_len(lnum);
			double check_x = myLINES[lnum].x1;
			double check_y = myLINES[lnum].y1;
			add_angle_offset(&check_x, &check_y, len / 2.0, alpha);
			if (myOBJECTS[object_num].climb == 0) {
				add_angle_offset(&check_x, &check_y, -0.01, alpha + 90);
			} else {
				add_angle_offset(&check_x, &check_y, 0.01, alpha + 90);
			}
			pipret = point_in_object(object_num, -1, check_x, check_y);
			if (myOBJECTS[object_num].force == 1) {
				if ((pipret == 1 && myOBJECTS[object_num].offset == 2) || (pipret == 0 && myOBJECTS[object_num].offset == 1)) {
					redir_object(object_num);
				}
			} else {
				if ((pipret == 1 && myOBJECTS[object_num].inside == 0) || (pipret == 0 && myOBJECTS[object_num].inside == 1)) {
					redir_object(object_num);
				}
			}
		}
	}
}

int intersect_check (double p0_x, double p0_y, double p1_x, double p1_y, double p2_x, double p2_y, double p3_x, double p3_y, double *i_x, double *i_y) {
	double s02_x, s02_y, s10_x, s10_y, s32_x, s32_y, s_numer, t_numer, denom, t;
	s10_x = p1_x - p0_x;
	s10_y = p1_y - p0_y;
	s02_x = p0_x - p2_x;
	s02_y = p0_y - p2_y;
	s_numer = s10_x * s02_y - s10_y * s02_x;
	if (s_numer < 0) {
		return 0;
	}
	s32_x = p3_x - p2_x;
	s32_y = p3_y - p2_y;
	t_numer = s32_x * s02_y - s32_y * s02_x;
	if (t_numer < 0) {
		return 0;
	}
	denom = s10_x * s32_y - s32_x * s10_y;
	if (s_numer > denom || t_numer > denom) {
		return 0;
	}
	t = t_numer / denom;
	if (i_x != NULL) {
		*i_x = p0_x + (t * s10_x);
	}
	if (i_y != NULL) {
		*i_y = p0_y + (t * s10_y);
	}
	return 1;
}

void intersect (double l1x1, double l1y1, double l1x2, double l1y2, double l2x1, double l2y1, double l2x2, double l2y2, double *x, double *y) {
	double a1 = l1x2 - l1x1;
	double b1 = l2x1 - l2x2;
	double c1 = l2x1 - l1x1;
	double a2 = l1y2 - l1y1;
	double b2 = l2y1 - l2y2;
	double c2 = l2y1 - l1y1;
	double t = (b1 * c2 - b2 * c1) / (a2 * b1 - a1 * b2);
	*x = l1x1 + t * a1;
	*y = l1y1 + t * a2;
	return;
}

void mill_begin (const char* path) {
	char tmp_str[1024];
	// init output
	mill_start_all = 0;
	tool_last = -1;
	mill_distance_xy = 0.0;
	mill_distance_z = 0.0;
	move_distance_xy = 0.0;
	move_distance_z = 0.0;
	mill_last_x = 0.0;
	mill_last_y = 0.0;
	mill_last_z = PARAMETER[P_CUT_SAVE].vdouble;
	if (output_buffer != NULL) {
		free(output_buffer);
		output_buffer = NULL;
	}
	if (postcam_plugin != PARAMETER[P_H_POST].vint) {
		postcam_exit_lua();
		strcpy(output_extension, "ngc");
		strcpy(output_info, "");
		postcam_init_lua(path, postcam_plugins[PARAMETER[P_H_POST].vint]);
		postcam_plugin = PARAMETER[P_H_POST].vint;
		gtk_label_set_text(GTK_LABEL(OutputInfoLabel), output_info);
		snprintf(tmp_str, sizeof(tmp_str), "Output (%s)", output_extension);
		gtk_label_set_text(GTK_LABEL(gCodeViewLabel), tmp_str);
		postcam_load_source(postcam_plugins[PARAMETER[P_H_POST].vint]);
	}
	postcam_var_push_string("fileName", PARAMETER[P_V_DXF].vstr);
	postcam_var_push_string("postName", postcam_plugins[PARAMETER[P_H_POST].vint]);
	postcam_var_push_string("date", "---");
	postcam_var_push_double("metric", 1.0);
	postcam_var_push_int("feedRate", PARAMETER[P_M_PLUNGE_SPEED].vint);
	postcam_var_push_int("spindleSpeed", PARAMETER[P_TOOL_SPEED].vint);
	postcam_var_push_double("currentX", _X(0.0));
	postcam_var_push_double("currentY", _Y(0.0));
	postcam_var_push_double("currentZ", _Z(0.0));
	postcam_var_push_double("endX", _X(0.0));
	postcam_var_push_double("endY", _Y(0.0));
	postcam_var_push_double("endZ", _Z(0.0));
	postcam_var_push_double("toolOffset", 0.0);
	postcam_var_push_int("tool", -1);
	postcam_var_push_int("lastinst", 0);
	postcam_var_push_int("velocityMode", PARAMETER[P_M_VELOCITYMODE].vint);
	postcam_var_push_double("blendingTolerance", PARAMETER[P_M_BLENDINGTOLERANCE].vdouble);
//	SetupShowGcode(fd_out);
	postcam_call_function("OnInit");
	if (PARAMETER[P_M_ROTARYMODE].vint == 1) {
		if (PARAMETER[P_H_ROTARYAXIS].vint == 1) {
			postcam_var_push_string("axisX", "B");
		} else {
			postcam_var_push_string("axisX", "X");
		}
		if (PARAMETER[P_H_ROTARYAXIS].vint == 0) {
			postcam_var_push_string("axisY", "A");
		} else {
			postcam_var_push_string("axisY", "Y");
		}
	} else {
		postcam_var_push_string("axisX", "X");
		postcam_var_push_string("axisY", "Y");
	}
	postcam_var_push_string("axisZ", "Z");
}

void mill_z (int gcmd, double z) {
	postcam_var_push_int("feedRate", PARAMETER[P_M_PLUNGE_SPEED].vint);
	postcam_var_push_double("currentX", _X(mill_last_x));
	postcam_var_push_double("currentY", _Y(mill_last_y));
	postcam_var_push_double("currentZ", _Z(mill_last_z));
	postcam_var_push_double("endX", _X(mill_last_x));
	postcam_var_push_double("endY", _Y(mill_last_y));
	postcam_var_push_double("endZ", _Z(z));
	if (gcmd == 0) {
		move_distance_z += set_positive(z - mill_last_z);
		postcam_call_function("OnRapid");
	} else {
		mill_distance_z += set_positive(z - mill_last_z);
		postcam_call_function("OnMove");
	}
	if (mill_start_all != 0) {
		glColor4f(0.0, 1.0, 1.0, 1.0);
		if (PARAMETER[P_M_ROTARYMODE].vint == 1) {
			draw_line_wrap_conn((float)mill_last_x, (float)mill_last_y, (float)mill_last_z, (float)z);
		} else {
			glBegin(GL_LINES);
			glVertex3f((float)mill_last_x, (float)mill_last_y, (float)mill_last_z);
			glVertex3f((float)mill_last_x, (float)mill_last_y, (float)z);
			glEnd();
		}
	}
	mill_last_z = z;
}

void mill_objects (void) {
	int object_num = 0;
	// update Object-Data
	for (object_num = 0; object_num < object_last; object_num++) {
		if (myOBJECTS[object_num].force == 0) {
			myOBJECTS[object_num].order = 5;
			myOBJECTS[object_num].tabs = PARAMETER[P_T_USE].vint;
			myOBJECTS[object_num].depth = PARAMETER[P_M_DEPTH].vdouble;
			myOBJECTS[object_num].overcut = PARAMETER[P_M_OVERCUT].vint;
			myOBJECTS[object_num].laser = PARAMETER[P_M_LASERMODE].vint;
			myOBJECTS[object_num].climb = PARAMETER[P_M_CLIMB].vint;
			if (PARAMETER[P_M_LASERMODE].vint == 1) {
				myOBJECTS[object_num].laser = 1;
			}
			if (strncmp(myOBJECTS[object_num].layer, "depth-", 6) == 0) {
				myOBJECTS[object_num].depth = atof(myOBJECTS[object_num].layer + 5);
			}
			if (strncmp(myOBJECTS[object_num].layer, "laser", 5) == 0) {
				myOBJECTS[object_num].laser = 1;
			}
			if (PARAMETER[P_M_NOOFFSET].vint == 1) {
				myOBJECTS[object_num].offset = OFFSET_NONE;
			} else if (myOBJECTS[object_num].closed == 1) {
				if (myOBJECTS[object_num].inside == 1) {
					myOBJECTS[object_num].offset = OFFSET_INSIDE;
					if (strstr(myOBJECTS[object_num].layer, "do_pocket") > 0) {
						myOBJECTS[object_num].pocket = 1;
					} else {
						myOBJECTS[object_num].pocket = PARAMETER[P_M_POCKET].vint;
					}
				} else {
					myOBJECTS[object_num].offset = OFFSET_OUTSIDE;
					myOBJECTS[object_num].pocket = 0;
				}
			} else {
				myOBJECTS[object_num].offset = OFFSET_NONE;
				myOBJECTS[object_num].pocket = 0;
			}
		}
	}

	/* 'shortest' path / first inside than outside objects */ 
	double last_x = 0.0;
	double last_y = 0.0;
	double next_x = 0.0;
	double next_y = 0.0;
	for (object_num = 0; object_num < object_last; object_num++) {
		myOBJECTS[object_num].visited = 0;
	}

	// manuel order < 0
	int order_num = 0;
	for (order_num = 0; order_num < 5; order_num++) {
		for (object_num = 0; object_num < object_last; object_num++) {
			double shortest_len = 9999999.0;
			int shortest_object = -1;
			int shortest_line = -1;
			int flag = 0;
			int object_num2 = 0;
			for (object_num2 = 0; object_num2 < object_last; object_num2++) {
				if (myOBJECTS[object_num2].order != order_num) {
					continue;
				}
				int nnum = 0;
				if (myLINES[myOBJECTS[object_num2].line[nnum]].type == TYPE_CIRCLE) {
					if (myOBJECTS[object_num2].line[nnum] != 0 && myOBJECTS[object_num2].visited == 0) {
						int lnum2 = myOBJECTS[object_num2].line[nnum];
						double len = get_len(last_x, last_y, myLINES[lnum2].cx - myLINES[lnum2].opt, myLINES[lnum2].cy);
						if (len < shortest_len) {
							shortest_len = len;
							shortest_object = object_num2;
							shortest_line = nnum;
							flag = 1;
						}
					}
				} else {
					for (nnum = 0; nnum < line_last; nnum++) {
						if (myOBJECTS[object_num2].line[nnum] != 0 && myOBJECTS[object_num2].visited == 0) {
							int lnum2 = myOBJECTS[object_num2].line[nnum];
							double len = get_len(last_x, last_y, myLINES[lnum2].x1, myLINES[lnum2].y1);
							if (len < shortest_len) {
								shortest_len = len;
								shortest_object = object_num2;
								shortest_line = nnum;
								flag = 1;
							}
						}
					}
				}
			}
			if (flag == 1) {
				myOBJECTS[shortest_object].visited = 1;
				if (myLINES[myOBJECTS[shortest_object].line[0]].type != TYPE_CIRCLE) {
					resort_object(shortest_object, shortest_line);
					object_optimize_dir(shortest_object);
				}
				if (myLINES[myOBJECTS[shortest_object].line[0]].type == TYPE_MTEXT) {
				} else {
					object_draw_offset(fd_out, shortest_object, &next_x, &next_y);
					object_draw(fd_out, shortest_object);
				}
				last_x = next_x;
				last_y = next_y;
			} else {
				break;
			}
		}
	}

	/* inside and open objects */
	for (object_num = 0; object_num < object_last; object_num++) {
		double shortest_len = 9999999.0;
		int shortest_object = -1;
		int shortest_line = -1;
		int flag = 0;
		int object_num2 = 0;
		for (object_num2 = 0; object_num2 < object_last; object_num2++) {
			if (myOBJECTS[object_num2].order != 5) {
				continue;
			}
			int nnum = 0;
			if (myLINES[myOBJECTS[object_num2].line[nnum]].type == TYPE_CIRCLE) {
				if (myOBJECTS[object_num2].line[nnum] != 0 && ((myOBJECTS[object_num2].force == 1 && myOBJECTS[object_num2].offset == 1) || (myOBJECTS[object_num2].force == 0 && myOBJECTS[object_num2].inside == 1)) && myOBJECTS[object_num2].visited == 0) {
					int lnum2 = myOBJECTS[object_num2].line[nnum];
					double len = get_len(last_x, last_y, myLINES[lnum2].cx - myLINES[lnum2].opt, myLINES[lnum2].cy);
					if (len < shortest_len) {
						shortest_len = len;
						shortest_object = object_num2;
						shortest_line = nnum;
						flag = 1;
					}
				}
			} else {
#pragma omp parallel
{
				for (nnum = 0; nnum < line_last; nnum++) {
					if (myOBJECTS[object_num2].line[nnum] != 0 && ((myOBJECTS[object_num2].force == 1 && myOBJECTS[object_num2].offset == 1) || (myOBJECTS[object_num2].force == 0 && myOBJECTS[object_num2].inside == 1)) && myOBJECTS[object_num2].visited == 0) {
						int lnum2 = myOBJECTS[object_num2].line[nnum];
						double len = get_len(last_x, last_y, myLINES[lnum2].x1, myLINES[lnum2].y1);
						if (len < shortest_len) {
							shortest_len = len;
							shortest_object = object_num2;
							shortest_line = nnum;
							flag = 1;
						}
					}
				}
}
			}
			nnum = 0;
			if (myOBJECTS[object_num2].line[nnum] != 0 && myOBJECTS[object_num2].closed == 0 && myOBJECTS[object_num2].visited == 0) {
				int lnum2 = myOBJECTS[object_num2].line[nnum];
				double len = get_len(last_x, last_y, myLINES[lnum2].x1, myLINES[lnum2].y1);
				if (len < shortest_len) {
					shortest_len = len;
					shortest_object = object_num2;
					shortest_line = nnum;
					flag = 1;
				}
			}
			nnum = object_line_last(object_num2);
			if (myOBJECTS[object_num2].line[nnum] != 0 && myOBJECTS[object_num2].closed == 0 && myOBJECTS[object_num2].visited == 0) {
				int lnum2 = myOBJECTS[object_num2].line[nnum];
				double len = get_len(last_x, last_y, myLINES[lnum2].x2, myLINES[lnum2].y2);
				if (len < shortest_len) {
					shortest_len = len;
					shortest_object = object_num2;
					shortest_line = nnum;
					flag = 2;
				}
			}
		}
		if (flag > 0) {
			myOBJECTS[shortest_object].visited = 1;
			if (flag > 1) {
				redir_object(shortest_object);
			}
			if (myOBJECTS[shortest_object].closed == 1 && myLINES[myOBJECTS[shortest_object].line[0]].type != TYPE_CIRCLE) {
				resort_object(shortest_object, shortest_line);
				object_optimize_dir(shortest_object);
			}
			object_draw_offset(fd_out, shortest_object, &next_x, &next_y);
			object_draw(fd_out, shortest_object);
			last_x = next_x;
			last_y = next_y;

		} else {
			break;
		}
	}

	/* outside objects */
	for (object_num = 0; object_num < object_last; object_num++) {
		double shortest_len = 9999999.0;
		int shortest_object = -1;
		int shortest_line = -1;
		int flag = 0;
		int object_num2 = 0;
		for (object_num2 = 0; object_num2 < object_last; object_num2++) {
			if (myOBJECTS[object_num2].order != 5) {
				continue;
			}
			int nnum = 0;
			if (myLINES[myOBJECTS[object_num2].line[nnum]].type == TYPE_CIRCLE) {
				if (myOBJECTS[object_num2].line[nnum] != 0 && myOBJECTS[object_num2].visited == 0) {
					int lnum2 = myOBJECTS[object_num2].line[nnum];
					double len = get_len(last_x, last_y, myLINES[lnum2].cx - myLINES[lnum2].opt, myLINES[lnum2].cy);
					if (len < shortest_len) {
						shortest_len = len;
						shortest_object = object_num2;
						shortest_line = nnum;
						flag = 1;
					}
				}
			} else {
#pragma omp parallel
{
				for (nnum = 0; nnum < line_last; nnum++) {
					if (myOBJECTS[object_num2].line[nnum] != 0 && myOBJECTS[object_num2].visited == 0) {
						int lnum2 = myOBJECTS[object_num2].line[nnum];
						double len = get_len(last_x, last_y, myLINES[lnum2].x1, myLINES[lnum2].y1);
						if (len < shortest_len) {
							shortest_len = len;
							shortest_object = object_num2;
							shortest_line = nnum;
							flag = 1;
						}
					}
				}
}
			}
		}
		if (flag == 1) {
			myOBJECTS[shortest_object].visited = 1;
			if (myLINES[myOBJECTS[shortest_object].line[0]].type != TYPE_CIRCLE) {
				resort_object(shortest_object, shortest_line);
				object_optimize_dir(shortest_object);
			}
			if (myLINES[myOBJECTS[shortest_object].line[0]].type == TYPE_MTEXT) {
			} else {
				object_draw_offset(fd_out, shortest_object, &next_x, &next_y);
				object_draw(fd_out, shortest_object);
			}
			last_x = next_x;
			last_y = next_y;
		} else {
			break;
		}
	}

	// manuel order > 0
	for (order_num = 6; order_num < 10; order_num++) {
		for (object_num = 0; object_num < object_last; object_num++) {
			double shortest_len = 9999999.0;
			int shortest_object = -1;
			int shortest_line = -1;
			int flag = 0;
			int object_num2 = 0;
			for (object_num2 = 0; object_num2 < object_last; object_num2++) {
				if (myOBJECTS[object_num2].order != order_num) {
					continue;
				}
				int nnum = 0;
				if (myLINES[myOBJECTS[object_num2].line[nnum]].type == TYPE_CIRCLE) {
					if (myOBJECTS[object_num2].line[nnum] != 0 && myOBJECTS[object_num2].visited == 0) {
						int lnum2 = myOBJECTS[object_num2].line[nnum];
						double len = get_len(last_x, last_y, myLINES[lnum2].cx - myLINES[lnum2].opt, myLINES[lnum2].cy);
						if (len < shortest_len) {
							shortest_len = len;
							shortest_object = object_num2;
							shortest_line = nnum;
							flag = 1;
						}
					}
				} else {
					for (nnum = 0; nnum < line_last; nnum++) {
						if (myOBJECTS[object_num2].line[nnum] != 0 && myOBJECTS[object_num2].visited == 0) {
							int lnum2 = myOBJECTS[object_num2].line[nnum];
							double len = get_len(last_x, last_y, myLINES[lnum2].x1, myLINES[lnum2].y1);
							if (len < shortest_len) {
								shortest_len = len;
								shortest_object = object_num2;
								shortest_line = nnum;
								flag = 1;
							}
						}
					}
				}
			}
			if (flag == 1) {
				myOBJECTS[shortest_object].visited = 1;
				if (myLINES[myOBJECTS[shortest_object].line[0]].type != TYPE_CIRCLE) {
					resort_object(shortest_object, shortest_line);
					object_optimize_dir(shortest_object);
				}
				if (myLINES[myOBJECTS[shortest_object].line[0]].type == TYPE_MTEXT) {
				} else {
					object_draw_offset(fd_out, shortest_object, &next_x, &next_y);
					object_draw(fd_out, shortest_object);
				}
				last_x = next_x;
				last_y = next_y;
			} else {
				break;
			}
		}
	}

	// show marked lines
	int lnum = 0;
	for (lnum = 0; lnum < line_last; lnum++) {
		if (myLINES[lnum].marked == 1) {
			glLineWidth(20);
			glColor4f(1.0, 0.0, 0.0, 0.5);
			draw_oline((float)myLINES[lnum].x1, (float)myLINES[lnum].y1, (float)myLINES[lnum].x2, (float)myLINES[lnum].y2, 0.1);
			glLineWidth(1);
		}
	}

}

void mill_end (void) {
		postcam_call_function("OnSpindleOff");
		postcam_call_function("OnFinish");
}

void mill_xy (int gcmd, double x, double y, double r, int feed, int object_num, char *comment) {
	if (comment[0] != 0) {
		postcam_comment(comment);
	}
	if (gcmd != 0) {
//		int hflag = 0;
		postcam_var_push_int("feedRate", feed);
		postcam_var_push_double("currentZ", _Z(mill_last_z));
		postcam_var_push_double("currentX", _X(mill_last_x));
		postcam_var_push_double("currentY", _Y(mill_last_y));
		postcam_var_push_double("endZ", _Z(mill_last_z));
		if (gcmd == 1) {
			double i_x = 0.0;
			double i_y = 0.0;
			int num = 0;
			int numr = 0;
			if (myOBJECTS[object_num].tabs == 1 && myOBJECTS[object_num].depth <= PARAMETER[P_M_DEPTH].vdouble && ((myOBJECTS[object_num].offset == OFFSET_OUTSIDE && PARAMETER[P_T_OUTSIDE].vint == 1) || (myOBJECTS[object_num].offset == OFFSET_INSIDE && PARAMETER[P_T_INSIDE].vint == 1) || (myOBJECTS[object_num].offset == OFFSET_NONE && PARAMETER[P_T_OPEN].vint == 1))) {
				if (PARAMETER[P_T_XGRID].vdouble == 0.0 && PARAMETER[P_T_YGRID].vdouble == 0.0) {
					int line_flag[MAX_LINES];
					for (num = 0; num < line_last; num++) {
						line_flag[num] = 0;
					}
					for (numr = 0; numr < line_last; numr++) {
						int min_dist_line = -1;
						double min_dist = 999999.0;
						for (num = 0; num < line_last; num++) {
							if (myLINES[num].istab == 1 && line_flag[num] == 0) {
								if (mill_last_z < PARAMETER[P_T_DEPTH].vdouble && (intersect_check(mill_last_x, mill_last_y, x, y, myLINES[num].x1 + 0.0002, myLINES[num].y1 + 0.0002, myLINES[num].x2 + 0.0002, myLINES[num].y2 + 0.0002, &i_x, &i_y) == 1 || intersect_check(x, y, mill_last_x, mill_last_y, myLINES[num].x1 + 0.0002, myLINES[num].y1 + 0.0002, myLINES[num].x2 + 0.0002, myLINES[num].y2 + 0.0002, &i_x, &i_y) == 1)) {
									double dist = set_positive(get_len(mill_last_x, mill_last_y, i_x, i_y));
									if (min_dist > dist) {
										min_dist = dist;
										min_dist_line = num;
									}
								} else {
									line_flag[num] = 1;
								}
							}
						}
						if (min_dist_line != -1) {
							line_flag[min_dist_line] = 1;
							num = min_dist_line;
							if (mill_last_z < PARAMETER[P_T_DEPTH].vdouble && (intersect_check(mill_last_x, mill_last_y, x, y, myLINES[num].x1 + 0.0002, myLINES[num].y1 + 0.0002, myLINES[num].x2 + 0.0002, myLINES[num].y2 + 0.0002, &i_x, &i_y) == 1 || intersect_check(x, y, mill_last_x, mill_last_y, myLINES[num].x1 + 0.0002, myLINES[num].y1 + 0.0002, myLINES[num].x2 + 0.0002, myLINES[num].y2 + 0.0002, &i_x, &i_y) == 1)) {
								double alpha1 = vector_angle(mill_last_x, mill_last_y, i_x, i_y);
								double i_x2 = i_x;
								double i_y2 = i_y;
								add_angle_offset(&i_x2, &i_y2, (PARAMETER[P_T_LEN].vdouble + PARAMETER[P_TOOL_DIAMETER].vdouble) / 2.0, alpha1 + 180);
								double alpha2 = vector_angle(x, y, i_x, i_y);
								double i_x3 = i_x;
								double i_y3 = i_y;
								add_angle_offset(&i_x3, &i_y3, (PARAMETER[P_T_LEN].vdouble + PARAMETER[P_TOOL_DIAMETER].vdouble) / 2.0, alpha2 + 180);
								draw_line((float)mill_last_x, (float)mill_last_y, (float)mill_last_z, (float)i_x2, (float)i_y2, (float)mill_last_z, PARAMETER[P_TOOL_DIAMETER].vdouble);
								draw_line((float)i_x2, (float)i_y2, (float)PARAMETER[P_T_DEPTH].vdouble, (float)i_x3, (float)i_y3, PARAMETER[P_T_DEPTH].vdouble, PARAMETER[P_TOOL_DIAMETER].vdouble);
//								hflag = 1;
								postcam_var_push_double("endX", _X(i_x2));
								postcam_var_push_double("endY", _Y(i_y2));
								postcam_call_function("OnMove");
								postcam_var_push_double("currentX", _X(i_x2));
								postcam_var_push_double("currentY", _Y(i_y2));
								if (PARAMETER[P_T_TYPE].vint == 0) {
									postcam_var_push_double("endZ", _Z(PARAMETER[P_T_DEPTH].vdouble));
									postcam_call_function("OnMove");
									postcam_var_push_double("currentZ", _Z(PARAMETER[P_T_DEPTH].vdouble));
									postcam_var_push_double("endX", _X(i_x3));
									postcam_var_push_double("endY", _Y(i_y3));
									postcam_call_function("OnMove");
									postcam_var_push_double("currentX", _X(i_x3));
									postcam_var_push_double("currentY", _Y(i_y3));
									postcam_var_push_double("endZ", _Z(mill_last_z));
									postcam_call_function("OnMove");
									postcam_var_push_double("currentZ", _Z(mill_last_z));
								} else {
									postcam_var_push_double("endX", _X(i_x));
									postcam_var_push_double("endY", _Y(i_y));
									postcam_var_push_double("endZ", _Z(PARAMETER[P_T_DEPTH].vdouble));
									postcam_call_function("OnMove");
									postcam_var_push_double("currentX", _X(i_x));
									postcam_var_push_double("currentY", _Y(i_y));
									postcam_var_push_double("currentZ", _Z(PARAMETER[P_T_DEPTH].vdouble));
									postcam_var_push_double("endX", _X(i_x3));
									postcam_var_push_double("endY", _Y(i_y3));
									postcam_var_push_double("endZ", _Z(mill_last_z));
									postcam_call_function("OnMove");
									postcam_var_push_double("currentX", _X(i_x3));
									postcam_var_push_double("currentY", _Y(i_y3));
									postcam_var_push_double("currentZ", _Z(mill_last_z));
								}
								mill_last_x = i_x3;
								mill_last_y = i_y3;
							}
						} else {
							break;
						}
					}
				} else {
					if (PARAMETER[P_T_XGRID].vdouble > 0.0) {
						double nx = 0.0;
						double tx1 = 0.0;
						double ty1 = -10.0;
						double tx2 = size_x;
						double ty2 = size_y + 10.0;
						int nn = (int)size_x / (int)PARAMETER[P_T_XGRID].vdouble;
						double maxn = nn * PARAMETER[P_T_XGRID].vdouble;
						while (1) {
							if (nx > size_x) {
								break;
							}
							if (mill_last_x < x) {
								tx1 = nx;
								tx2 = nx;
							} else {
								tx1 = maxn - nx;
								tx2 = maxn - nx;
							}
							if (PARAMETER[P_M_ROTARYMODE].vint == 0 && PARAMETER[P_T_GRID].vint == 1) {
								glColor4f(0.0, 0.0, 1.0, 0.1);
								glBegin(GL_LINES);
								glVertex3f(tx1, ty1, PARAMETER[P_T_DEPTH].vdouble);
								glVertex3f(tx2, ty2, PARAMETER[P_T_DEPTH].vdouble);
								glEnd();
							}
							if (mill_last_z < PARAMETER[P_T_DEPTH].vdouble && (intersect_check(mill_last_x, mill_last_y, x, y, tx1 + 0.0002, ty1 + 0.0002, tx2 + 0.0002, ty2 + 0.0002, &i_x, &i_y) == 1 || intersect_check(x, y, mill_last_x, mill_last_y, tx1 + 0.0002, ty1 + 0.0002, tx2 + 0.0002, ty2 + 0.0002, &i_x, &i_y) == 1)) {
								double alpha1 = vector_angle(mill_last_x, mill_last_y, i_x, i_y);
								double i_x2 = i_x;
								double i_y2 = i_y;
								add_angle_offset(&i_x2, &i_y2, (PARAMETER[P_T_LEN].vdouble + PARAMETER[P_TOOL_DIAMETER].vdouble) / 2.0, alpha1 + 180);
								double dist = set_positive(get_len(mill_last_x, mill_last_y, i_x, i_y));
								double dist2 = set_positive(get_len(x, y, i_x, i_y));
								if (dist < PARAMETER[P_T_LEN].vdouble || dist2 < PARAMETER[P_T_LEN].vdouble) {
									nx += PARAMETER[P_T_XGRID].vdouble;
									continue;
								}
								double alpha2 = vector_angle(x, y, i_x, i_y);
								double i_x3 = i_x;
								double i_y3 = i_y;
								add_angle_offset(&i_x3, &i_y3, (PARAMETER[P_T_LEN].vdouble + PARAMETER[P_TOOL_DIAMETER].vdouble) / 2.0, alpha2 + 180);
								draw_line((float)mill_last_x, (float)mill_last_y, (float)mill_last_z, (float)i_x2, (float)i_y2, (float)mill_last_z, PARAMETER[P_TOOL_DIAMETER].vdouble);
								draw_line((float)i_x2, (float)i_y2, (float)PARAMETER[P_T_DEPTH].vdouble, (float)i_x3, (float)i_y3, PARAMETER[P_T_DEPTH].vdouble, PARAMETER[P_TOOL_DIAMETER].vdouble);
//								hflag = 1;
								postcam_var_push_double("endX", _X(i_x2));
								postcam_var_push_double("endY", _Y(i_y2));
								postcam_call_function("OnMove");
								postcam_var_push_double("currentX", _X(i_x2));
								postcam_var_push_double("currentY", _Y(i_y2));
								if (PARAMETER[P_T_TYPE].vint == 0) {
									if (PARAMETER[P_M_ROTARYMODE].vint == 0) {
										glColor4f(1.0, 1.0, 0.0, 0.5);
										glBegin(GL_QUADS);
										glVertex3f(i_x2, i_y2 - PARAMETER[P_TOOL_DIAMETER].vdouble, PARAMETER[P_M_DEPTH].vdouble);
										glVertex3f(i_x2, i_y2 - PARAMETER[P_TOOL_DIAMETER].vdouble, PARAMETER[P_T_DEPTH].vdouble);
										glVertex3f(i_x2, i_y2 + PARAMETER[P_TOOL_DIAMETER].vdouble, PARAMETER[P_T_DEPTH].vdouble);
										glVertex3f(i_x2, i_y2 + PARAMETER[P_TOOL_DIAMETER].vdouble, PARAMETER[P_M_DEPTH].vdouble);
										glEnd();
										glBegin(GL_QUADS);
										glVertex3f(i_x2, i_y2 - PARAMETER[P_TOOL_DIAMETER].vdouble, PARAMETER[P_T_DEPTH].vdouble);
										glVertex3f(i_x3, i_y3 - PARAMETER[P_TOOL_DIAMETER].vdouble, PARAMETER[P_T_DEPTH].vdouble);
										glVertex3f(i_x3, i_y3 + PARAMETER[P_TOOL_DIAMETER].vdouble, PARAMETER[P_T_DEPTH].vdouble);
										glVertex3f(i_x2, i_y2 + PARAMETER[P_TOOL_DIAMETER].vdouble, PARAMETER[P_T_DEPTH].vdouble);
										glEnd();
										glBegin(GL_QUADS);
										glVertex3f(i_x3, i_y3 - PARAMETER[P_TOOL_DIAMETER].vdouble, PARAMETER[P_T_DEPTH].vdouble);
										glVertex3f(i_x3, i_y3 - PARAMETER[P_TOOL_DIAMETER].vdouble, PARAMETER[P_M_DEPTH].vdouble);
										glVertex3f(i_x3, i_y3 + PARAMETER[P_TOOL_DIAMETER].vdouble, PARAMETER[P_M_DEPTH].vdouble);
										glVertex3f(i_x3, i_y3 + PARAMETER[P_TOOL_DIAMETER].vdouble, PARAMETER[P_T_DEPTH].vdouble);
										glEnd();
									}
									postcam_var_push_double("endZ", _Z(PARAMETER[P_T_DEPTH].vdouble));
									postcam_call_function("OnMove");
									postcam_var_push_double("currentZ", _Z(PARAMETER[P_T_DEPTH].vdouble));
									postcam_var_push_double("endX", _X(i_x3));
									postcam_var_push_double("endY", _Y(i_y3));
									postcam_call_function("OnMove");
									postcam_var_push_double("currentX", _X(i_x3));
									postcam_var_push_double("currentY", _Y(i_y3));
									postcam_var_push_double("endZ", _Z(mill_last_z));
									postcam_call_function("OnMove");
									postcam_var_push_double("currentZ", _Z(mill_last_z));
								} else {
									if (PARAMETER[P_M_ROTARYMODE].vint == 0) {
										glColor4f(1.0, 1.0, 0.0, 0.5);
										glBegin(GL_QUADS);
										glVertex3f(i_x2, i_y2 - PARAMETER[P_TOOL_DIAMETER].vdouble, PARAMETER[P_M_DEPTH].vdouble);
										glVertex3f(i_x, i_y - PARAMETER[P_TOOL_DIAMETER].vdouble, PARAMETER[P_T_DEPTH].vdouble);
										glVertex3f(i_x, i_y + PARAMETER[P_TOOL_DIAMETER].vdouble, PARAMETER[P_T_DEPTH].vdouble);
										glVertex3f(i_x2, i_y2 + PARAMETER[P_TOOL_DIAMETER].vdouble, PARAMETER[P_M_DEPTH].vdouble);
										glEnd();
										glBegin(GL_QUADS);
										glVertex3f(i_x, i_y - PARAMETER[P_TOOL_DIAMETER].vdouble, PARAMETER[P_T_DEPTH].vdouble);
										glVertex3f(i_x3, i_y3 - PARAMETER[P_TOOL_DIAMETER].vdouble, PARAMETER[P_M_DEPTH].vdouble);
										glVertex3f(i_x3, i_y3 + PARAMETER[P_TOOL_DIAMETER].vdouble, PARAMETER[P_M_DEPTH].vdouble);
										glVertex3f(i_x, i_y + PARAMETER[P_TOOL_DIAMETER].vdouble, PARAMETER[P_T_DEPTH].vdouble);
										glEnd();
									}
									postcam_var_push_double("endX", _X(i_x));
									postcam_var_push_double("endY", _Y(i_y));
									postcam_var_push_double("endZ", _Z(PARAMETER[P_T_DEPTH].vdouble));
									postcam_call_function("OnMove");
									postcam_var_push_double("currentX", _X(i_x));
									postcam_var_push_double("currentY", _Y(i_y));
									postcam_var_push_double("currentZ", _Z(PARAMETER[P_T_DEPTH].vdouble));
									postcam_var_push_double("endX", _X(i_x3));
									postcam_var_push_double("endY", _Y(i_y3));
									postcam_var_push_double("endZ", _Z(mill_last_z));
									postcam_call_function("OnMove");
									postcam_var_push_double("currentX", _X(i_x3));
									postcam_var_push_double("currentY", _Y(i_y3));
									postcam_var_push_double("currentZ", _Z(mill_last_z));
								}
								mill_last_x = i_x3;
								mill_last_y = i_y3;
							}
							nx += PARAMETER[P_T_XGRID].vdouble;
						}
					}
					if (PARAMETER[P_T_YGRID].vdouble > 0.0) {
						double ny = 0.0;
						double tx1 = -10.0;
						double ty1 = 0.0;
						double tx2 = size_x + 10.0;
						double ty2 = size_y;
						int nn = (int)size_y / (int)PARAMETER[P_T_YGRID].vdouble;
						double maxn = nn * PARAMETER[P_T_YGRID].vdouble;
						while (1) {
							if (ny > size_y) {
								break;
							}
							if (mill_last_y < y) {
								ty1 = ny;
								ty2 = ny;
							} else {
								ty1 = maxn - ny;
								ty2 = maxn - ny;
							}
							if (PARAMETER[P_M_ROTARYMODE].vint == 0 && PARAMETER[P_T_GRID].vint == 1) {
								glColor4f(0.0, 0.0, 1.0, 0.1);
								glBegin(GL_LINES);
								glVertex3f(tx1, ty1, PARAMETER[P_T_DEPTH].vdouble);
								glVertex3f(tx2, ty2, PARAMETER[P_T_DEPTH].vdouble);
								glEnd();
							}
							if (mill_last_z < PARAMETER[P_T_DEPTH].vdouble && (intersect_check(mill_last_x, mill_last_y, x, y, tx1 + 0.0002, ty1 + 0.0002, tx2 + 0.0002, ty2 + 0.0002, &i_x, &i_y) == 1 || intersect_check(x, y, mill_last_x, mill_last_y, tx1 + 0.0002, ty1 + 0.0002, tx2 + 0.0002, ty2 + 0.0002, &i_x, &i_y) == 1)) {
								double alpha1 = vector_angle(mill_last_x, mill_last_y, i_x, i_y);
								double i_x2 = i_x;
								double i_y2 = i_y;
								add_angle_offset(&i_x2, &i_y2, (PARAMETER[P_T_LEN].vdouble + PARAMETER[P_TOOL_DIAMETER].vdouble) / 2.0, alpha1 + 180);
								double dist = set_positive(get_len(mill_last_x, mill_last_y, i_x, i_y));
								double dist2 = set_positive(get_len(x, y, i_x, i_y));
								if (dist < PARAMETER[P_T_LEN].vdouble || dist2 < PARAMETER[P_T_LEN].vdouble) {
									ny += PARAMETER[P_T_YGRID].vdouble;
									continue;
								}
								double alpha2 = vector_angle(x, y, i_x, i_y);
								double i_x3 = i_x;
								double i_y3 = i_y;
								add_angle_offset(&i_x3, &i_y3, (PARAMETER[P_T_LEN].vdouble + PARAMETER[P_TOOL_DIAMETER].vdouble) / 2.0, alpha2 + 180);
								draw_line((float)mill_last_x, (float)mill_last_y, (float)mill_last_z, (float)i_x2, (float)i_y2, (float)mill_last_z, PARAMETER[P_TOOL_DIAMETER].vdouble);
								draw_line((float)i_x2, (float)i_y2, (float)PARAMETER[P_T_DEPTH].vdouble, (float)i_x3, (float)i_y3, PARAMETER[P_T_DEPTH].vdouble, PARAMETER[P_TOOL_DIAMETER].vdouble);
//								hflag = 1;
								postcam_var_push_double("endX", _X(i_x2));
								postcam_var_push_double("endY", _Y(i_y2));
								postcam_call_function("OnMove");
								postcam_var_push_double("currentX", _X(i_x2));
								postcam_var_push_double("currentY", _Y(i_y2));
								if (PARAMETER[P_T_TYPE].vint == 0) {
									if (PARAMETER[P_M_ROTARYMODE].vint == 0) {
										glColor4f(1.0, 1.0, 0.0, 0.5);
										glBegin(GL_QUADS);
										glVertex3f(i_x2 - PARAMETER[P_TOOL_DIAMETER].vdouble, i_y2, PARAMETER[P_M_DEPTH].vdouble);
										glVertex3f(i_x2 - PARAMETER[P_TOOL_DIAMETER].vdouble, i_y2, PARAMETER[P_T_DEPTH].vdouble);
										glVertex3f(i_x2 + PARAMETER[P_TOOL_DIAMETER].vdouble, i_y2, PARAMETER[P_T_DEPTH].vdouble);
										glVertex3f(i_x2 + PARAMETER[P_TOOL_DIAMETER].vdouble, i_y2, PARAMETER[P_M_DEPTH].vdouble);
										glEnd();
										glBegin(GL_QUADS);
										glVertex3f(i_x2 - PARAMETER[P_TOOL_DIAMETER].vdouble, i_y2, PARAMETER[P_T_DEPTH].vdouble);
										glVertex3f(i_x3 - PARAMETER[P_TOOL_DIAMETER].vdouble, i_y3, PARAMETER[P_T_DEPTH].vdouble);
										glVertex3f(i_x3 + PARAMETER[P_TOOL_DIAMETER].vdouble, i_y3, PARAMETER[P_T_DEPTH].vdouble);
										glVertex3f(i_x2 + PARAMETER[P_TOOL_DIAMETER].vdouble, i_y2, PARAMETER[P_T_DEPTH].vdouble);
										glEnd();
										glBegin(GL_QUADS);
										glVertex3f(i_x3 - PARAMETER[P_TOOL_DIAMETER].vdouble, i_y3, PARAMETER[P_T_DEPTH].vdouble);
										glVertex3f(i_x3 - PARAMETER[P_TOOL_DIAMETER].vdouble, i_y3, PARAMETER[P_M_DEPTH].vdouble);
										glVertex3f(i_x3 + PARAMETER[P_TOOL_DIAMETER].vdouble, i_y3, PARAMETER[P_M_DEPTH].vdouble);
										glVertex3f(i_x3 + PARAMETER[P_TOOL_DIAMETER].vdouble, i_y3, PARAMETER[P_T_DEPTH].vdouble);
										glEnd();
									}
									postcam_var_push_double("endZ", _Z(PARAMETER[P_T_DEPTH].vdouble));
									postcam_call_function("OnMove");
									postcam_var_push_double("currentZ", _Z(PARAMETER[P_T_DEPTH].vdouble));
									postcam_var_push_double("endX", _X(i_x3));
									postcam_var_push_double("endY", _Y(i_y3));
									postcam_call_function("OnMove");
									postcam_var_push_double("currentX", _X(i_x3));
									postcam_var_push_double("currentY", _Y(i_y3));
									postcam_var_push_double("endZ", _Z(mill_last_z));
									postcam_call_function("OnMove");
									postcam_var_push_double("currentZ", _Z(mill_last_z));
								} else {
									if (PARAMETER[P_M_ROTARYMODE].vint == 0) {
										glColor4f(1.0, 1.0, 0.0, 0.5);
										glBegin(GL_QUADS);
										glVertex3f(i_x2 - PARAMETER[P_TOOL_DIAMETER].vdouble, i_y2, PARAMETER[P_M_DEPTH].vdouble);
										glVertex3f(i_x - PARAMETER[P_TOOL_DIAMETER].vdouble, i_y, PARAMETER[P_T_DEPTH].vdouble);
										glVertex3f(i_x + PARAMETER[P_TOOL_DIAMETER].vdouble, i_y, PARAMETER[P_T_DEPTH].vdouble);
										glVertex3f(i_x2 + PARAMETER[P_TOOL_DIAMETER].vdouble, i_y2, PARAMETER[P_M_DEPTH].vdouble);
										glEnd();
										glBegin(GL_QUADS);
										glVertex3f(i_x - PARAMETER[P_TOOL_DIAMETER].vdouble, i_y, PARAMETER[P_T_DEPTH].vdouble);
										glVertex3f(i_x3 - PARAMETER[P_TOOL_DIAMETER].vdouble, i_y3, PARAMETER[P_M_DEPTH].vdouble);
										glVertex3f(i_x3 + PARAMETER[P_TOOL_DIAMETER].vdouble, i_y3, PARAMETER[P_M_DEPTH].vdouble);
										glVertex3f(i_x + PARAMETER[P_TOOL_DIAMETER].vdouble, i_y, PARAMETER[P_T_DEPTH].vdouble);
										glEnd();
									}
									postcam_var_push_double("endX", _X(i_x));
									postcam_var_push_double("endY", _Y(i_y));
									postcam_var_push_double("endZ", _Z(PARAMETER[P_T_DEPTH].vdouble));
									postcam_call_function("OnMove");
									postcam_var_push_double("currentX", _X(i_x));
									postcam_var_push_double("currentY", _Y(i_y));
									postcam_var_push_double("currentZ", _Z(PARAMETER[P_T_DEPTH].vdouble));
									postcam_var_push_double("endX", _X(i_x3));
									postcam_var_push_double("endY", _Y(i_y3));
									postcam_var_push_double("endZ", _Z(mill_last_z));
									postcam_call_function("OnMove");
									postcam_var_push_double("currentX", _X(i_x3));
									postcam_var_push_double("currentY", _Y(i_y3));
									postcam_var_push_double("currentZ", _Z(mill_last_z));
								}
								mill_last_x = i_x3;
								mill_last_y = i_y3;
							}
							ny += PARAMETER[P_T_YGRID].vdouble;
						}
					}
				}
			}
		}
		draw_line((float)mill_last_x, (float)mill_last_y, (float)mill_last_z, (float)x, (float)y, (float)mill_last_z, PARAMETER[P_TOOL_DIAMETER].vdouble);
		if (gcmd == 1) {
			postcam_var_push_double("endX", _X(x));
			postcam_var_push_double("endY", _Y(y));
			postcam_call_function("OnMove");
		} else if (gcmd == 2 || gcmd == 3) {
			postcam_var_push_double("endX", _X(x));
			postcam_var_push_double("endY", _Y(y));
			double e = x - mill_last_x;
			double f = y - mill_last_y;
			double p = sqrt(e*e + f*f);
			double k = (p*p + r*r - r*r) / (2 * p);
			if (gcmd == 2) {
				double cx = mill_last_x + e * k/p + (f/p) * sqrt(r * r - k * k);
				double cy = mill_last_y + f * k/p - (e/p) * sqrt(r * r - k * k);
				double dx1 = cx - mill_last_x;
				double dy1 = cy - mill_last_y;
				double alpha1 = toDeg(atan2(dx1, dy1)) + 180.0;
				double dx2 = cx - x;
				double dy2 = cy - y;
				double alpha2 = toDeg(atan2(dx2, dy2)) + 180.0;
				double alpha = alpha2 - alpha1;
				if (alpha >= 360.0) {
					alpha -= 360.0;
				}
				if (alpha < 0.0) {
					alpha += 360.0;
				}
				postcam_var_push_double("arcCentreX", _X(cx));
				postcam_var_push_double("arcCentreY", _Y(cy));
				postcam_var_push_double("arcAngle", toRad(alpha));
			} else {
				double cx = mill_last_x + e * k/p - (f/p) * sqrt(r * r - k * k);
				double cy = mill_last_y + f * k/p + (e/p) * sqrt(r * r - k * k);
				double dx1 = cx - mill_last_x;
				double dy1 = cy - mill_last_y;
				double alpha1 = toDeg(atan2(dx1, dy1)) + 180.0;
				double dx2 = cx - x;
				double dy2 = cy - y;
				double alpha2 = toDeg(atan2(dx2, dy2)) + 180.0;
				double alpha = alpha2 - alpha1;
				if (alpha <= -360.0) {
					alpha += 360.0;
				}
				if (alpha > 0.0) {
					alpha -= 360.0;
				}
				postcam_var_push_double("arcCentreX", _X(cx));
				postcam_var_push_double("arcCentreY", _Y(cy));
				postcam_var_push_double("arcAngle", toRad(alpha));
			}
			postcam_var_push_double("arcRadius", r);
			postcam_call_function("OnArc");
		}
	} else {
		if (mill_start_all != 0) {
			glColor4f(0.0, 1.0, 1.0, 1.0);
			draw_line3((float)mill_last_x, (float)mill_last_y, (float)mill_last_z, (float)x, (float)y, (float)mill_last_z);
		}
		postcam_var_push_int("feedRate", feed);
		postcam_var_push_double("currentX", _X(mill_last_x));
		postcam_var_push_double("currentY", _Y(mill_last_y));
		postcam_var_push_double("currentZ", _Z(mill_last_z));
		postcam_var_push_double("endX", _X(x));
		postcam_var_push_double("endY", _Y(y));
		postcam_var_push_double("endZ", _Z(mill_last_z));
		postcam_call_function("OnRapid");
	}
	if (gcmd == 0) {
		move_distance_xy += set_positive(get_len(mill_last_x, mill_last_y, x, y));
	} else {
		mill_distance_xy += set_positive(get_len(mill_last_x, mill_last_y, x, y));
	}

	mill_start_all = 1;
	mill_last_x = x;
	mill_last_y = y;
}

void mill_drill (double x, double y, double depth, int feed, int object_num, char *comment) {
	if (comment[0] != 0) {
		postcam_comment(comment);
	}
	mill_z(1, depth);
	draw_line(x, y, (float)mill_last_z, (float)x, (float)y, (float)mill_last_z, PARAMETER[P_TOOL_DIAMETER].vdouble);
	mill_z(0, 0.0);
}

void mill_circle (int gcmd, double x, double y, double r, double depth, int feed, int inside, int object_num, char *comment) {
	if (comment[0] != 0) {
		postcam_comment(comment);
	}
	mill_z(1, depth);
	postcam_var_push_int("feedRate", feed);
	postcam_var_push_double("currentX", _X(mill_last_x));
	postcam_var_push_double("currentY", _Y(mill_last_y));
	postcam_var_push_double("currentZ", _Z(mill_last_z));
	postcam_var_push_double("endX", _X(x + r));
	postcam_var_push_double("endY", _Y(y));
	postcam_var_push_double("endZ", _Z(mill_last_z));
	postcam_var_push_double("arcRadius", r);
	postcam_var_push_double("arcCentreX", _X(x));
	postcam_var_push_double("arcCentreY", _Y(y));
	if (gcmd == 2) {
		postcam_var_push_double("arcAngle", 360.0);
	} else {
		postcam_var_push_double("arcAngle", -360.0);
	}
	postcam_call_function("OnArc");
	postcam_var_push_double("currentX", _X(x + r));
	postcam_var_push_double("endX", _X(x - r));
	postcam_call_function("OnArc");
	float an = 0.0;
	float last_x = x + r;
	float last_y = y;
	for (an = 0.0; an <= 360.0; an += 18.0) {
		float angle1 = toRad(an);
		float x1 = r * cos(angle1);
		float y1 = r * sin(angle1);
		if (inside == 1) {
			if (gcmd == 3) {
				draw_line(last_x, last_y, (float)mill_last_z, (float)x + x1, (float)y + y1, (float)mill_last_z, PARAMETER[P_TOOL_DIAMETER].vdouble);
			} else {
				draw_line((float)x + x1, (float)y + y1, (float)mill_last_z, last_x, last_y, (float)mill_last_z, PARAMETER[P_TOOL_DIAMETER].vdouble);
			}
		} else {
			if (gcmd == 2) {
				draw_line(last_x, last_y, (float)mill_last_z, (float)x + x1, (float)y + y1, (float)mill_last_z, PARAMETER[P_TOOL_DIAMETER].vdouble);
			} else {
				draw_line((float)x + x1, (float)y + y1, (float)mill_last_z, last_x, last_y, (float)mill_last_z, PARAMETER[P_TOOL_DIAMETER].vdouble);
			}
		}
		last_x = (float)x + x1;
		last_y = (float)y + y1;
	}
	mill_distance_xy += set_positive(r * 2 * PI);
}

void mill_move_in (double x, double y, double depth, int lasermode, int object_num) {
	// move to
	if (lasermode == 1) {
		if (tool_last != 5) {
			postcam_call_function("OnSpindleOff");
			postcam_var_push_double("tool", PARAMETER[P_TOOL_NUM].vint);
			char tmp_str[1024];
			snprintf(tmp_str, sizeof(tmp_str), "Tool# %i", PARAMETER[P_TOOL_NUM].vint);
			postcam_var_push_string("toolName", tmp_str);
			postcam_call_function("OnToolChange");
			postcam_var_push_int("spindleSpeed", PARAMETER[P_TOOL_SPEED].vint);
		}
		tool_last = PARAMETER[P_TOOL_NUM].vint;
		mill_xy(0, x, y, 0.0, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
		mill_z(0, 0.0);
		postcam_call_function("OnSpindleCW");
	} else {
		if (tool_last != PARAMETER[P_TOOL_NUM].vint) {
			postcam_call_function("OnSpindleOff");
			postcam_var_push_double("tool", PARAMETER[P_TOOL_NUM].vint);
			char tmp_str[1024];
			snprintf(tmp_str, sizeof(tmp_str), "Tool# %i", PARAMETER[P_TOOL_NUM].vint);
			postcam_var_push_string("toolName", tmp_str);
			postcam_var_push_double("endZ", _Z(mill_last_z));
			postcam_var_push_int("spindleSpeed", PARAMETER[P_TOOL_SPEED].vint);
			postcam_call_function("OnToolChange");
			postcam_call_function("OnSpindleCW");
		}
		tool_last = PARAMETER[P_TOOL_NUM].vint;
		mill_z(0, PARAMETER[P_CUT_SAVE].vdouble);
		mill_xy(0, x, y, 0.0, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
	}
}

void mill_move_out (int lasermode, int object_num) {
	// move out
	if (lasermode == 1) {
		postcam_call_function("OnSpindleOff");
	} else {
		mill_z(0, PARAMETER[P_CUT_SAVE].vdouble);
	}
}

void object_draw (FILE *fd_out, int object_num) {
	int num = 0;
	int lasermode = 0;
	double mill_depth_real = 0.0;
	char tmp_str[1024];

	lasermode = myOBJECTS[object_num].laser;
	mill_depth_real = myOBJECTS[object_num].depth;

	glLoadName(object_num);


	if (PARAMETER[P_O_SELECT].vint == object_num) {
		glColor4f(1.0, 0.0, 0.0, 0.3);
		glBegin(GL_QUADS);
		glVertex3f(myOBJECTS[object_num].min_x - PARAMETER[P_TOOL_DIAMETER].vdouble, myOBJECTS[object_num].min_y - PARAMETER[P_TOOL_DIAMETER].vdouble, PARAMETER[P_M_DEPTH].vdouble);
		glVertex3f(myOBJECTS[object_num].max_x + PARAMETER[P_TOOL_DIAMETER].vdouble, myOBJECTS[object_num].min_y - PARAMETER[P_TOOL_DIAMETER].vdouble, PARAMETER[P_M_DEPTH].vdouble);
		glVertex3f(myOBJECTS[object_num].max_x + PARAMETER[P_TOOL_DIAMETER].vdouble, myOBJECTS[object_num].max_y + PARAMETER[P_TOOL_DIAMETER].vdouble, PARAMETER[P_M_DEPTH].vdouble);
		glVertex3f(myOBJECTS[object_num].min_x - PARAMETER[P_TOOL_DIAMETER].vdouble, myOBJECTS[object_num].max_y + PARAMETER[P_TOOL_DIAMETER].vdouble, PARAMETER[P_M_DEPTH].vdouble);
		glEnd();
	}
	if (PARAMETER[P_M_NCDEBUG].vint == 1) {
	}
	if (myLINES[myOBJECTS[object_num].line[0]].type == TYPE_CIRCLE) {
		int lnum = myOBJECTS[object_num].line[0];
		double an = 0.0;
		double r = (float)myLINES[lnum].opt;
		double x = (float)myLINES[lnum].cx;
		double y = (float)myLINES[lnum].cy;
		double last_x = x + r;
		double last_y = y;

		if (PARAMETER[P_M_NCDEBUG].vint == 1) {
			mill_move_in(myLINES[lnum].cx - r, myLINES[lnum].cy, 0.0, lasermode, object_num);
			mill_circle(2, myLINES[lnum].cx, myLINES[lnum].cy, r, 0.0, PARAMETER[P_M_FEEDRATE].vint, myOBJECTS[object_num].inside, object_num, "");
		}
		for (an = 0.0; an <= 360.0; an += 9.0) {
			double angle1 = toRad(an);
			double x1 = r * cos(angle1);
			double y1 = r * sin(angle1);
			if (PARAMETER[P_O_SELECT].vint == object_num) {
				glColor4f(1.0, 0.0, 0.0, 1.0);
			} else {
				glColor4f(0.0, 1.0, 0.0, 1.0);
			}
			draw_oline(last_x, last_y, (float)x + x1, (float)y + y1, mill_depth_real);
			last_x = (float)x + x1;
			last_y = (float)y + y1;
		}
		if (PARAMETER[P_M_ROTARYMODE].vint == 0) {
			if (object_num == PARAMETER[P_O_SELECT].vint) {
				glColor4f(1.0, 0.0, 0.0, 1.0);
			} else {
				glColor4f(1.0, 1.0, 1.0, 1.0);
			}
			snprintf(tmp_str, sizeof(tmp_str), "%i", object_num);
			output_text_gl_center(tmp_str, (float)x + (float)r, (float)y, PARAMETER[P_CUT_SAVE].vdouble, 0.2);
			if (PARAMETER[P_V_HELPLINES].vint == 1) {
				if (myOBJECTS[object_num].closed == 1 && myOBJECTS[object_num].inside == 0) {
					object2poly(object_num, 0.0, mill_depth_real, 0);
				} else if (myOBJECTS[object_num].inside == 1 && mill_depth_real > PARAMETER[P_M_DEPTH].vdouble) {
					object2poly(object_num, mill_depth_real - 0.001, mill_depth_real - 0.001, 1);
				}
			}
		}
		return;
	}

	for (num = 0; num < line_last; num++) {
		if (myOBJECTS[object_num].line[num] != 0) {
			int lnum = myOBJECTS[object_num].line[num];
			if (PARAMETER[P_O_SELECT].vint == object_num) {
				glColor4f(1.0, 0.0, 0.0, 1.0);
			} else {
				glColor4f(0.0, 1.0, 0.0, 1.0);
			}
			draw_oline((float)myLINES[lnum].x1, (float)myLINES[lnum].y1, (float)myLINES[lnum].x2, (float)myLINES[lnum].y2, mill_depth_real);
			if (myOBJECTS[object_num].closed == 0 && (myLINES[lnum].type != TYPE_MTEXT || PARAMETER[P_M_TEXT].vint == 1)) {
				draw_line2((float)myLINES[lnum].x1, (float)myLINES[lnum].y1, 0.01, (float)myLINES[lnum].x2, (float)myLINES[lnum].y2, 0.01, (PARAMETER[P_TOOL_DIAMETER].vdouble));
			}
			glLineWidth(1);
			if (PARAMETER[P_M_NCDEBUG].vint == 1) {
				if (num == 0) {
					if (lasermode == 1) {
						if (tool_last != 5) {
							postcam_call_function("OnSpindleOff");
							postcam_var_push_int("tool", 5);
							char tmp_str[1024];
							snprintf(tmp_str, sizeof(tmp_str), "Tool# %i", PARAMETER[P_TOOL_NUM].vint);
							postcam_var_push_string("toolName", tmp_str);
							postcam_call_function("OnToolChange");
						}
						tool_last = 5;
					}
					mill_xy(0, myLINES[lnum].x1, myLINES[lnum].y1, 0.0, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
					if (lasermode == 1) {
						mill_z(0, 0.0);
						postcam_call_function("OnSpindleCW");
					}
				}
				if (myLINES[lnum].type == TYPE_ARC || myLINES[lnum].type == TYPE_CIRCLE) {
					if (myOBJECTS[object_num].climb == 0) {
						if (myLINES[lnum].opt < 0) {
							mill_xy(3, myLINES[lnum].x2, myLINES[lnum].y2, myLINES[lnum].opt * -1, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
						} else {
							mill_xy(2, myLINES[lnum].x2, myLINES[lnum].y2, myLINES[lnum].opt, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
						}
					} else {
						if (myLINES[lnum].opt < 0) {
							mill_xy(2, myLINES[lnum].x2, myLINES[lnum].y2, myLINES[lnum].opt * -1, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
						} else {
							mill_xy(3, myLINES[lnum].x2, myLINES[lnum].y2, myLINES[lnum].opt, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
						}
					}
				} else if (myLINES[lnum].type == TYPE_MTEXT) {
					if (PARAMETER[P_M_TEXT].vint == 1) {
						mill_xy(1, myLINES[lnum].x2, myLINES[lnum].y2, 0.0, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
					}
				} else {
					mill_xy(1, myLINES[lnum].x2, myLINES[lnum].y2, 0.0, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
				}
			}
			if (num == 0) {
				if (myLINES[lnum].type != TYPE_MTEXT || PARAMETER[P_M_TEXT].vint == 1) {
					if (PARAMETER[P_M_ROTARYMODE].vint == 0) {
						if (object_num == PARAMETER[P_O_SELECT].vint) {
							glColor4f(1.0, 0.0, 0.0, 1.0);
						} else {
							glColor4f(1.0, 1.0, 1.0, 1.0);
						}
						snprintf(tmp_str, sizeof(tmp_str), "%i", object_num);
						output_text_gl_center(tmp_str, (float)myLINES[lnum].x1, (float)myLINES[lnum].y1, PARAMETER[P_CUT_SAVE].vdouble, 0.2);
					}
				}
			}
		}
	}
	if (PARAMETER[P_M_ROTARYMODE].vint == 0) {
		if (PARAMETER[P_V_HELPLINES].vint == 1) {
			if (myOBJECTS[object_num].closed == 1 && myOBJECTS[object_num].offset == 2) {
				object2poly(object_num, 0.0, mill_depth_real, 0);
			} else if (myOBJECTS[object_num].inside == 1 && mill_depth_real > PARAMETER[P_M_DEPTH].vdouble) {
				object2poly(object_num, mill_depth_real - 0.001, mill_depth_real - 0.001, 1);
			}
		}
	}
}

void object_draw_offset_depth (FILE *fd_out, int object_num, double depth, double *next_x, double *next_y, double tool_offset, int overcut, int lasermode, int offset) {
	int error = 0;
	int lnum1 = 0;
	int lnum2 = 0;
	int num = 0;
	int last = 0;
	int last_lnum = 0;
	double first_x = 0.0;
	double first_y = 0.0;
	double last_x = 0.0;
	double last_y = 0.0;
	glLoadName(object_num);

	/* find last line in object */
	for (num = 0; num < line_last; num++) {
		if (myOBJECTS[object_num].line[num] != 0) {
			last = myOBJECTS[object_num].line[num];
		}
	}
	if (myLINES[last].type == TYPE_MTEXT && PARAMETER[P_M_TEXT].vint == 0) {
		return;
	}

	if (myLINES[myOBJECTS[object_num].line[0]].type == TYPE_CIRCLE) {
		int lnum = myOBJECTS[object_num].line[0];
		double r = myLINES[lnum].opt;
		if (r < 0.0) {
			r *= -1;
		}
		if (r > PARAMETER[P_TOOL_DIAMETER].vdouble / 2.0) {
			if (offset == 1) {
				r -= tool_offset;
			} else if (offset == 2) {
				r += tool_offset;
			}
			if (mill_start == 0) {
				mill_move_in(myLINES[lnum].cx - r, myLINES[lnum].cy, depth, lasermode, object_num);
				mill_start = 1;
			}
			if (myOBJECTS[object_num].climb == 0) {
				mill_circle(2, myLINES[lnum].cx, myLINES[lnum].cy, r, depth, PARAMETER[P_M_FEEDRATE].vint, myOBJECTS[object_num].inside, object_num, "");
			} else {
				mill_circle(3, myLINES[lnum].cx, myLINES[lnum].cy, r, depth, PARAMETER[P_M_FEEDRATE].vint, myOBJECTS[object_num].inside, object_num, "");
			}
			*next_x = myLINES[lnum].cx - r;
			*next_y = myLINES[lnum].cy;
		} else {

			if (mill_start == 0) {
				mill_move_in(myLINES[lnum].cx, myLINES[lnum].cy, depth, lasermode, object_num);
				mill_start = 1;
			}
			mill_drill(myLINES[lnum].cx, myLINES[lnum].cy, depth, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
			*next_x = myLINES[lnum].cx;
			*next_y = myLINES[lnum].cy;
		}
		return;
	}

	for (num = 0; num < line_last; num++) {
		if (myOBJECTS[object_num].line[num] != 0) {
			if (num == 0) {
				lnum1 = last;
			} else {
				lnum1 = last_lnum;
			}
			lnum2 = myOBJECTS[object_num].line[num];
			if (myOBJECTS[object_num].closed == 1 && offset != 0) {
				// line1 Offsets & Angle
				double alpha1 = line_angle(lnum1);
				double check1_x = myLINES[lnum1].x1;
				double check1_y = myLINES[lnum1].y1;
				double check1b_x = myLINES[lnum1].x2;
				double check1b_y = myLINES[lnum1].y2;
				add_angle_offset(&check1b_x, &check1b_y, 0.0, alpha1);
				if (myOBJECTS[object_num].climb == 0) {
					add_angle_offset(&check1_x, &check1_y, -tool_offset, alpha1 + 90);
					add_angle_offset(&check1b_x, &check1b_y, -tool_offset, alpha1 + 90);
				} else {
					add_angle_offset(&check1_x, &check1_y, tool_offset, alpha1 + 90);
					add_angle_offset(&check1b_x, &check1b_y, tool_offset, alpha1 + 90);
				}

				// line2 Offsets & Angle
				double alpha2 = line_angle(lnum2);
				double check2_x = myLINES[lnum2].x1;
				double check2_y = myLINES[lnum2].y1;
				add_angle_offset(&check2_x, &check2_y, 0.0, alpha2);
				double check2b_x = myLINES[lnum2].x2;
				double check2b_y = myLINES[lnum2].y2;
				if (myOBJECTS[object_num].climb == 0) {
					add_angle_offset(&check2_x, &check2_y, -tool_offset, alpha2 + 90);
					add_angle_offset(&check2b_x, &check2b_y, -tool_offset, alpha2 + 90);
				} else {
					add_angle_offset(&check2_x, &check2_y, tool_offset, alpha2 + 90);
					add_angle_offset(&check2b_x, &check2b_y, tool_offset, alpha2 + 90);
				}

				// Angle-Diff
				alpha1 = alpha1 + 180.0;
				alpha2 = alpha2 + 180.0;
				double alpha_diff = alpha2 - alpha1;
				if (alpha_diff < 0.0) {
					alpha_diff += 360.0;
				}
				if (alpha_diff > 360.0) {
					alpha_diff -= 360.0;
				}
				alpha1 = line_angle2(lnum1);
				alpha2 = line_angle2(lnum2);
				alpha_diff = alpha2 - alpha1;
				if (alpha_diff > 180.0) {
					alpha_diff -= 360.0;
				}
				if (alpha_diff < -180.0) {
					alpha_diff += 360.0;
				}
				if (alpha_diff == 0.0) {
				} else if ((myOBJECTS[object_num].climb == 1 && alpha_diff > 0.0) || (myOBJECTS[object_num].climb == 0 && alpha_diff < 0.0)) {
					// Aussenkante
					if (num == 0) {
						first_x = check1b_x;
						first_y = check1b_y;
						if (mill_start == 0) {
							mill_move_in(first_x, first_y, depth, lasermode, object_num);
							mill_start = 1;
						}
						mill_z(1, depth);

						if (myOBJECTS[object_num].climb == 0) {
							mill_xy(3, check2_x, check2_y, tool_offset, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
						} else {
							mill_xy(2, check2_x, check2_y, tool_offset, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
						}
					} else {
						if (myLINES[lnum1].type == TYPE_ARC || myLINES[lnum1].type == TYPE_CIRCLE) {
							if (myOBJECTS[object_num].climb == 0) {
								if (myLINES[lnum1].opt < 0) {
									mill_xy(2, check1b_x, check1b_y, (myLINES[lnum1].opt + tool_offset) * -1, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
								} else {
									mill_xy(3, check1b_x, check1b_y, (myLINES[lnum1].opt + tool_offset), PARAMETER[P_M_FEEDRATE].vint, object_num, "");
								}
							} else {
								if (myLINES[lnum1].opt < 0) {
									mill_xy(2, check1b_x, check1b_y, (myLINES[lnum1].opt - tool_offset) * -1, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
								} else {
									mill_xy(3, check1b_x, check1b_y, (myLINES[lnum1].opt - tool_offset), PARAMETER[P_M_FEEDRATE].vint, object_num, "");
								}
							}
							if (myLINES[lnum2].type == TYPE_ARC || myLINES[lnum2].type == TYPE_CIRCLE) {
							} else {
								if (myOBJECTS[object_num].climb == 0) {
									mill_xy(3, check2_x, check2_y, tool_offset, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
								} else {
									mill_xy(2, check2_x, check2_y, tool_offset, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
								}
							}
						} else {
							mill_xy(1, check1b_x, check1b_y, 0.0, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
							if (myOBJECTS[object_num].climb == 0) {
								mill_xy(3, check2_x, check2_y, tool_offset, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
							} else {
								mill_xy(2, check2_x, check2_y, tool_offset, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
							}
						}
					}
					last_x = check2_x;
					last_y = check2_y;
				} else {
					// Innenkante
					if (myLINES[lnum2].type == TYPE_ARC || myLINES[lnum2].type == TYPE_CIRCLE) {
						if (myLINES[lnum2].opt < 0 && myLINES[lnum2].opt * -1 < tool_offset) {
							error = 1;
							break;
						}
					}
					double px = 0.0;
					double py = 0.0;
					intersect(check1_x, check1_y, check1b_x, check1b_y, check2_x, check2_y, check2b_x, check2b_y, &px, &py);
					double enx = px;
					double eny = py;
					if (num == 0) {
						first_x = px;
						first_y = py;
						if (mill_start == 0) {
							mill_move_in(first_x, first_y, depth, lasermode, object_num);
							mill_start = 1;
							last_x = first_x;
							last_y = first_y;
						}
						mill_z(1, depth);
						if (overcut == 1 && ((myLINES[lnum1].type == TYPE_LINE && myLINES[lnum2].type == TYPE_LINE) || (myLINES[lnum1].type == TYPE_ELLIPSE && myLINES[lnum2].type == TYPE_ELLIPSE))) {
							double adx = myLINES[lnum2].x1 - px;
							double ady = myLINES[lnum2].y1 - py;
							double aalpha = toDeg(atan(ady / adx));
							if (adx < 0 && ady >= 0) {
								aalpha = aalpha + 180;
							} else if (adx < 0 && ady < 0) {
								aalpha = aalpha - 180;
							}
							double len = sqrt(adx * adx + ady * ady);
							add_angle_offset(&enx, &eny, len - tool_offset, aalpha);
							mill_xy(1, enx, eny, 0.0, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
							mill_xy(1, px, py, 0.0, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
							last_x = px;
							last_y = py;
						}
					} else {
						if (overcut == 1 && ((myLINES[lnum1].type == TYPE_LINE && myLINES[lnum2].type == TYPE_LINE) || (myLINES[lnum1].type == TYPE_ELLIPSE && myLINES[lnum2].type == TYPE_ELLIPSE))) {
							double adx = myLINES[lnum1].x2 - px;
							double ady = myLINES[lnum1].y2 - py;
							double aalpha = toDeg(atan(ady / adx));
							if (adx < 0 && ady >= 0) {
								aalpha = aalpha + 180;
							} else if (adx < 0 && ady < 0) {
								aalpha = aalpha - 180;
							}
							double len = sqrt(adx * adx + ady * ady);
							add_angle_offset(&enx, &eny, len - tool_offset, aalpha);
						}
						if (myLINES[lnum1].type == TYPE_ARC || myLINES[lnum1].type == TYPE_CIRCLE) {
							if (myOBJECTS[object_num].climb == 0) {
								if (myLINES[lnum1].opt < 0) {
									mill_xy(2, px, py, (myLINES[lnum1].opt + tool_offset) * -1, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
								} else {
									mill_xy(3, px, py, (myLINES[lnum1].opt + tool_offset), PARAMETER[P_M_FEEDRATE].vint, object_num, "");
								}
							} else {
								if (myLINES[lnum1].opt < 0) {
									mill_xy(2, px, py, (myLINES[lnum1].opt - tool_offset) * -1, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
								} else {
									mill_xy(3, px, py, (myLINES[lnum1].opt - tool_offset), PARAMETER[P_M_FEEDRATE].vint, object_num, "");
								}
							}
						} else {
							mill_xy(1, px, py, 0.0, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
							if (overcut == 1 && ((myLINES[lnum1].type == TYPE_LINE && myLINES[lnum2].type == TYPE_LINE) || (myLINES[lnum1].type == TYPE_ELLIPSE && myLINES[lnum2].type == TYPE_ELLIPSE))) {
								mill_xy(1, enx, eny, 0.0, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
								mill_xy(1, px, py, 0.0, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
							}
						}
						last_x = px;
						last_y = py;
					}
				}
			} else {
				if (num == 0) {
					first_x = myLINES[lnum2].x1;
					first_y = myLINES[lnum2].y1;
					mill_move_in(first_x, first_y, depth, lasermode, object_num);
					mill_start = 1;
					mill_z(1, depth);
				}
				double alpha1 = line_angle2(lnum1);
				double alpha2 = line_angle2(lnum2);
				double alpha_diff = alpha2 - alpha1;
				if (myLINES[lnum2].type == TYPE_ARC || myLINES[lnum2].type == TYPE_CIRCLE) {
					if (myLINES[lnum2].opt < 0) {
						mill_xy(2, myLINES[lnum2].x2, myLINES[lnum2].y2, myLINES[lnum2].opt * -1, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
					} else {
						mill_xy(3, myLINES[lnum2].x2, myLINES[lnum2].y2, myLINES[lnum2].opt, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
					}
				} else {
					if (PARAMETER[P_M_KNIFEMODE].vint == 1) {
						if (alpha_diff > 180.0) {
							alpha_diff -= 360.0;
						}
						if (alpha_diff < -180.0) {
							alpha_diff += 360.0;
						}
						if (alpha_diff > PARAMETER[P_H_KNIFEMAXANGLE].vdouble || alpha_diff < -PARAMETER[P_H_KNIFEMAXANGLE].vdouble) {
							mill_z(0, PARAMETER[P_CUT_SAVE].vdouble);
							snprintf(cline, sizeof(cline), "TAN: %f\n", alpha2);
							postcam_comment(cline);
							mill_z(1, depth);
						} else {
							snprintf(cline, sizeof(cline), "TAN: %f\n", alpha2);
							postcam_comment(cline);
						}
					}
					mill_xy(1, myLINES[lnum2].x2, myLINES[lnum2].y2, 0.0, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
				}
				last_x = myLINES[lnum2].x2;
				last_y = myLINES[lnum2].y2;
			}
			last_lnum = lnum2;
		}
	}
	if (myOBJECTS[object_num].closed == 1) {
		if (myLINES[last].type == TYPE_ARC || myLINES[last].type == TYPE_CIRCLE) {
			if (myOBJECTS[object_num].climb == 0) {
				if (myLINES[last].opt < 0) {
					mill_xy(2, first_x, first_y, (myLINES[last].opt + tool_offset) * -1, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
				} else {
					mill_xy(3, first_x, first_y, (myLINES[last].opt + tool_offset), PARAMETER[P_M_FEEDRATE].vint, object_num, "");
				}
			} else {
				if (myLINES[last].opt < 0) {
					mill_xy(2, first_x, first_y, (myLINES[last].opt - tool_offset) * -1, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
				} else {
					mill_xy(3, first_x, first_y, (myLINES[last].opt - tool_offset), PARAMETER[P_M_FEEDRATE].vint, object_num, "");
				}
			}
		} else {
			mill_xy(1, first_x, first_y, 0.0, PARAMETER[P_M_FEEDRATE].vint, object_num, "");
		}
		last_x = first_x;
		last_y = first_y;
	}
	*next_x = last_x;
	*next_y = last_y;
	if (error > 0) {
		return;
	}
}


void object_draw_offset (FILE *fd_out, int object_num, double *next_x, double *next_y) {
	double depth = 0.0;
	double tool_offset = 0.0;
	int overcut = 0;
	int lasermode = 0;
	int tangencialmode = 0;
	int offset = 0;
	double mill_depth_real = 0.0;
	if (PARAMETER[P_M_LASERMODE].vint == 1) {
		lasermode = 1;
		PARAMETER[P_M_KNIFEMODE].vint = 0;
	}
	if (PARAMETER[P_M_KNIFEMODE].vint == 1) {
		tangencialmode = 1;
	}

	if (strncmp(myOBJECTS[object_num].layer, "knife", 5) == 0) {
		tangencialmode = 1;
	}

	offset = myOBJECTS[object_num].offset;
	mill_depth_real = myOBJECTS[object_num].depth;
	overcut = myOBJECTS[object_num].overcut;
	lasermode = myOBJECTS[object_num].laser;
	glLoadName(object_num);

	tool_offset = PARAMETER[P_TOOL_DIAMETER].vdouble / 2.0;
	if (myOBJECTS[object_num].use == 0) {
		return;
	}

	if (myOBJECTS[object_num].pocket == 1) {
		mill_pocket(object_num, next_x, next_y);
	}

	postcam_comment("--------------------------------------------------");
	snprintf(cline, sizeof(cline), "Object: #%i", object_num);
	postcam_var_push_string("partName", cline);
	postcam_comment(cline);
	snprintf(cline, sizeof(cline), "Layer: %s", myOBJECTS[object_num].layer);
	postcam_comment(cline);
	snprintf(cline, sizeof(cline), "Overcut: %i",  overcut);
	postcam_comment(cline);
	if (tangencialmode == 1) {
		snprintf(cline, sizeof(cline), "Tangencial-Mode: On");
	} else if (lasermode == 1) {
		snprintf(cline, sizeof(cline), "Laser-Mode: On");
	} else { 
		snprintf(cline, sizeof(cline), "Depth: %f", mill_depth_real);
	}
	postcam_comment(cline);
	if (offset == 0) {
		snprintf(cline, sizeof(cline), "Offset: None");
	} else if (offset == 1) {
		snprintf(cline, sizeof(cline), "Offset: Inside");
	} else {
		snprintf(cline, sizeof(cline), "Offset: Outside");
	}
	postcam_comment(cline);
	postcam_comment("--------------------------------------------------");
	postcam_call_function("OnNewPart");

	mill_start = 0;

	// offset for each depth-step
	double new_depth = 0.0;
	if (lasermode == 1 || tangencialmode == 1) {
		object_draw_offset_depth(fd_out, object_num, 0.0, next_x, next_y, tool_offset, overcut, lasermode, offset);
	} else {
		for (depth = PARAMETER[P_M_Z_STEP].vdouble; depth > mill_depth_real + PARAMETER[P_M_Z_STEP].vdouble; depth += PARAMETER[P_M_Z_STEP].vdouble) {
			if (depth < mill_depth_real) {
				new_depth = mill_depth_real;
			} else {
				new_depth = depth;
			}
			object_draw_offset_depth(fd_out, object_num, new_depth, next_x, next_y, tool_offset, overcut, lasermode, offset);
		}
	}
	mill_move_out(lasermode, object_num);
}

int find_next_line (int object_num, int first, int num, int dir, int depth) {
	int fnum = 0;
	int num4 = 0;
	int num5 = 0;
	double px = 0;
	double py = 0;
	int ret = 0;
	if (dir == 0) {
		px = myLINES[num].x1;
		py = myLINES[num].y1;
	} else {
		px = myLINES[num].x2;
		py = myLINES[num].y2;
	}
//	for (num4 = 0; num4 < depth; num4++) {
//		printf(" ");
//	}
	for (num5 = 0; num5 < object_last; num5++) {
		if (myOBJECTS[num5].line[0] == 0) {
			continue;
		}
		for (num4 = 0; num4 < line_last; num4++) {
			if (myOBJECTS[num5].line[num4] == num) {
//				printf("##LINE %i in OBJECT %i / %i\n", num, num5, num4);
				return 2;
			}
		}
	}
	if (depth > line_last) {
		printf("###### ERROR / CAN'T FIND NEXT LINE ########\n");
		return 0;
	}
	for (num4 = 0; num4 < line_last; num4++) {
		if (myOBJECTS[object_num].line[num4] == 0) {
//			printf("##ADD LINE %i to OBJECT %i / %i\n", num, object_num, num4);
			myOBJECTS[object_num].line[num4] = num;
			strcpy(myOBJECTS[object_num].layer, myLINES[num].layer);
			break;
		}
	}
	int num2 = 0;
	fnum = 0;
#pragma omp parallel
{

	for (num2 = 1; num2 < line_last; num2++) {
		if (myLINES[num2].used == 1 && num != num2 && strcmp(myLINES[num2].layer, myLINES[num].layer) == 0) {
			if (px >= myLINES[num2].x1 - PARAMETER[P_O_TOLERANCE].vdouble && px <= myLINES[num2].x1 + PARAMETER[P_O_TOLERANCE].vdouble && py >= myLINES[num2].y1 - PARAMETER[P_O_TOLERANCE].vdouble && py <= myLINES[num2].y1 + PARAMETER[P_O_TOLERANCE].vdouble) {
//				printf("###### %i NEXT LINE: %f,%f -> %f,%f START\n", depth, myLINES[num2].x1, myLINES[num2].y1, myLINES[num2].x2, myLINES[num2].y2);
				fnum++;
			} else if (px >= myLINES[num2].x2 - PARAMETER[P_O_TOLERANCE].vdouble && px <= myLINES[num2].x2 + PARAMETER[P_O_TOLERANCE].vdouble && py >= myLINES[num2].y2 - PARAMETER[P_O_TOLERANCE].vdouble && py <= myLINES[num2].y2 + PARAMETER[P_O_TOLERANCE].vdouble) {
//				printf("###### %i NEXT LINE: %f,%f -> %f,%f END\n", depth, myLINES[num2].x1, myLINES[num2].y1, myLINES[num2].x2, myLINES[num2].y2);
				fnum++;
			}
		}
	}
}
	for (num2 = 1; num2 < line_last; num2++) {
		if (myLINES[num2].used == 1 && num != num2 && strcmp(myLINES[num2].layer, myLINES[num].layer) == 0) {
			if (px >= myLINES[num2].x1 - PARAMETER[P_O_TOLERANCE].vdouble && px <= myLINES[num2].x1 + PARAMETER[P_O_TOLERANCE].vdouble && py >= myLINES[num2].y1 - PARAMETER[P_O_TOLERANCE].vdouble && py <= myLINES[num2].y1 + PARAMETER[P_O_TOLERANCE].vdouble) {
//				printf("###### %i NEXT LINE: %f,%f -> %f,%f START\n", depth, myLINES[num2].x1, myLINES[num2].y1, myLINES[num2].x2, myLINES[num2].y2);
				if (num2 != first) {
					ret = find_next_line(object_num, first, num2, 1, depth + 1);
					if (ret == 1) {
						return 1;
					}
				} else {
//					printf("###### OBJECT CLOSED\n");
					return 1;
				}
			} else if (px >= myLINES[num2].x2 - PARAMETER[P_O_TOLERANCE].vdouble && px <= myLINES[num2].x2 + PARAMETER[P_O_TOLERANCE].vdouble && py >= myLINES[num2].y2 - PARAMETER[P_O_TOLERANCE].vdouble && py <= myLINES[num2].y2 + PARAMETER[P_O_TOLERANCE].vdouble) {
				line_invert(num2);
//				printf("###### %i NEXT LINE: %f,%f -> %f,%f END\n", depth, myLINES[num2].x1, myLINES[num2].y1, myLINES[num2].x2, myLINES[num2].y2);
				if (num2 != first) {
					ret = find_next_line(object_num, first, num2, 1, depth + 1);
					if (ret == 1) {
						return 1;
					}
				} else {
//					printf("###### OBJECT CLOSED\n");
					return 1;
				}
			}
		}
	}
	return ret;
}

int line_open_check (int num) {
	int ret = 0;
	int dir = 0;
	int num2 = 0;
	int onum = 0;
	double px = 0.0;
	double py = 0.0;
	for (onum = 0; onum < object_last; onum++) {
		if (myOBJECTS[onum].line[0] == 0) {
			break;
		}
		for (num2 = 0; num2 < line_last; num2++) {
			if (myOBJECTS[onum].line[num2] == num) {
//				printf("##LINE %i in OBJECT %i / %i\n", num, onum, num2);
				return 0;
			}
		}
	}
	px = myLINES[num].x1;
	py = myLINES[num].y1;
#pragma omp parallel
{

	for (num2 = 1; num2 < line_last; num2++) {
		if (myLINES[num2].used == 1 && num != num2 && strcmp(myLINES[num2].layer, myLINES[num].layer) == 0) {
			if (px >= myLINES[num2].x1 - PARAMETER[P_O_TOLERANCE].vdouble && px <= myLINES[num2].x1 + PARAMETER[P_O_TOLERANCE].vdouble && py >= myLINES[num2].y1 - PARAMETER[P_O_TOLERANCE].vdouble && py <= myLINES[num2].y1 + PARAMETER[P_O_TOLERANCE].vdouble) {
				ret++;
				dir = 1;
				break;
			} else if (px >= myLINES[num2].x2 - PARAMETER[P_O_TOLERANCE].vdouble && px <= myLINES[num2].x2 + PARAMETER[P_O_TOLERANCE].vdouble && py >= myLINES[num2].y2 - PARAMETER[P_O_TOLERANCE].vdouble && py <= myLINES[num2].y2 + PARAMETER[P_O_TOLERANCE].vdouble) {
				ret++;
				dir = 1;
				break;
			}
		}
	}
}
	px = myLINES[num].x2;
	py = myLINES[num].y2;
#pragma omp parallel
{
	for (num2 = 1; num2 < line_last; num2++) {
		if (myLINES[num2].used == 1 && num != num2 && strcmp(myLINES[num2].layer, myLINES[num].layer) == 0) {
			if (px >= myLINES[num2].x1 - PARAMETER[P_O_TOLERANCE].vdouble && px <= myLINES[num2].x1 + PARAMETER[P_O_TOLERANCE].vdouble && py >= myLINES[num2].y1 - PARAMETER[P_O_TOLERANCE].vdouble && py <= myLINES[num2].y1 + PARAMETER[P_O_TOLERANCE].vdouble) {
				ret++;
				dir = 2;
				break;
			} else if (px >= myLINES[num2].x2 - PARAMETER[P_O_TOLERANCE].vdouble && px <= myLINES[num2].x2 + PARAMETER[P_O_TOLERANCE].vdouble && py >= myLINES[num2].y2 - PARAMETER[P_O_TOLERANCE].vdouble && py <= myLINES[num2].y2 + PARAMETER[P_O_TOLERANCE].vdouble) {
				ret++;
				dir = 2;
				break;
			}
		}
	}
}
	if (ret == 1) {
		return dir;
	} else if (ret == 0) {
		return 3;
	}
	return 0;
}

void init_objects (void) {
	int num2 = 0;
	int num5b = 0;
	int object_num = 0;
	/* init objects */
	if (myOBJECTS != NULL) {
		free(myOBJECTS);
		myOBJECTS = NULL;
	}
	myOBJECTS = (_OBJECT *)malloc(sizeof(_OBJECT) * (line_last + 1));
#pragma omp parallel
{
	for (object_num = 0; object_num < line_last; object_num++) {
		myOBJECTS[object_num].use = 1;
		myOBJECTS[object_num].closed = 0;
		myOBJECTS[object_num].climb = 0;
		myOBJECTS[object_num].force = 0;
		myOBJECTS[object_num].offset = 0;
		myOBJECTS[object_num].overcut = 0;
		myOBJECTS[object_num].pocket = 0;
		myOBJECTS[object_num].laser = 0;
		myOBJECTS[object_num].order = 5;
		myOBJECTS[object_num].tabs = 1;
		myOBJECTS[object_num].visited = 0;
		myOBJECTS[object_num].depth = 0.0;
		myOBJECTS[object_num].layer[0] = 0;
		myOBJECTS[object_num].min_x = 999999.0;
		myOBJECTS[object_num].min_y = 999999.0;
		myOBJECTS[object_num].max_x = -999999.0;
		myOBJECTS[object_num].max_y = -999999.0;
		for (num2 = 0; num2 < line_last; num2++) {
			myOBJECTS[object_num].line[num2] = 0;
		}
	}
}
	DrawCheckSize();
	DrawSetZero();

	/* remove erros (like double lines) */
	remove_double_lines();

	/* first find objects on open lines */
	object_num = 0;
	for (num2 = 1; num2 < line_last; num2++) {
		if (myLINES[num2].used == 1) {
			int ends = line_open_check(num2);
			if (ends == 1) {
				line_invert(num2);
			}
			if (ends > 0) {
				find_next_line(object_num, num2, num2, 1, 0);
				myOBJECTS[object_num].closed = 0;
				object_num++;
				object_last = object_num + 2;
			}
		}
	}

	/* find objects and check if open or close */
	for (num2 = 1; num2 < line_last; num2++) {
		if (myLINES[num2].used == 1) {
			int ret = find_next_line(object_num, num2, num2, 1, 0);
			if (myLINES[num2].type == TYPE_MTEXT) {
				myOBJECTS[object_num].closed = 0;
				object_num++;
			} else if (ret == 1) {
				myOBJECTS[object_num].closed = 1;
				object_num++;
			} else if (ret == 0) {
				myOBJECTS[object_num].closed = 0;
				object_num++;
			}
			object_last = object_num + 2;
		}
	}
	/* check if object inside or outside */
	for (num5b = 0; num5b < object_last; num5b++) {
		int flag = 0;
/*
		int lnum = myOBJECTS[num5b].line[0];
		if (myLINES[lnum].used == 1) {
			int pipret = 0;
			double testx = myLINES[lnum].x1;
			double testy = myLINES[lnum].y1;
			pipret = point_in_object(-1, num5b, testx, testy);
			if (pipret != 0) {
				flag = 1;
			}
			pipret = 0;
			testx = myLINES[lnum].x2;
			testy = myLINES[lnum].y2;
			pipret = point_in_object(-1, num5b, testx, testy);
			if (pipret != 0) {
				flag = 1;
			}
		}
*/
		int num4b = 0;
		for (num4b = 0; num4b < line_last; num4b++) {
			if (myOBJECTS[num5b].line[num4b] != 0) {
				int lnum = myOBJECTS[num5b].line[num4b];
				if (myLINES[lnum].used == 1) {
					int pipret = 0;
					double testx = myLINES[lnum].x1;
					double testy = myLINES[lnum].y1;
					pipret = point_in_object(-1, num5b, testx, testy);
					if (pipret != 0) {
						flag = 1;
					}
					pipret = 0;
					testx = myLINES[lnum].x2;
					testy = myLINES[lnum].y2;
					pipret = point_in_object(-1, num5b, testx, testy);
					if (pipret != 0) {
						flag = 1;
					}
				}
			}
		}

		if (flag > 0) {
			myOBJECTS[num5b].inside = 1;
		} else if (myOBJECTS[num5b].line[0] != 0) {
			myOBJECTS[num5b].inside = 0;
		}
	}
#pragma omp parallel
{
	for (object_num = 0; object_num < line_last; object_num++) {
		if (myOBJECTS[object_num].line[0] != 0) {
			if (strncmp(myOBJECTS[object_num].layer, "offset-inside", 13) == 0) {
				if (myOBJECTS[object_num].inside == 0) {
					redir_object(object_num);
					myOBJECTS[object_num].inside = 1;
				}
			} else if (strncmp(myOBJECTS[object_num].layer, "offset-outside", 14) == 0) {
				if (myOBJECTS[object_num].inside == 1) {
					redir_object(object_num);
					myOBJECTS[object_num].inside = 0;
				}
			}
		}
	}
}
	// object-boundingbox
	for (num5b = 0; num5b < object_last; num5b++) {
		if (myLINES[myOBJECTS[num5b].line[0]].type == TYPE_CIRCLE) {
			int lnum = myOBJECTS[num5b].line[0];
			if (myOBJECTS[num5b].min_x > myLINES[lnum].cx - myLINES[lnum].opt) {
				myOBJECTS[num5b].min_x = myLINES[lnum].cx - myLINES[lnum].opt;
			}
			if (myOBJECTS[num5b].min_y > myLINES[lnum].cy - myLINES[lnum].opt) {
				myOBJECTS[num5b].min_y = myLINES[lnum].cy - myLINES[lnum].opt;
			}
			if (myOBJECTS[num5b].max_x < myLINES[lnum].cx + myLINES[lnum].opt) {
				myOBJECTS[num5b].max_x = myLINES[lnum].cx + myLINES[lnum].opt;
			}
			if (myOBJECTS[num5b].max_y < myLINES[lnum].cy + myLINES[lnum].opt) {
				myOBJECTS[num5b].max_y = myLINES[lnum].cy + myLINES[lnum].opt;
			}
		} else {
#pragma omp parallel
{
			int num4b = 0;
			for (num4b = 0; num4b < line_last; num4b++) {
				int lnum = myOBJECTS[num5b].line[num4b];
				if (myLINES[lnum].used == 1) {
					if (myOBJECTS[num5b].min_x > myLINES[lnum].x1) {
						myOBJECTS[num5b].min_x = myLINES[lnum].x1;
					}
					if (myOBJECTS[num5b].min_y > myLINES[lnum].y1) {
						myOBJECTS[num5b].min_y = myLINES[lnum].y1;
					}
					if (myOBJECTS[num5b].min_x > myLINES[lnum].x2) {
						myOBJECTS[num5b].min_x = myLINES[lnum].x2;
					}
					if (myOBJECTS[num5b].min_y > myLINES[lnum].y2) {
						myOBJECTS[num5b].min_y = myLINES[lnum].y2;
					}
					if (myOBJECTS[num5b].max_x < myLINES[lnum].x1) {
						myOBJECTS[num5b].max_x = myLINES[lnum].x1;
					}
					if (myOBJECTS[num5b].max_y < myLINES[lnum].y1) {
						myOBJECTS[num5b].max_y = myLINES[lnum].y1;
					}
					if (myOBJECTS[num5b].max_x < myLINES[lnum].x2) {
						myOBJECTS[num5b].max_x = myLINES[lnum].x2;
					}
					if (myOBJECTS[num5b].max_y < myLINES[lnum].y2) {
						myOBJECTS[num5b].max_y = myLINES[lnum].y2;
					}
				}
			}
}
		}
	}
	PARAMETER[P_O_SELECT].vint = -1;
	if (PARAMETER[P_O_BATCHMODE].vint != 1) {
		gtk_list_store_clear(ListStore[P_O_SELECT]);
		for (object_num = 0; object_num < line_last; object_num++) {
			if (myOBJECTS[object_num].line[0] != 0) {
				char tmp_str[128];	
				snprintf(tmp_str, 128, "%i (%s)", object_num, myOBJECTS[object_num].layer);
				gtk_list_store_insert_with_values(ListStore[P_O_SELECT], NULL, object_num, 0, NULL, 1, tmp_str, -1);
			}
		}
	}
	update_post = 1;
}



char *csv_getfield (char *line, int num, char *val) {
	char *p;
	int n = 0;
	int field = 0;
	char prevc = ';';
	strcpy(val, line);
	for (p = line; *p != '\n'; p++) {  
		if (*p == prevc) {
			val[n] = 0;
			if (field == num) {
				return val;
			}
			n = 0;
			strcpy(val, p + 1);
			field++;
		} else {
			n++;
		}
	}
	return 0;
}

void MaterialLoadList (const char* path) {
	char filename[PATH_MAX];
	if (path[0] == 0) {
		snprintf(filename, PATH_MAX, "%s", "material.tbl");
	} else {
		snprintf(filename, PATH_MAX, "%s%s%s", path, DIR_SEP, "material.tbl");
	}
	FILE *stream = fopen(filename, "r");
	MaterialMax = 0;
	if (stream != NULL) {
		char line[1024];
		char val_str[1024];
		while (fgets(line, 1024, stream)) {
			if (line[0] != '#' && strstr(line, ";") >= 0) {
				if (PARAMETER[P_O_BATCHMODE].vint != 1) {
					gtk_list_store_insert_with_values(ListStore[P_MAT_SELECT], NULL, -1, 0, NULL, 1, csv_getfield(line, 0, val_str), -1);
				}
				Material[MaterialMax].vc = atoi(csv_getfield(line, 1, val_str));
				Material[MaterialMax].fz[0] = atof(csv_getfield(line, 2, val_str));
				Material[MaterialMax].fz[1] = atof(csv_getfield(line, 3, val_str));
				Material[MaterialMax].fz[2] = atof(csv_getfield(line, 4, val_str));
				Material[MaterialMax].texture = malloc(strlen(csv_getfield(line, 5, val_str)) + 1);
				strcpy(Material[MaterialMax].texture, csv_getfield(line, 5, val_str));
				MaterialMax++;
			}
		}
		fclose(stream);
	} else {
		fprintf(stderr, "unable to open '%s'\n", filename);
		if (PARAMETER[P_O_BATCHMODE].vint != 1) {
			gtk_list_store_insert_with_values(ListStore[P_MAT_SELECT], NULL, -1, 0, NULL, 1, "AL", -1);
		}
		Material[MaterialMax].vc = 200;
		Material[MaterialMax].fz[0] = 0.04;
		Material[MaterialMax].fz[1] = 0.05;
		Material[MaterialMax].fz[2] = 0.10;
		Material[MaterialMax].texture = malloc(129);
		strcpy(Material[MaterialMax].texture, "textures/metal.bmp");
		MaterialMax++;
	}
	if (PARAMETER[P_MAT_SELECT].vint >= MaterialMax) {
		PARAMETER[P_MAT_SELECT].vint = 0;
	}
}

void DrawCheckSize (void) {
	int num2 = 0;
	/* check size */
	min_x = 99999.0;
	min_y = 99999.0;
	max_x = 0.0;
	max_y = 0.0;
#pragma omp parallel
{
	for (num2 = 0; num2 < line_last; num2++) {
		if (myLINES[num2].used == 1) {
			if (max_x < myLINES[num2].x1) {
				max_x = myLINES[num2].x1;
			}
			if (max_x < myLINES[num2].x2) {
				max_x = myLINES[num2].x2;
			}
			if (max_y < myLINES[num2].y1) {
				max_y = myLINES[num2].y1;
			}
			if (max_y < myLINES[num2].y2) {
				max_y = myLINES[num2].y2;
			}
			if (myLINES[num2].type == TYPE_CIRCLE && max_x < myLINES[num2].cx + myLINES[num2].opt) {
				max_y = myLINES[num2].cx + myLINES[num2].opt;
			}
			if (myLINES[num2].type == TYPE_CIRCLE && max_y < myLINES[num2].cy + myLINES[num2].opt) {
				max_y = myLINES[num2].cy + myLINES[num2].opt;
			}
			if (min_x > myLINES[num2].x1) {
				min_x = myLINES[num2].x1;
			}
			if (min_x > myLINES[num2].x2) {
				min_x = myLINES[num2].x2;
			}
			if (min_y > myLINES[num2].y1) {
				min_y = myLINES[num2].y1;
			}
			if (min_y > myLINES[num2].y2) {
				min_y = myLINES[num2].y2;
			}
			if (myLINES[num2].type == TYPE_CIRCLE && min_x > myLINES[num2].cx - myLINES[num2].opt) {
				min_x = myLINES[num2].cx - myLINES[num2].opt;
			}
			if (myLINES[num2].type == TYPE_CIRCLE && min_y > myLINES[num2].cy - myLINES[num2].opt) {
				min_y = myLINES[num2].cy - myLINES[num2].opt;
			}
		}
	}
}
}

void DrawSetZero (void) {
	int num = 0;
	/* set bottom-left to 0,0 */
#pragma omp parallel
{
	for (num = 0; num < line_last; num++) {
		if (myLINES[num].used == 1 || myLINES[num].istab == 1) {
			myLINES[num].x1 -= min_x;
			myLINES[num].y1 -= min_y;
			myLINES[num].x2 -= min_x;
			myLINES[num].y2 -= min_y;
			myLINES[num].cx -= min_x;
			myLINES[num].cy -= min_y;
		}
	}
}
}

#define DEFFLEQEPSILON 0.001
#define FLOAT_EQE(x, v, e) ((((v) - (e)) < (x)) && ((x) < ((v) + (e))))

int Within (float fl, float flLow, float flHi){
	if ((fl > flLow) && (fl < flHi)) {
		return 1;
	}
	if (FLOAT_EQE(fl, flLow, DEFFLEQEPSILON) || FLOAT_EQE(fl, flHi, DEFFLEQEPSILON)){
		return 1;
	}
	return 0;
}

int PointOnLine (float ptL1_X, float ptL1_Y, float ptL2_X, float ptL2_Y, float ptTest_X, float ptTest_Y) {
	int bTestX = 1;
	const float flX = ptL2_X - ptL1_X;
	if (FLOAT_EQE(flX, 0.0f, DEFFLEQEPSILON)){
		if(! FLOAT_EQE(ptTest_X, ptL1_X, DEFFLEQEPSILON)) {
			return 0;
		}
		bTestX = 0;
	}
	int bTestY = 1;
	const float flY = ptL2_Y - ptL1_Y;
	if (FLOAT_EQE(flY, 0.0f, DEFFLEQEPSILON)){
		if (! FLOAT_EQE(ptTest_Y, ptL1_Y, DEFFLEQEPSILON)) {
			return 0;
		}
		bTestY = 0;
	}
	const float pX = bTestX ? ((ptTest_X - ptL1_X) / flX) : 0.5f;
	const float pY = bTestY ? ((ptTest_Y - ptL1_Y) / flY) : 0.5f;
	return Within(pX, 0.0f, 1.0f) && Within(pY, 0.0f, 1.0f);
}


void remove_double_lines (void) {
	int num = 0;
	int num2 = 0;
//	glLineWidth(10);
//	glColor4f(1.0, 0.0, 0.0, 1.0);
//	glBegin(GL_LINES);
	for (num = 0; num < line_last; num++) {
		if (myLINES[num].used == 1) {
			for (num2 = 0; num2 < line_last; num2++) {
				if (num != num2 && strcmp(myLINES[num].layer, myLINES[num2].layer) == 0) {
					if (myLINES[num2].used == 1 && ((myLINES[num2].type == TYPE_LINE && myLINES[num].type == TYPE_LINE) || (myLINES[num2].type == TYPE_ELLIPSE && myLINES[num].type == TYPE_ELLIPSE))) {
						if (myLINES[num].x1 == myLINES[num2].x1 && myLINES[num].y1 == myLINES[num2].y1 && myLINES[num].x2 == myLINES[num2].x2 && myLINES[num].y2 == myLINES[num2].y2) {
//							printf("remove double line: %i\n", num2);
//							glVertex3f(myLINES[num2].x1, myLINES[num2].y1, 0.0);
//							glVertex3f(myLINES[num2].x2, myLINES[num2].y2, 0.0);
							myLINES[num2].used = 0;
							myLINES[num].marked = 1;
						} else if (myLINES[num].x1 == myLINES[num2].x2 && myLINES[num].y1 == myLINES[num2].y2 && myLINES[num].x2 == myLINES[num2].x1 && myLINES[num].y2 == myLINES[num2].y1) {
//							printf("remove double line: %i\n", num2);
//							glVertex3f(myLINES[num2].x1, myLINES[num2].y1, 0.0);
//							glVertex3f(myLINES[num2].x2, myLINES[num2].y2, 0.0);
							myLINES[num2].used = 0;
							myLINES[num].marked = 1;
						} else if (PointOnLine(myLINES[num].x1, myLINES[num].y1, myLINES[num].x2, myLINES[num].y2, myLINES[num2].x1, myLINES[num2].y1) == 1 && PointOnLine(myLINES[num].x1, myLINES[num].y1, myLINES[num].x2, myLINES[num].y2, myLINES[num2].x2, myLINES[num2].y2) == 1) {
							float len1 = get_len(myLINES[num].x1, myLINES[num].y1, myLINES[num].x2, myLINES[num].y2);
							float len2 = get_len(myLINES[num2].x1, myLINES[num2].y1, myLINES[num2].x2, myLINES[num2].y2);
//							printf("found line on line: %i(%f) <-> %i(%f)\n", num, len1, num2, len2);
							if (len1 > len2) {
//								myLINES[num2].marked = 1;
//								myLINES[num2].used = 0;
							} else {
//								myLINES[num].marked = 1;
//								myLINES[num].used = 0;
							}
						}
					} else if (myLINES[num2].used == 1 && ((myLINES[num2].type == TYPE_ARC && myLINES[num].type == TYPE_ARC) || (myLINES[num2].type == TYPE_CIRCLE && myLINES[num].type == TYPE_CIRCLE))) {
						if (myLINES[num].cx == myLINES[num2].cx && myLINES[num].cy == myLINES[num2].cy) {
							if (myLINES[num].x1 == myLINES[num2].x1 && myLINES[num].y1 == myLINES[num2].y1 && myLINES[num].x2 == myLINES[num2].x2 && myLINES[num].y2 == myLINES[num2].y2) {
//								printf("remove double arc: %i\n", num2);
//								glVertex3f(myLINES[num2].x1, myLINES[num2].y1, 0.0);
//								glVertex3f(myLINES[num2].x2, myLINES[num2].y2, 0.0);
								myLINES[num2].used = 0;
								myLINES[num].marked = 1;
							} else if (myLINES[num].x1 == myLINES[num2].x2 && myLINES[num].y1 == myLINES[num2].y2 && myLINES[num].x2 == myLINES[num2].x1 && myLINES[num].y2 == myLINES[num2].y1) {
//								printf("remove double arc: %i\n", num2);
//								glVertex3f(myLINES[num2].x1, myLINES[num2].y1, 0.0);
//								glVertex3f(myLINES[num2].x2, myLINES[num2].y2, 0.0);
								myLINES[num2].used = 0;
								myLINES[num].marked = 1;
							}
						}
					}
				}
			}
		}
	}
//	glEnd();
//	glLineWidth(1);
}

int intersect3D_SegmentPlane (float S_P0x, float S_P0y, float S_P0z, float S_P1x, float S_P1y, float S_P1z, float Pn_V0x, float Pn_V0y, float Pn_V0z, float Pn_nx, float Pn_ny, float Pn_nz, float *Ix, float *Iy, float *Iz ) {
	float ux = S_P1x - S_P0x;
	float uy = S_P1y - S_P0y;
	float uz = S_P1z - S_P0z;
	float wx = S_P0x - Pn_V0x;
	float wy = S_P0y - Pn_V0y;
	float wz = S_P0z - Pn_V0z;
	float D = dot(Pn_nx, Pn_ny, Pn_nz, ux, uy, uz);
	float N = -dot(Pn_nx, Pn_ny, Pn_nz, wx, wy, wz);
	float sI = N / D;
	if (sI < 0 || sI > 1) {
		return 0;
	}
	*Ix = S_P0x + sI * ux;
	*Iy = S_P0y + sI * uy;
	*Iz = S_P0z + sI * uz;
	return 1;
}


#ifdef USE_G3D

extern int line_n;

void slice_3d (char *file, float z) {
	G3DContext *context;
	G3DObject *object;
	GSList *olist;
	G3DModel *model;
	int i = 0;
//	int j = 0;
	float Pn_V0x = 0.0;
	float Pn_V0y = 0.0;
	float Pn_V0z = z;
	float Pn_nx = 0.0;
	float Pn_ny = 0.0;
	float Pn_nz = 1.0;

	line_last = 0;
	line_n = 1;
	if (myLINES != NULL) {
		free(myLINES);
		myLINES = NULL;
	}

	if((context = g3d_context_new()) != NULL) {
		if((model = g3d_model_load(context, file)) != NULL) {
			olist = model->objects;
			while(olist != NULL) {
				object = (G3DObject *)olist->data;

				glBegin(GL_TRIANGLES);
				for (i = 0; i < (int)object->_num_faces; i++) {
					float p1x = object->vertex_data[object->_indices[i*3+0]*3+0];
					float p1y = object->vertex_data[object->_indices[i*3+0]*3+1];
					float p1z = object->vertex_data[object->_indices[i*3+0]*3+2];
					float p2x = object->vertex_data[object->_indices[i*3+1]*3+0];
					float p2y = object->vertex_data[object->_indices[i*3+1]*3+1];
					float p2z = object->vertex_data[object->_indices[i*3+1]*3+2];
					float p3x = object->vertex_data[object->_indices[i*3+2]*3+0];
					float p3y = object->vertex_data[object->_indices[i*3+2]*3+1];
					float p3z = object->vertex_data[object->_indices[i*3+2]*3+2];
					float Ix = 0.0;
					float Iy = 0.0;
					float Iz = 0.0;

//					glColor4f(1.0, 1.0, 1.0, 1.0);
//					glBegin(GL_LINES);

					int flag = 0;
					float x1 = 0.0;
					float y1 = 0.0;
					float x2 = 0.0;
					float y2 = 0.0;

					if ((p1z <= Pn_V0z && p2z >= Pn_V0z) || (p1z >= Pn_V0z && p2z <= Pn_V0z)) {
						if (intersect3D_SegmentPlane(p1x, p1y, p1z, p2x, p2y, p2z, Pn_V0x, Pn_V0y, Pn_V0z, Pn_nx, Pn_ny, Pn_nz, &Ix, &Iy, &Iz) == 1) {
//							glVertex3f(Ix, Iy, Iz + 10.1);
							if (flag == 0) {
								flag++;
								x1 = Ix;
								y1 = Iy;
							} else {
								flag++;
								x2 = Ix;
								y2 = Iy;
							}
						}
					}
					if ((p2z <= Pn_V0z && p3z >= Pn_V0z) || (p2z >= Pn_V0z && p3z <= Pn_V0z)) {
						if (intersect3D_SegmentPlane(p3x, p3y, p3z, p2x, p2y, p2z, Pn_V0x, Pn_V0y, Pn_V0z, Pn_nx, Pn_ny, Pn_nz, &Ix, &Iy, &Iz) == 1) {
//							glVertex3f(Ix, Iy, Iz + 10.1);
							if (flag == 0) {
								flag++;
								x1 = Ix;
								y1 = Iy;
							} else {
								flag++;
								x2 = Ix;
								y2 = Iy;
							}
						}
					}
					if ((p1z <= Pn_V0z && p3z >= Pn_V0z) || (p1z >= Pn_V0z && p3z <= Pn_V0z)) {
						if (intersect3D_SegmentPlane(p1x, p1y, p1z, p3x, p3y, p3z, Pn_V0x, Pn_V0y, Pn_V0z, Pn_nx, Pn_ny, Pn_nz, &Ix, &Iy, &Iz) == 1) {
//							glVertex3f(Ix, Iy, Iz + 10.1);
							if (flag == 0) {
								flag++;
								x1 = Ix;
								y1 = Iy;
							} else {
								flag++;
								x2 = Ix;
								y2 = Iy;
							}
						}
					}
//					glEnd();

					if (flag == 2) {
						add_line(TYPE_LINE, "3d", x1 * 10.0, y1 * 10.0, x2 * 10.0, y2 * 10.0, 0.0, 0.0, 0.0);
					}

//					glColor4f(0.0, 0.0, 1.0, 0.5);
//					glBegin(GL_TRIANGLES);
//					for (j = 0; j < 3; j++) {
//						glNormal3f(object->_normals[(i*3+j)*3+0], object->_normals[(i*3+j)*3+1], object->_normals[(i*3+j)*3+2]);
//						glVertex3f(object->vertex_data[object->_indices[i*3+j]*3+0], object->vertex_data[object->_indices[i*3+j]*3+1], object->vertex_data[object->_indices[i*3+j]*3+2]);
//					}
//					glEnd();

				}
				olist = olist->next;
			}
		}
		g3d_context_free(context);
	}
}

#endif





