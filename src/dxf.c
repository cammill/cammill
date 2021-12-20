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
#ifdef __APPLE__
#include <malloc/malloc.h>
#endif
#include <locale.h>

#include "os-hacks.h" // for getline()

typedef struct {
	int num;
	char help[512];
} DXF_HELP;

DXF_HELP DH[] = {
	{-4, "APP: conditional operator (used only with ssget)"},
	{-3, "APP: extended data (XDATA) sentinel (fixed)"},
	{-2, "APP: entity name reference (fixed)"},
	{-1, "APP: entity name. The name changes each time a drawing is opened. It is never saved (fixed)"},
	{0, "Text string indicating the entity type (fixed)"},
	{1, "Primary text value for an entity"},
	{2, "Name (attribute tag, block name, and so on)"},
	{3, "Other text or name values"},
	{4, "Other text or name values"},
	{5, "Entity handle; text string of up to 16 hexadecimal digits (fixed)"},
	{6, "Linetype name (fixed)"},
	{7, "Text style name (fixed)"},
	{8, "Layer name (fixed)"},
	{9, "DXF: variable name identifier (used only in HEADER section of the DXF file)"},
	{10, "Primary point; this is the start point of a line or text entity, center of a circle, and so on DXF: X value of the primary point (followed by Y and Z value codes 20 and 30) APP: 3D point (list of three reals)"},
	{11, "Other points DXF: X value of other points (followed by Y value codes 21-28 and Z value codes 31-38) APP: 3D point (list of three reals)"},
	{12, "Other points DXF: X value of other points (followed by Y value codes 21-28 and Z value codes 31-38) APP: 3D point (list of three reals)"},
	{13, "Other points DXF: X value of other points (followed by Y value codes 21-28 and Z value codes 31-38) APP: 3D point (list of three reals)"},
	{14, "Other points DXF: X value of other points (followed by Y value codes 21-28 and Z value codes 31-38) APP: 3D point (list of three reals)"},
	{15, "Other points DXF: X value of other points (followed by Y value codes 21-28 and Z value codes 31-38) APP: 3D point (list of three reals)"},
	{16, "Other points DXF: X value of other points (followed by Y value codes 21-28 and Z value codes 31-38) APP: 3D point (list of three reals)"},
	{17, "Other points DXF: X value of other points (followed by Y value codes 21-28 and Z value codes 31-38) APP: 3D point (list of three reals)"},
	{18, "Other points DXF: X value of other points (followed by Y value codes 21-28 and Z value codes 31-38) APP: 3D point (list of three reals)"},
	{20, "DXF: Y and Z values of the primary point "},
	{21, "DXF: Y and Z values of other points "},
	{22, "DXF: Y and Z values of other points "},
	{23, "DXF: Y and Z values of other points "},
	{24, "DXF: Y and Z values of other points "},
	{25, "DXF: Y and Z values of other points "},
	{26, "DXF: Y and Z values of other points "},
	{27, "DXF: Y and Z values of other points "},
	{28, "DXF: Y and Z values of other points "},
	{30, "DXF: Y and Z values of the primary point "},
	{31, "DXF: Y and Z values of other points "},
	{32, "DXF: Y and Z values of other points "},
	{33, "DXF: Y and Z values of other points "},
	{34, "DXF: Y and Z values of other points "},
	{35, "DXF: Y and Z values of other points "},
	{36, "DXF: Y and Z values of other points "},
	{37, "DXF: Y and Z values of other points "},
	{38, "DXF: entity's elevation if nonzero"},
	{39, "Entity's thickness if nonzero (fixed)"},
	{40, "Floating-point values (text height, scale factors, and so on)"},
	{41, "Floating-point values (text height, scale factors, and so on)"},
	{42, "Floating-point values (text height, scale factors, and so on)"},
	{43, "Floating-point values (text height, scale factors, and so on)"},
	{44, "Floating-point values (text height, scale factors, and so on)"},
	{45, "Floating-point values (text height, scale factors, and so on)"},
	{46, "Floating-point values (text height, scale factors, and so on)"},
	{47, "Floating-point values (text height, scale factors, and so on)"},
	{48, "Linetype scale; floating-point scalar value; default value is defined for all entity types"},
	{49, "Repeated floating-point value. Multiple 49 groups may appear in one entity for variable-length tables (such as the dash lengths in the LTYPE table). A 7x group always appears before the first 49 group to specify the table length"},
	{50, "Angles (output in degrees to DXF files and radians through AutoLISP and ObjectARX applications)"},
	{51, "Angles (output in degrees to DXF files and radians through AutoLISP and ObjectARX applications)"},
	{52, "Angles (output in degrees to DXF files and radians through AutoLISP and ObjectARX applications)"},
	{53, "Angles (output in degrees to DXF files and radians through AutoLISP and ObjectARX applications)"},
	{54, "Angles (output in degrees to DXF files and radians through AutoLISP and ObjectARX applications)"},
	{55, "Angles (output in degrees to DXF files and radians through AutoLISP and ObjectARX applications)"},
	{56, "Angles (output in degrees to DXF files and radians through AutoLISP and ObjectARX applications)"},
	{57, "Angles (output in degrees to DXF files and radians through AutoLISP and ObjectARX applications)"},
	{58, "Angles (output in degrees to DXF files and radians through AutoLISP and ObjectARX applications)"},
	{60, "Entity visibility; integer value; absence or 0 indicates visibility; 1 indicates invisibility"},
	{62, "Color number (fixed)"},
	{66, "'Entities follow' flag (fixed)"},
	{67, "Space-that is, model or paper space (fixed)"},
	{68, "APP: identifies whether viewport is on but fully off screen; is not active or is off"},
	{69, "APP: viewport identification number"},
	{70, "Integer values, such as repeat counts, flag bits, or modes"},
	{71, "Integer values, such as repeat counts, flag bits, or modes"},
	{72, "Integer values, such as repeat counts, flag bits, or modes"},
	{73, "Integer values, such as repeat counts, flag bits, or modes"},
	{74, "Integer values, such as repeat counts, flag bits, or modes"},
	{75, "Integer values, such as repeat counts, flag bits, or modes"},
	{76, "Integer values, such as repeat counts, flag bits, or modes"},
	{77, "Integer values, such as repeat counts, flag bits, or modes"},
	{78, "Integer values, such as repeat counts, flag bits, or modes"},
	{90, "32-bit integer values"},
	{91, "32-bit integer values"},
	{92, "32-bit integer values"},
	{93, "32-bit integer values"},
	{94, "32-bit integer values"},
	{95, "32-bit integer values"},
	{96, "32-bit integer values"},
	{97, "32-bit integer values"},
	{98, "32-bit integer values"},
	{99, "32-bit integer values"},
	{100, "Subclass data marker (with derived class name as a string). Required for all objects and entity classes that are derived from another concrete class. The subclass data marker segregates data defined by different classes in the inheritance chain for the same object. This is in addition to the requirement for DXF names for each distinct concrete class derived from ObjectARX (see <a href='subclass_markers_dxf_ab.htm#XREF_11832_DXF_AB'>'Subclass Markers'</a>)"},
	{102, "Control string, followed by '{&lt;arbitrary name&gt;' or '}'. Similar to the xdata 1002 group code, except that when the string begins with '{', it can be followed by an arbitrary string whose interpretation is up to the application. The only other control string allowed is '}' as a group terminator. AutoCAD does not interpret these strings except during drawing audit operations. They are for application use"},
	{105, "Object handle for DIMVAR symbol table entry"},
	{210, "Extrusion direction (fixed) DXF: X value of extrusion direction APP: 3D extrusion direction vector"},
	{220, "DXF: Y and Z values of the extrusion direction"},
	{230, "DXF: Y and Z values of the extrusion direction"},
	{280, "8-bit integer values"},
	{281, "8-bit integer values"},
	{282, "8-bit integer values"},
	{283, "8-bit integer values"},
	{284, "8-bit integer values"},
	{285, "8-bit integer values"},
	{286, "8-bit integer values"},
	{287, "8-bit integer values"},
	{288, "8-bit integer values"},
	{289, "8-bit integer values"},
	{290, "-299 Boolean flag value"},
	{300, "-309 Arbitrary text strings"},
	{310, "-319 Arbitrary binary chunks with same representation and limits as 1004 group codes: hexadecimal strings of up to 254 characters represent data chunks of up to 127 bytes"},
	{320, "-329 Arbitrary object handles; handle values that are taken 'as is.' They are not translated during INSERT and XREF operations"},
	{330, "-339 Soft-pointer handle; arbitrary soft pointers to other objects within same DXF file or drawing. Translated during INSERT and XREF operations"},
	{340, "-349 Hard-pointer handle; arbitrary hard pointers to other objects within same DXF file or drawing. Translated during INSERT and XREF operations"},
	{350, "-359 Soft-owner handle; arbitrary soft ownership links to other objects within same DXF file or drawing. Translated during INSERT and XREF operations"},
	{360, "-369 Hard-owner handle; arbitrary hard ownership links to other objects within same DXF file or drawing. Translated during INSERT and XREF operations"},
	{370, "-379 Lineweight enum value (AcDb::LineWeight). Stored and moved around as a short. Custom non-entity objects may use the full range, but entity classes only use 371-379 DXF group codes in their representation, because AutoCAD and AutoLISP both always assume a 370 group code is the entity's lineweight. This allows 370 to behave like other 'common' entity fields."},
	{380, "-389 PlotStyleName type enum (AcDb::PlotStyleNameType). Stored and moved around as a short. Custom non-entity objects may use the full range, but entity classes only use 381-389 DXF group codes in their representation, for the same reason as the Lineweight range above."},
	{390, "-399 String representing handle value of the PlotStyleName object, basically a hard pointer, but has a different range to make backward compatibility easier to deal with. Stored and moved around as an Object ID (a handle in DXF files) and a special type in AutoLISP. Custom non-entity objects may use the full range, but entity classes only use 391-399 DXF group codes in their representation, for the same reason as the Lineweight range above."},
	{400, "-409 16-bit Integers "},
	{410, "-419 String "},
	{999, "DXF: The 999 group code indicates that the line following it is a comment string. SAVEAS does not include such groups in a DXF output file, but OPEN honors them and ignores the comments. You can use the 999 group to include comments in a DXF file that you've edited"},
	{1000, "ASCII string (up to 255 bytes long) in extended data"},
	{1001, "Registered application name (ASCII string up to 31 bytes long) for extended data"},
	{1002, "Extended data control string ('{' or '}')"},
	{1003 , "Extended data layer name"},
	{1004, "Chunk of bytes (up to 127 bytes long) in extended data"},
	{1005, "Entity handle in extended data; text string of up to 16 hexadecimal digits "},
	{1010, "A point in extended data  DXF: X value (followed by 1020 and 1030 groups) APP: 3D point"},
	{1020, "DXF: Y and Z values of a point"},
	{1030, "DXF: Y and Z values of a point"},
	{1011, "A 3D world space position in extended data  DXF: X value (followed by 1021 and 1031 groups) APP: 3D point"},
	{1021, "DXF: Y and Z values of a world space position"},
	{1031, "DXF: Y and Z values of a world space position"},
	{1012, "A 3D world space displacement in extended data DXF: X value (followed by 1022 and 1032 groups) APP: 3D vector"},
	{1022, "DXF: Y and Z values of a world space displacement"},
	{1032, "DXF: Y and Z values of a world space displacement"},
	{1013, "A 3D world space direction in extended data.  DXF: X value (followed by 1022 and 1032 groups) APP: 3D vector"},
	{1023, "DXF: Y and Z values of a world space direction"},
	{1033, "DXF: Y and Z values of a world space direction"},
	{1040, "Extended data floating-point value"},
	{1041, "Extended data distance value"},
	{1042, "Extended data scale factor"},
	{1070, "Extended data 16-bit signed integer"},
	{1071, "Extended data 32-bit signed long"},
};

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
int object_selected = -1;


