#include <stdlib.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>

#include "os-hacks.h"
#ifndef __MINGW32__
#include <pwd.h>
#endif

#include <dxf.h>
#include <setup.h>
#include <postprocessor.h>
#include <gtk/gtk.h>

extern GtkWidget *hbox;
extern GtkWidget *ViewExpander;
extern GtkWidget *ToolExpander;
extern GtkWidget *MillingExpander;
extern GtkWidget *TabsExpander;
extern GtkWidget *RotaryExpander;
extern GtkWidget *TangencialExpander;
extern GtkWidget *MachineExpander;
extern GtkWidget *MaterialExpander;
extern GtkWidget *ObjectsExpander;
extern GtkWidget *MiscExpander;
extern int PannedStat;
extern int ExpanderStat[20];

PARA PARAMETER[] = {
	{"Zoom",	"View",		"-zo",	T_FLOAT,	0,	1.0,	0.0,	"",	0.1,	0.1,	20.0,		"x", 1, 0, "view-zoom", 0, 0, 0},
	{"Helplines",	"View", 	"-hl",	T_BOOL	,	1,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 0, "show helplines", 0, 0, 0},
	{"ShowGrid",	"View", 	"-sg",	T_BOOL	,	1,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 0, "show grid", 0, 0, 0},
	{"ShowTool",	"View", 	"-st",	T_BOOL	,	1,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 0, "show tool-diameter", 0, 0, 0},
	{"Textures",	"View", 	"-vt",	T_BOOL	,	0,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 0, "show textures", 0, 0, 0},
	{"Rotate-X",	"View", 	"-rx",	T_FLOAT	,	0,	-25.0,	0.0,	"",	-360.0,	1.0,	360.0,		"", 1, 0, "View-Rotate X", 0, 0, 0},
	{"Rotate-Y",	"View", 	"-rx",	T_FLOAT	,	0,	0.0,	0.0,	"",	-360.0,	1.0,	360.0,		"", 1, 0, "View-Rotate Y", 0, 0, 0},
	{"Rotate-Z",	"View", 	"-rx",	T_FLOAT	,	0,	0.0,	0.0,	"",	-360.0,	1.0,	360.0,		"", 1, 0, "View-Rotate Z", 0, 0, 0},
	{"Translate-X",	"View", 	"-rx",	T_INT	,	0,	0.0,	0.0,	"",	-1000.0,1.0,	1000.0,		"", 1, 0, "View-Translate X", 0, 0, 0},
	{"Translate-Y",	"View", 	"-rx",	T_INT	,	0,	0.0,	0.0,	"",	-1000.0,1.0,	1000.0,		"", 1, 0, "View-Translate Y", 0, 0, 0},
	{"Grid-Size",	"View", 	"-gs",	T_FLOAT	,	0,	10.0,	0.0,	"",	0.001,	0.1,	100.0,		"mm", 1, 0, "size of the grid", 0, 0, 0},
	{"Arrow-Scale",	"View", 	"-as",	T_FLOAT	,	0,	1.0,	0.0,	"",	0.001,	0.1,	10.0,		"", 1, 0, "size of arrows", 0, 0, 0},
	{"DXF-File",	"View",		"-d",	T_FILE,		0,	0.0,	0.0,	"",	0.0,	0.0,	0.0,		"", 0, 0, "dxf-filename", 0, 0, 0},
	{"Select",	"Tool",		"-tn",	T_SELECT,	0,	0.0,	0.0,	"",	0.0,	1.0,	100.0,		"#", 0, 0, "selected tool from tooltable", 0, 0, 0},
	{"Number",	"Tool",		"-tn",	T_INT,		1,	0.0,	0.0,	"",	1.0,	1.0,	18.0,		"#", 1, 1, "tool-number", 0, 0, 0},
	{"Diameter",	"Tool",		"-td",	T_DOUBLE,	0,	3.0,	3.0,	"",	0.01,	0.01,	18.0,		"mm", 1, 1, "tool-diameter", 0, 0, 0},
	{"CalcSpeed",	"Tool",		"-cs",	T_INT,		10000,	0.0,	0.0,	"",	1.0,	10.0,	100000.0,	"rpm", 1, 0, "calculated spindle-speed", 1, 0, 0},
	{"Speed",	"Tool",		"-ts",	T_INT,		10000,	0.0,	0.0,	"",	1.0,	10.0,	100000.0,	"rpm", 1, 1, "real spindle-speed", 0, 0, 0},
	{"Flutes",	"Tool",		"-tw",	T_INT,		2,	0.0,	0.0,	"",	1.0,	1.0,	10.0,		"#", 1, 0, "flutes of the tool, to calculate max toolspeed and feedrate", 0, 0, 0},
	{"Table",	"Tool",		"-tt",	T_FILE	,	0,	0.0,	0.0,	"",	0.0,	0.0,	0.0,		"", 0, 0, "the tooltable filename", 0, 0, 0},
	{"MaxFeedRate",	"Milling",	"-fm",	T_INT	,	200,	0.0,	0.0,	"",	1.0,	1.0,	10000.0,	"mm/min", 1, 0, "calculated maximum feedrate", 1, 0, 0},
	{"FeedRate",	"Milling",	"-fr",	T_INT	,	200,	0.0,	0.0,	"",	1.0,	1.0,	10000.0,	"mm/min", 1, 1, "real feedrate", 0, 0, 0},
	{"PlungeRate",	"Milling",	"-pr",	T_INT	,	100,	0.0,	0.0,	"",	1.0,	1.0,	10000.0,	"mm/min", 1, 1, "plunge feedrate", 0, 0, 0},
	{"Depth",	"Milling",	"-md",	T_DOUBLE,	0,	-4.0,	-4.0,	"",	-40.0,	0.01,	-0.1,		"mm", 1, 1, "end depth", 0, 0, 0},
	{"Z-Step",	"Milling",	"-msp",	T_DOUBLE,	0,	-4.0,	-4.0,	"",	-40.0,	0.01,	-0.1,		"mm", 1, 1, "depth per step", 0, 0, 0},
	{"Save-Move",	"Milling",	"-msm",	T_DOUBLE,	0,	4.0,	4.0,	"",	1.0,	1.0,	80.0,		"mm", 1, 1, "save move z-position", 0, 0, 0},
	{"Overcut",	"Milling",	"-oc",	T_BOOL	,	0,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 1, "overcutting edges", 0, 0, 0},
	{"Lasermode",	"Milling",	"-lm",	T_BOOL	,	0,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 1, "lasermode, depth=0.0, turn on/off per object", 0, 0, 0},
	{"Climb",	"Milling",	"-mr",	T_BOOL	,	0,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 1, "reverse milling direction / climb milling", 0, 0, 0},
	{"NoOffsets",	"Milling",	"-no",	T_BOOL	,	0,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 1, "without tool-offsets", 0, 0, 0},
	{"Pocket",	"Milling",	"-mp",	T_BOOL	,	0,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 1, "mill pockets inside", 0, 0, 0},
	{"Pocket-Stepover","Milling",	"-ps",	T_INT	,	50,	0.0,	0.0,	"",	1.0,	1.0,	100.0,		"%", 1, 1, "pocket-stepover in percent / relative to the Tool-Diameter", 0, 0, 0},
	{"Velocity-Mode","Milling",	"-vm",	T_BOOL,		0,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 1, "Velocity-Mode G61/G64 P...", 0, 0, 0},
	{"Blending Tolerance","Milling","-bt",	T_DOUBLE,	0,	0.0,	0.05,	"",	0.0,	0.01,	10.0,		"mm", 1, 1, "Blending Tolerance G64 P?", 0, 0, 0},
	{"Mill Text",	"Milling",	"-mt",	T_BOOL	,	0,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 1, "mill mtext from dxf-files", 0, 0, 0},
//	{"Text-Font",	"Milling",	"-mf",	T_SELECT,	0,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 1, "font for mtext", 0, 0, 0},
	{"NC-Debug",	"Milling", 	"-nd",	T_BOOL	,	0,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 0, "output objects whithout offsets / for debuging", 0, 0, 0},
	{"Activate",	"Holding-Tabs",	"-hu",	T_BOOL	,	1,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 1, "using holding-tabs", 0, 0, 0},
	{"Show Grid",	"Holding-Tabs",	"-tg",	T_BOOL	,	1,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 1, "show tab-grid", 0, 0, 0},
	{"Depth",	"Holding-Tabs",	"-hd",	T_DOUBLE,	0,	0.0,	-2.0,	"",	-40.0,	1.0,	0.01,		"mm", 1, 1, "depth of the holding-tabs", 0, 0, 0},
	{"Lenght",	"Holding-Tabs",	"-hl",	T_DOUBLE,	0,	0.0,	5.0,	"",	0.1,	0.1,	40.0,		"mm", 1, 1, "lenght of the holding-tabs", 0, 0, 0},
	{"Triangle-Type","Holding-Tabs","-ht",	T_BOOL	,	1,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 1, "type of the holding-tabs", 0, 0, 0},
	{"On Open","Holding-Tabs","-ht",	T_BOOL	,	0,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 1, "holding-tabs on open contours", 0, 0, 0},
	{"On Inside","Holding-Tabs","-ht",	T_BOOL	,	0,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 1, "holding-tabs on inside-contours", 0, 0, 0},
	{"On Outside","Holding-Tabs","-ht",	T_BOOL	,	1,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 1, "holding-tabs on outside-contours", 0, 0, 0},
	{"X-Grid",	"Holding-Tabs","-ht",	T_DOUBLE,	0,	0.0,	55.0,	"",	0.0,	1.0,	500.0,		"", 1, 1, "X-grid of the holding-tabs", 0, 0, 0},
	{"Y-Grid",	"Holding-Tabs","-ht",	T_DOUBLE,	0,	0.0,	55.0,	"",	0.0,	1.0,	500.0,		"", 1, 1, "Y-grid of the holding-tabs", 0, 0, 0},
	{"Fastmove-Speed","Machine",	 "-fs",	T_INT,		1000,	0.0,	0.0,	"",	0.0,	1.0,	10000.0,	"mm/min", 1, 0, "fast-move speed of the machine, to calculate the milling-time", 0, 0, 0},
	{"Post",	"Machine",	"-mp",	T_SELECT,	0,	0.0,	0.0,	"",	1.0,	1.0,	100.0,		"#", 0, 1, "post-processor selection", 0, 0, 0},
	{"Output-File",	"Milling",	"-o",	T_FILE,		0,	0.0,	0.0,	"",	0.0,	0.0,	0.0,		"", 0, 0, "gcode-output filename", 0, 0, 0},
	{"Select",	"Material",	"-ms",	T_SELECT,	1,	0.0,	0.0,	"",	1.0,	1.0,	100.0,		"#", 0, 1, "material selection to calculate feedrate and spindlespeed", 0, 0, 0},
	{"Cutting Speed","Material",	"-cs",	T_INT	,	200,	0.0,	0.0,	"",	1.0,	1.0,	10000.0,	"m/min", 1, 1, "Cutting Speed", 1, 0, 0},
	{"Feedrate/Flute/<4mm","Material","-f4",T_DOUBLE,	0,	0.0,	00.4,	"",	0.01,	0.01,	10.0,		"", 1, 1, "Feedrate/Flute/<4mm", 1, 0, 0},
	{"Feedrate/Flute/<8mm","Material","-f4",T_DOUBLE,	0,	0.0,	00.4,	"",	0.01,	0.01,	10.0,		"", 1, 1, "Feedrate/Flute/<8mm", 1, 0, 0},
	{"Feedrate/Flute/<12mm","Material","-f4",T_DOUBLE,	0,	0.0,	00.4,	"",	0.01,	0.01,	10.0,		"", 1, 1, "Feedrate/Flute/<12mm", 1, 0, 0},
	{"Texture",	"Material",	"-pc",	T_STRING,	0,	0.0,	0.0,	"",	0.0,	0.0,	0.0,		"", 1, 1, "Bitmap file to load for texture rendering", 1, 0, 0},
	{"Post-Command","Milling",	"-pc",	T_STRING,	0,	0.0,	0.0,	"",	0.0,	0.0,	0.0,		"", 0, 0, "postcommand to trigger an script after saving the gcode (you can use this to copy the gcode to your cnc-machine)", 0, 0, 0},
	{"Select",	"Objects",	"-so",	T_SELECT,	0,	0.0,	0.0,	"",	1.0,	1.0,	10000.0,	"#", 0, 1, "object selection", 0, 0, 0},
	{"Use",		"Objects",	"-ou",	T_BOOL	,	1,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 1, "use this object", 0, 0, 0},
	{"Overwrite",	"Objects",	"-of",	T_BOOL	,	1,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 1, "override global parameters for this object", 0, 0, 0},
	{"Climb",	"Objects",	"-oc",	T_BOOL	,	1,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 1, "Climb Milling", 0, 0, 0},
	{"Offset",	"Objects",	"-oo",	T_SELECT,	1,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 1, "Offset", 0, 0, 0},
	{"Overcut",	"Objects",	"-ov",	T_BOOL	,	1,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 1, "admit an internal cutting error due to the tool diameter", 0, 0, 0},
	{"Pocket",	"Objects",	"-op",	T_BOOL	,	1,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 1, "Pocket cutting", 0, 0, 0},
	{"Laser",	"Objects",	"-ol",	T_BOOL	,	1,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 1, "Laser cutting", 0, 0, 0},
	{"Depth",	"Objects",	"-od",	T_DOUBLE,	0,	-4.0,	-4.0,	"",	-40.0,	0.01,	-0.1,		"mm", 1, 1, "end depth", 0, 0, 0},
	{"Holding-Tab's","Objects",	"-ot",	T_BOOL	,	1,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 1, "use holding tabs", 0, 0, 0},
	{"Order",	"Objects",	"-mo",	T_SELECT,	0,	-1.0,	-1.0,	"",	-1.0,	1.0,	10.0,		"", 1, 1, "milling order (Auto= first internal then external, -1 -> -5 = mill first, 1 -> 5 = mill last)", 0, 0, 0},
	{"Activate",	"Rotary",	"-rm",	T_BOOL	,	0,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 1, "rotarymode to project 2D on rotating axis", 0, 0, 0},
	{"Axis",	"Rotary",	"-ra",	T_SELECT,	0,	0.0,	0.0,	"",	0.0,	1.0,	2.0,		"A/B/C", 0, 1, "axis that used as rotating axis", 0, 0, 0},
	{"Diameter",	"Rotary",	"-md",	T_DOUBLE,	0,	0.0,	100.0,	"",	0.01,	0.01,	1000.0,		"mm", 1, 1, "diameter of the material for the rotary-mode", 0, 0, 0},
	{"Activate",	"Tangencial",	"-tm",	T_BOOL	,	0,	0.0,	0.0,	"",	0.0,	1.0,	1.0,		"", 1, 1, "tangencial-knife cutting, depth=0.0", 0, 0, 0},
	{"Axis",	"Tangencial",	"-ta",	T_SELECT,	1,	0.0,	0.0,	"",	0.0,	1.0,	2.0,		"A/B/C", 0, 1, "axis that used as tangencial axis", 0, 0, 0},
	{"MaxAngle",	"Tangencial", 	"-tm",	T_DOUBLE,	0,	0.0,	10.0,	"",	0.0,	1.0,	360.0,		"°", 1, 1, "maximum angle before push up the knife", 0, 0, 0},
	{"Tolerance",	"Misc",		"-mt",	T_DOUBLE,	0,	0.01,	0.001,	"",	0.0001,	0.01,	10.0,		"mm", 1, 1, "Tollerance between points to close objects", 0, 0, 0},
	{"Parameter",	"Misc",		"-st",	T_SELECT,	0,	0.0,	0.0,	"",	0,	1.0,	1.0,		"", 1, 1, "Tabs or Expander for Parameter", 0, 0, 0},
	{"Setup-Autosave","Misc",	"-as",	T_BOOL,		1,	0.0,	0.0,	"",	0,	1.0,	1.0,		"", 1, 1, "Save setup at exit", 0, 0, 0},
	{"Batchmode",	"Misc",		"-bm",	T_BOOL,		0,	0.0,	0.0,	"",	0,	1.0,	1.0,		"", 0, 1, "Batchmode", 1, 0, 0},
#ifdef USE_VNC
	{"VNC-Server",	"Misc",		"-vs",	T_STRING,	0,	0.0,	0.0,	"",	0.0,	0.0,	0.0,		"", 0, 0, "VNC-Server address", 0, 0, 0},
	{"VNC-Port",	"Misc",		"-vp",	T_INT,		5900,	0.0,	0.0,	"",	0.0,	1.0,	35000.0,	"", 0, 0, "VNC-Server port", 0, 0, 0},
#endif
};


void SetupShow (void) {
	int n = 0;
	fprintf(stdout, "\n");
	for (n = 0; n < P_LAST; n++) {
		char name_str[1024];
		snprintf(name_str, sizeof(name_str), "%s-%s", PARAMETER[n].group, PARAMETER[n].name);
		if (PARAMETER[n].type == T_FLOAT) {
			fprintf(stdout, "%22s: %f\n", name_str, PARAMETER[n].vfloat);
		} else if (PARAMETER[n].type == T_DOUBLE) {
			fprintf(stdout, "%22s: %f\n", name_str, PARAMETER[n].vdouble);
		} else if (PARAMETER[n].type == T_INT) {
			fprintf(stdout, "%22s: %i\n", name_str, PARAMETER[n].vint);
		} else if (PARAMETER[n].type == T_SELECT) {
			fprintf(stdout, "%22s: %i\n", name_str, PARAMETER[n].vint);
		} else if (PARAMETER[n].type == T_BOOL) {
			fprintf(stdout, "%22s: %i\n", name_str, PARAMETER[n].vint);
		} else if (PARAMETER[n].type == T_STRING) {
			fprintf(stdout, "%22s: %s\n", name_str, PARAMETER[n].vstr);
		} else if (PARAMETER[n].type == T_FILE) {
			fprintf(stdout, "%22s: %s\n", name_str, PARAMETER[n].vstr);
		}
	}
	fprintf(stdout, "\n");
}