void add_line (int type, char *layer, double x1, double y1, double x2, double y2, double opt, double cx, double cy) {
	if (PARAMETER[P_M_DXFDEBUG].vint > 0) {
		printf("dxf: ADD_LINE (%i %i): %f,%f -> %f,%f (%s / %f)\n", line_n, line_last, x1, y1, x2, y2, layer, opt);
	}
	x1 *= PARAMETER[P_O_SCALE].vdouble;
	y1 *= PARAMETER[P_O_SCALE].vdouble;
	x2 *= PARAMETER[P_O_SCALE].vdouble;
	y2 *= PARAMETER[P_O_SCALE].vdouble;
	if (type == TYPE_ARC || type == TYPE_CIRCLE) {
		opt *= PARAMETER[P_O_SCALE].vdouble;
	}
	cx *= PARAMETER[P_O_SCALE].vdouble;
	cy *= PARAMETER[P_O_SCALE].vdouble;

	if (x1 > 10000.0 || y1 > 10000.0 || x2 > 10000.0 || y2 > 10000.0) {
		fprintf(stderr, "dxf: ###### LINE TOO BIG; %f %f -> %f %f ######\n", x1, y1, x2, y2);
		return;
	}
	if (line_n < MAX_LINES) {
		// add layer name if not exist
		int num;
		for (num = 1; num < line_last; num++) {
			if (strcmp(LayerNames[num], layer) == 0) {
				break;
			} else if (LayerNames[num][0] == 0) {
				strncpy(LayerNames[num], layer, sizeof(LayerNames[num]));
				break;
			}
		}
		// check if line allready exist
		for (num = 1; num < line_last; num++) {
			if (myLINES[num].blockdata != 1 && myLINES[num].cx == cx && myLINES[num].cy == cy && myLINES[num].opt == opt) {
				if (myLINES[num].x1 == x1 && myLINES[num].y1 == y1 && myLINES[num].x2 == x2 && myLINES[num].y2 == y2) {
					printf("## DOUBLE_LINE (%i %i): %f,%f -> %f,%f (%s / %f)\n", line_n, line_last, x1, y1, x2, y2, layer, opt);
					if (PARAMETER[P_M_DELETE_DOUBLE].vint == 1 || strcmp(myLINES[num].layer, layer) == 0) {
						fprintf(stderr, "## DROPPED\n");
						return;
					} else {
						fprintf(stderr, "## OTHER LAYER, WARNING ONLY\n");
					}
				} else if (myLINES[num].x1 == x2 && myLINES[num].y1 == y2 && myLINES[num].x2 == x1 && myLINES[num].y2 == y1) {
					printf("## DOUBLE_LINE (%i %i): %f,%f -> %f,%f (%s / %f)\n", line_n, line_last, x1, y1, x2, y2, layer, opt);
					if (PARAMETER[P_M_DELETE_DOUBLE].vint == 1 || strcmp(myLINES[num].layer, layer) == 0) {
						fprintf(stderr, "## DROPPED\n");
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
		strncpy(myLINES[line_n].block, block_name, sizeof(myLINES[line_n].block));
		myLINES[line_n].blockdata = block;
		myLINES[line_n].len = get_len(x1, y1, x2, y2);
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

void add_buldge (char *layer, double pl_last_x, double pl_last_y, double p_x1, double p_y1, double p_r1) {
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
	if (p_r1 > 0.0) {
		cx = x4;
		cy = y4;
	}
	// calc start/end angle
	double a1 = vector_angle(cx, cy, pl_last_x, pl_last_y);
	double a2 = vector_angle(cx, cy, p_x1, p_y1);
	// split arcs
	p_x1 = cx;
	p_y1 = cy;
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
	double p_rast = 18.0;
	for (an = p_a1 + p_rast; an <= p_a2 - (p_rast / 2.0); an += p_rast) {
		double angle1 = toRad(an);
		double x1 = r * cos(angle1);
		double y1 = r * sin(angle1);
		add_line(TYPE_ARC, layer, last_x, last_y, p_x1 + x1, p_y1 + y1, r, p_x1, p_y1);
		last_x = p_x1 + x1;
		last_y = p_y1 + y1;
	}
	double angle3 = toRad(p_a2);
	double x3 = r * cos(angle3);
	double y3 = r * sin(angle3);
	add_line(TYPE_ARC, layer, last_x, last_y, p_x1 + x3, p_y1 + y3, r, p_x1, p_y1);
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
	if (PARAMETER[P_O_UNIT_LOAD].vint == 0) {
		PARAMETER[P_O_UNIT].vint = 0;
		strcpy(PARAMETER[P_O_UNIT].vstr, "inch");
	} else {
		PARAMETER[P_O_UNIT].vint = 1;
		strcpy(PARAMETER[P_O_UNIT].vstr, "mm");
	}
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
	int spl_flag = 0;
	int pl_flag = 0;
	int pl_closed = 0;
	double pl_first_x = 0.0;
	double pl_first_y = 0.0;
	double pl_last_x = 0.0;
	double pl_last_y = 0.0;
	char spline_points[100000];
	spline_points[0] = 0;
	while ((read = getline(&line, &len, fp)) != -1) {
		trimline(line2, 1024, line);
		int dxfoption = atoi(line2);
		if (PARAMETER[P_M_DXFDEBUG].vint > 0) {
			int n = 0;
			for (n = 0; n < 134; n++) {
				if (DH[n].num == dxfoption) {
					printf("dxf: # %s \t(%s)\n", line2, DH[n].help);
					break;
				}
			}
			if (n == 134) {
				printf("dxf: # %s\n", line2);
			}
		}
		if ((read = getline(&line, &len, fp)) != -1) {
			trimline(line2, 1024, line);
			if (PARAMETER[P_M_DXFDEBUG].vint > 0) {
				printf("dxf: ## %s\n", line2);
			}
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
						for (num = 0; num < last; num++) {
							if (myLINES[num].blockdata == 1 && myLINES[num].block[0] != 0 && strcmp(myLINES[num].block, block_name) == 0) {
								add_line(myLINES[num].type, dxf_options[OPTION_LAYERNAME], myLINES[num].x1 * scale_x + block_x, myLINES[num].y1 * scale_y + block_y, myLINES[num].x2 * scale_x + block_x, myLINES[num].y2 * scale_y + block_y, myLINES[num].opt, myLINES[num].cx * scale_x + block_x, myLINES[num].cy * scale_y + block_y);
							}
						}
					} else if (strcmp(last_0, "LINE") == 0) {
						double p_x1 = atof(dxf_options[OPTION_LINE_X1]);
						double p_y1 = atof(dxf_options[OPTION_LINE_Y1]);
						double p_x2 = atof(dxf_options[OPTION_LINE_X2]);
						double p_y2 = atof(dxf_options[OPTION_LINE_Y2]);
						add_line(TYPE_LINE, dxf_options[OPTION_LAYERNAME], p_x1, p_y1, p_x2, p_y2, 0.0, 0.0, 0.0);
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
								add_line(TYPE_ARC, dxf_options[OPTION_LAYERNAME], pl_last_x, pl_last_y, p_x1, p_y1, radius, 0.0, 0.0);
							} else {
								add_line(TYPE_LINE, dxf_options[OPTION_LAYERNAME], pl_last_x, pl_last_y, p_x1, p_y1, 0.0, 0.0, 0.0);
							}
						}
						pl_last_x = p_x1;
						pl_last_y = p_y1;
						pl_flag = 1;
					} else if (strcmp(last_0, "SEQEND") == 0) {
						if (pl_closed == 1) {
							add_line(TYPE_LINE, dxf_options[OPTION_LAYERNAME], pl_last_x, pl_last_y, pl_first_x, pl_first_y, 0.0, 0.0, 0.0);
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
						if (pl_closed == 1) {
							if (dxf_options[42][0] == 0) {
								add_line(TYPE_LINE, dxf_options[OPTION_LAYERNAME], pl_last_x, pl_last_y, pl_first_x, pl_first_y, 0.0, 0.0, 0.0);
							} else {
								add_buldge(dxf_options[OPTION_LAYERNAME], pl_last_x, pl_last_y, pl_first_x, pl_first_y, p_r1);
							}
						}
						dxf_options[42][0] = 0;
						lwpl_flag = 0;
						pl_closed = 0;
					} else if (strcmp(last_0, "POINT") == 0) {
						double p_x1 = atof(dxf_options[OPTION_POINT_X]);
						double p_y1 = atof(dxf_options[OPTION_POINT_Y]);
						double p_x2 = atof(dxf_options[OPTION_POINT_X]);
						double p_y2 = atof(dxf_options[OPTION_POINT_Y]);
						add_line(TYPE_POINT, dxf_options[OPTION_LAYERNAME], p_x1, p_y1, p_x2, p_y2, 0.0, 0.0, 0.0);
					} else if (strcmp(last_0, "SPLINE") == 0) {
						pl_closed = atoi(dxf_options[70]);
						double p_x1 = atof(dxf_options[OPTION_LINE_X1]);
						double p_y1 = atof(dxf_options[OPTION_LINE_Y1]);
						pl_last_x = p_x1;
						pl_last_y = p_y1;
						if (pl_closed == 1) {
						}
						spl_flag = 0;
						pl_closed = 0;
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
						double p_rast = 18.0;
						for (an = p_a1 + p_rast; an <= p_a2 - (p_rast / 2.0); an += p_rast) {
							double angle1 = toRad(an);
							double x1 = r * cos(angle1);
							double y1 = r * sin(angle1);
							if (strcmp(last_0, "CIRCLE") == 0) {
								add_line(TYPE_CIRCLE, dxf_options[OPTION_LAYERNAME], last_x, last_y, p_x1 + x1, p_y1 + y1, r, p_x1, p_y1);
							} else {
								add_line(TYPE_ARC, dxf_options[OPTION_LAYERNAME], last_x, last_y, p_x1 + x1, p_y1 + y1, r, p_x1, p_y1);
							}
							last_x = p_x1 + x1;
							last_y = p_y1 + y1;
						}
						double angle3 = toRad(p_a2);
						double x3 = r * cos(angle3);
						double y3 = r * sin(angle3);
						if (strcmp(last_0, "CIRCLE") == 0) {
							add_line(TYPE_CIRCLE, dxf_options[OPTION_LAYERNAME], last_x, last_y, p_x1 + x3, p_y1 + y3, r, p_x1, p_y1);
						} else {
							add_line(TYPE_ARC, dxf_options[OPTION_LAYERNAME], last_x, last_y, p_x1 + x3, p_y1 + y3, r, p_x1, p_y1);
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
						double p_rast = 18.0;
						for (an = p_a1 + p_rast; an <= p_a2 - (p_rast / 2.0); an += p_rast) {
							double angle1 = toRad(an);
							double x1 = r * cos(angle1);
							double y1 = r * ratio * sin(angle1);
							add_line(TYPE_ELLIPSE, dxf_options[OPTION_LAYERNAME], last_x, last_y, p_x1 + x1, p_y1 + y1, 0.0, 0.0, 0.0);
							last_x = p_x1 + x1;
							last_y = p_y1 + y1;
						}
						add_line(TYPE_ELLIPSE, dxf_options[OPTION_LAYERNAME], last_x, last_y, first_x, first_y, 0.0, 0.0, 0.0);
					} else if (strcmp(last_0, "MTEXT") == 0) {
						double p_x1 = atof(dxf_options[OPTION_MTEXT_X]);
						double p_y1 = atof(dxf_options[OPTION_MTEXT_Y]);
						double p_s = atof(dxf_options[OPTION_MTEXT_SIZE]);
						output_text_dxf(dxf_options[OPTION_MTEXT_TEXT], dxf_options[OPTION_LAYERNAME], p_x1, p_y1, 0.0, p_s, PARAMETER[P_M_TEXT_SCALE_WIDTH].vdouble, PARAMETER[P_M_TEXT_SCALE_HEIGHT].vdouble, PARAMETER[P_M_TEXT_FIXED_WIDTH].vint, PARAMETER[P_M_TEXT_FONT].vstr, mtext_n);
						mtext_n++;
					} else if (strcmp(last_0, "$MEASUREMENT") == 0) {
						int mesurement = atoi(dxf_options[OPTION_MEASUREMENT]);
						if (PARAMETER[P_O_UNIT_LOAD].vint == 2) {
							if (mesurement == 1) {
								PARAMETER[P_O_UNIT].vint = 1;
								strcpy(PARAMETER[P_O_UNIT].vstr, "mm");
							} else {
								PARAMETER[P_O_UNIT].vint = 0;
								strcpy(PARAMETER[P_O_UNIT].vstr, "inch");
							}
						}
					} else {
						pl_flag = 0;
						lwpl_flag = 0;
						spl_flag = 0;
					}
					clear_dxfoptions();
				}
				strncpy(last_0, line2, sizeof(last_0));
				if (PARAMETER[P_M_DXFDEBUG].vint > 0) {
					printf("dxf: CMD: %s\n", last_0);
				}
			} else if(dxfoption == 9 && strcmp(line2, "$MEASUREMENT") == 0) {
				strncpy(last_0, line2, sizeof(last_0));
				if (PARAMETER[P_M_DXFDEBUG].vint > 0) {
					printf("dxf: CMD: %s\n", last_0);
				}
			}
//			printf("## %i: %s\n", dxfoption, line2);
			if (dxfoption < 256) {
				strncpy(dxf_options[dxfoption], line2, sizeof(dxf_options[dxfoption]));
				if (strcmp(last_0, "SPLINE") == 0) {
					if (dxfoption == 10) {
					} else if (dxfoption == 20) {
						double p_x1 = atof(dxf_options[OPTION_SPLINE_CX]);
						double p_y1 = atof(dxf_options[OPTION_SPLINE_CY]);
						if (spl_flag > 0) {
						} else {
							pl_first_x = p_x1;
							pl_first_y = p_y1;
						}
						pl_last_x = p_x1;
						pl_last_y = p_y1;
						spl_flag++;
					}
				} else if (strcmp(last_0, "LWPOLYLINE") == 0) {
					if (dxfoption == 10) {
					} else if (dxfoption == 20) {
						double p_x1 = atof(dxf_options[OPTION_LINE_X1]);
						double p_y1 = atof(dxf_options[OPTION_LINE_Y1]);
						double p_r1 = atof(dxf_options[42]);
						if (lwpl_flag > 0) {
							if (dxf_options[42][0] == 0) {
								add_line(TYPE_LINE, dxf_options[OPTION_LAYERNAME], pl_last_x, pl_last_y, p_x1, p_y1, 0.0, 0.0, 0.0);
							} else {
								add_buldge(dxf_options[OPTION_LAYERNAME], pl_last_x, pl_last_y, p_x1, p_y1, p_r1);
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
	fclose(fp);
}