void SetupShowGcode (FILE *out) {
	char tmp_str[1024];
	int n = 0;
	postcam_comment("--------------------------------------------------");
	postcam_comment("GENERATED BY: CAMmill");
	for (n = 0; n < P_LAST; n++) {
		char name_str[1024];
		snprintf(name_str, sizeof(name_str), "%s-%s", PARAMETER[n].group, PARAMETER[n].name);
		if (PARAMETER[n].type == T_FLOAT) {
			snprintf(tmp_str, sizeof(tmp_str), "%s: %f", name_str, PARAMETER[n].vfloat);
		} else if (PARAMETER[n].type == T_DOUBLE) {
			snprintf(tmp_str, sizeof(tmp_str), "%s: %f", name_str, PARAMETER[n].vdouble);
		} else if (PARAMETER[n].type == T_INT) {
			snprintf(tmp_str, sizeof(tmp_str), "%s: %i", name_str, PARAMETER[n].vint);
		} else if (PARAMETER[n].type == T_SELECT) {
			snprintf(tmp_str, sizeof(tmp_str), "%s: %i", name_str, PARAMETER[n].vint);
		} else if (PARAMETER[n].type == T_BOOL) {
			snprintf(tmp_str, sizeof(tmp_str), "%s: %i", name_str, PARAMETER[n].vint);
		} else if (PARAMETER[n].type == T_STRING) {
			snprintf(tmp_str, sizeof(tmp_str), "%s: %s", name_str, PARAMETER[n].vstr);
		} else if (PARAMETER[n].type == T_FILE) {
			snprintf(tmp_str, sizeof(tmp_str), "%s: %s", name_str, PARAMETER[n].vstr);
		} else {
			continue;
		}
		postcam_comment(tmp_str);
	}
	postcam_comment("--------------------------------------------------");
}

void SetupShowHelp (void) {
	int n = 0;
	fprintf(stdout, "\n");
	fprintf(stdout, "cammill [OPTIONS] FILE\n");
	for (n = 0; n < P_LAST; n++) {
		char name_str[1024];
		snprintf(name_str, sizeof(name_str), "%s-%s", PARAMETER[n].group, PARAMETER[n].name);
		if (PARAMETER[n].type == T_FLOAT) {
			fprintf(stdout, "%5s FLOAT    %s\n", PARAMETER[n].arg, name_str);
		} else if (PARAMETER[n].type == T_DOUBLE) {
			fprintf(stdout, "%5s DOUBLE   %s\n", PARAMETER[n].arg, name_str);
		} else if (PARAMETER[n].type == T_INT) {
			fprintf(stdout, "%5s INT      %s\n", PARAMETER[n].arg, name_str);
		} else if (PARAMETER[n].type == T_SELECT) {
			fprintf(stdout, "%5s INT      %s\n", PARAMETER[n].arg, name_str);
		} else if (PARAMETER[n].type == T_BOOL) {
			fprintf(stdout, "%5s 0/1      %s\n", PARAMETER[n].arg, name_str);
		} else if (PARAMETER[n].type == T_STRING) {
			fprintf(stdout, "%5s STRING   %s\n", PARAMETER[n].arg, name_str);
		} else if (PARAMETER[n].type == T_FILE) {
			fprintf(stdout, "%5s FILE     %s\n", PARAMETER[n].arg, name_str);
		}
	}
	fprintf(stdout, "\n");
}

void SetupSave (void) {
	char cfgfile[2048];
	FILE *cfg_fp;
	int n = 0;
	char homedir[PATH_MAX];
        get_home_dir(homedir);
	setlocale(LC_NUMERIC, "C");
	snprintf(cfgfile, PATH_MAX, "%s%s.cammill.cfg", homedir, DIR_SEP);
	cfg_fp = fopen(cfgfile, "w");
	if (cfg_fp == NULL) {
		fprintf(stderr, "Can not write Setup: %s\n", cfgfile);
		return;
	}
	for (n = 0; n < P_LAST; n++) {
		char name_str[1024];
		snprintf(name_str, sizeof(name_str), "%s|%s", PARAMETER[n].group, PARAMETER[n].name);
		if (PARAMETER[n].type == T_FLOAT) {
			fprintf(cfg_fp, "%s=%f\n", name_str, PARAMETER[n].vfloat);
		} else if (PARAMETER[n].type == T_DOUBLE) {
			fprintf(cfg_fp, "%s=%f\n", name_str, PARAMETER[n].vdouble);
		} else if (PARAMETER[n].type == T_INT) {
			fprintf(cfg_fp, "%s=%i\n", name_str, PARAMETER[n].vint);
		} else if (PARAMETER[n].type == T_SELECT) {
			fprintf(cfg_fp, "%s=%i\n", name_str, PARAMETER[n].vint);
		} else if (PARAMETER[n].type == T_BOOL) {
			fprintf(cfg_fp, "%s=%i\n", name_str, PARAMETER[n].vint);
		} else if (PARAMETER[n].type == T_STRING) {
			fprintf(cfg_fp, "%s=%s\n", name_str, PARAMETER[n].vstr);
		} else if (PARAMETER[n].type == T_FILE) {
			fprintf(cfg_fp, "%s=%s\n", name_str, PARAMETER[n].vstr);
		}
	}
	if (PARAMETER[P_O_PARAVIEW].vint == 0) {
		fprintf(cfg_fp, "GUI|PANED|Position=%i\n", gtk_paned_get_position(GTK_PANED(hbox)));
		fprintf(cfg_fp, "GUI|EXPANDER|View=%i\n", gtk_expander_get_expanded(GTK_EXPANDER(ViewExpander)));
		fprintf(cfg_fp, "GUI|EXPANDER|Tool=%i\n", gtk_expander_get_expanded(GTK_EXPANDER(ToolExpander)));
		fprintf(cfg_fp, "GUI|EXPANDER|Milling=%i\n", gtk_expander_get_expanded(GTK_EXPANDER(MillingExpander)));
		fprintf(cfg_fp, "GUI|EXPANDER|Tabs=%i\n", gtk_expander_get_expanded(GTK_EXPANDER(TabsExpander)));
		fprintf(cfg_fp, "GUI|EXPANDER|Rotary=%i\n", gtk_expander_get_expanded(GTK_EXPANDER(RotaryExpander)));
		fprintf(cfg_fp, "GUI|EXPANDER|Tangencial=%i\n", gtk_expander_get_expanded(GTK_EXPANDER(TangencialExpander)));
		fprintf(cfg_fp, "GUI|EXPANDER|Machine=%i\n", gtk_expander_get_expanded(GTK_EXPANDER(MachineExpander)));
		fprintf(cfg_fp, "GUI|EXPANDER|Material=%i\n", gtk_expander_get_expanded(GTK_EXPANDER(MaterialExpander)));
		fprintf(cfg_fp, "GUI|EXPANDER|Objects=%i\n", gtk_expander_get_expanded(GTK_EXPANDER(ObjectsExpander)));
		fprintf(cfg_fp, "GUI|EXPANDER|Misc=%i\n", gtk_expander_get_expanded(GTK_EXPANDER(MiscExpander)));
	}
	fclose(cfg_fp);
}

void SetupSavePreset (char *cfgfile) {
	FILE *cfg_fp;
	int n = 0;
	setlocale(LC_NUMERIC, "C");
	cfg_fp = fopen(cfgfile, "w");
	if (cfg_fp == NULL) {
		fprintf(stderr, "Can not write Setup: %s\n", cfgfile);
		return;
	}
	for (n = 0; n < P_LAST; n++) {
		char name_str[1024];
		snprintf(name_str, sizeof(name_str), "%s|%s", PARAMETER[n].group, PARAMETER[n].name);
		if (PARAMETER[n].inpreset == 0) {
		} else if (PARAMETER[n].type == T_FLOAT) {
			fprintf(cfg_fp, "%s=%f\n", name_str, PARAMETER[n].vfloat);
		} else if (PARAMETER[n].type == T_DOUBLE) {
			fprintf(cfg_fp, "%s=%f\n", name_str, PARAMETER[n].vdouble);
		} else if (PARAMETER[n].type == T_INT) {
			fprintf(cfg_fp, "%s=%i\n", name_str, PARAMETER[n].vint);
		} else if (PARAMETER[n].type == T_SELECT) {
			fprintf(cfg_fp, "%s=%i\n", name_str, PARAMETER[n].vint);
		} else if (PARAMETER[n].type == T_BOOL) {
			fprintf(cfg_fp, "%s=%i\n", name_str, PARAMETER[n].vint);
		} else if (PARAMETER[n].type == T_STRING) {
			fprintf(cfg_fp, "%s=%s\n", name_str, PARAMETER[n].vstr);
		} else if (PARAMETER[n].type == T_FILE) {
			fprintf(cfg_fp, "%s=%s\n", name_str, PARAMETER[n].vstr);
		}
	}
	fclose(cfg_fp);
}

void SetupLoad (void) {
	char cfgfile[2048];
	char line2[2048];
	FILE *cfg_fp;
	int n = 0;
	for (n = 0; n < 20; n++) {
		ExpanderStat[n] = 0;
	}
	char homedir[PATH_MAX];
        get_home_dir(homedir);
	setlocale(LC_NUMERIC, "C");
	snprintf(cfgfile, PATH_MAX, "%s%s.cammill.cfg", homedir, DIR_SEP);
	cfg_fp = fopen(cfgfile, "r");
	if (cfg_fp == NULL) {
//		fprintf(stderr, "Can not read Setup: %s\n", cfgfile);
	} else {
		char *line = NULL;
		size_t len = 0;
		ssize_t read;
		while ((read = getline(&line, &len, cfg_fp)) != -1) {
			trimline(line2, 1024, line);
			if (strncmp(line2, "GUI|PANED|Position", 18) == 0) {
				PannedStat = atoi(strstr(line2, "=") + 1);
			} else if (strncmp(line2, "GUI|EXPANDER|", 13) == 0) {
				if (strncmp(line2 + 13, "View", 4) == 0) {
					ExpanderStat[0] = atoi(strstr(line2, "=") + 1);
				} else if (strncmp(line2 + 13, "Tool", 4) == 0) {
					ExpanderStat[1] = atoi(strstr(line2, "=") + 1);
				} else if (strncmp(line2 + 13, "Milling", 7) == 0) {
					ExpanderStat[2] = atoi(strstr(line2, "=") + 1);
				} else if (strncmp(line2 + 13, "Tabs", 4) == 0) {
					ExpanderStat[3] = atoi(strstr(line2, "=") + 1);
				} else if (strncmp(line2 + 13, "Rotary", 6) == 0) {
					ExpanderStat[4] = atoi(strstr(line2, "=") + 1);
				} else if (strncmp(line2 + 13, "Tangencial", 10) == 0) {
					ExpanderStat[5] = atoi(strstr(line2, "=") + 1);
				} else if (strncmp(line2 + 13, "Machine", 7) == 0) {
					ExpanderStat[6] = atoi(strstr(line2, "=") + 1);
				} else if (strncmp(line2 + 13, "Material", 8) == 0) {
					ExpanderStat[7] = atoi(strstr(line2, "=") + 1);
				} else if (strncmp(line2 + 13, "Objects", 7) == 0) {
					ExpanderStat[8] = atoi(strstr(line2, "=") + 1);
				} else if (strncmp(line2 + 13, "Misc", 4) == 0) {
					ExpanderStat[9] = atoi(strstr(line2, "=") + 1);
				}
			} else {
				for (n = 0; n < P_LAST; n++) {
					char name_str[1024];
					snprintf(name_str, sizeof(name_str), "%s|%s=", PARAMETER[n].group, PARAMETER[n].name);
					if (strncmp(line2, name_str, strlen(name_str)) == 0) {
						if (PARAMETER[n].type == T_FLOAT) {
							PARAMETER[n].vfloat = atof(line2 + strlen(name_str));
						} else if (PARAMETER[n].type == T_DOUBLE) {
							PARAMETER[n].vdouble = atof(line2 + strlen(name_str));
						} else if (PARAMETER[n].type == T_INT) {
							PARAMETER[n].vint = atoi(line2 + strlen(name_str));
						} else if (PARAMETER[n].type == T_SELECT) {
							PARAMETER[n].vint = atoi(line2 + strlen(name_str));
						} else if (PARAMETER[n].type == T_BOOL) {
							PARAMETER[n].vint = atoi(line2 + strlen(name_str));
						} else if (PARAMETER[n].type == T_STRING) {
							strcpy(PARAMETER[n].vstr, line2 + strlen(name_str));
						} else if (PARAMETER[n].type == T_FILE) {
							strcpy(PARAMETER[n].vstr, line2 + strlen(name_str));
						}
					}
				}
			}
		}
		fclose(cfg_fp);
	}
}

void SetupLoadPreset (char *cfgfile) {
	char line2[2048];
	FILE *cfg_fp;
	int n = 0;
	setlocale(LC_NUMERIC, "C");
	cfg_fp = fopen(cfgfile, "r");
	if (cfg_fp == NULL) {
		fprintf(stderr, "Can not read Setup: %s\n", cfgfile);
	} else {
		char *line = NULL;
		size_t len = 0;
		ssize_t read;
		while ((read = getline(&line, &len, cfg_fp)) != -1) {
			trimline(line2, 1024, line);
			for (n = 0; n < P_LAST; n++) {
				char name_str[1024];
				snprintf(name_str, sizeof(name_str), "%s|%s=", PARAMETER[n].group, PARAMETER[n].name);
				if (strncmp(line2, name_str, strlen(name_str)) == 0) {
					if (PARAMETER[n].type == T_FLOAT) {
						PARAMETER[n].vfloat = atof(line2 + strlen(name_str));
					} else if (PARAMETER[n].type == T_DOUBLE) {
						PARAMETER[n].vdouble = atof(line2 + strlen(name_str));
					} else if (PARAMETER[n].type == T_INT) {
						PARAMETER[n].vint = atoi(line2 + strlen(name_str));
					} else if (PARAMETER[n].type == T_SELECT) {
						PARAMETER[n].vint = atoi(line2 + strlen(name_str));
					} else if (PARAMETER[n].type == T_BOOL) {
						PARAMETER[n].vint = atoi(line2 + strlen(name_str));
					} else if (PARAMETER[n].type == T_STRING) {
						strcpy(PARAMETER[n].vstr, line2 + strlen(name_str));
					} else if (PARAMETER[n].type == T_FILE) {
						strcpy(PARAMETER[n].vstr, line2 + strlen(name_str));
					}
				}
			}
		}
		fclose(cfg_fp);
	}
}

int SetupArgCheck (char *arg, char *arg2) {
	int n = 0;
	for (n = 0; n < P_LAST; n++) {
		if (strcmp(arg, PARAMETER[n].arg) == 0) {
			if (PARAMETER[n].type == T_FLOAT) {
				PARAMETER[n].vfloat = atof(arg2);
			} else if (PARAMETER[n].type == T_DOUBLE) {
				PARAMETER[n].vdouble = atof(arg2);
			} else if (PARAMETER[n].type == T_INT) {
				PARAMETER[n].vint = atoi(arg2);
			} else if (PARAMETER[n].type == T_SELECT) {
				PARAMETER[n].vint = atoi(arg2);
			} else if (PARAMETER[n].type == T_BOOL) {
				PARAMETER[n].vint = atoi(arg2);
			} else if (PARAMETER[n].type == T_STRING) {
				strcpy(PARAMETER[n].vstr, arg2);
			} else if (PARAMETER[n].type == T_FILE) {
				strcpy(PARAMETER[n].vstr, arg2);
			}
			return 1;
		}
	}
	return 0;
}

