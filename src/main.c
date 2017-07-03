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
#include <font.h>
#include <setup.h>
#include <dxf.h>
#include <hpgl.h>
#include <postprocessor.h>
#include <calc.h>
#ifdef USE_BMPMODE
#include <bmp.h>
#endif
#include "os-hacks.h"
#include <libintl.h>
#define _(String) gettext(String)
#define gettext_noop(String) String
#define N_(String) gettext_noop(String)


enum {
  COL_OPTION,
  COL_VALUE,
  COL_GROUP,
  COL_NAME,
  COL_OBJECT,
  COL_OVERWRITE,
  NUM_COLS,
};

enum {
  TREE_VIEW,
  TREE_MACHINE,
  TREE_ROTARY,
  TREE_TANGENCIAL,
  TREE_BITMAP,
  TREE_MISC,
  TREE_GLOBALS,
  TREE_OBJECTS,
  NUM_TREE,
};


void texture_init (void);

// path to cammill executable
char program_path[PATH_MAX];

static char *about1 = "CAMmill 2D";
static char *author1 = "Oliver Dippel <a href='mailto:oliver@multixmedia.org'>oliver@multixmedia.org</a>\nOS X port by McUles <a href='mailto:mcules@fpv-club.de'>mcules@fpv-club.de</a>";
static char *author2 = "improvements by <a href='https://github.com/koppi'>@koppi</a> and <a href='https://github.com/onekk'>@onekk</a>";
static char *website = "Website: <a href='https://github.com/cammill'>https://github.com/cammill</a>\nChat: <a href='https://gitter.im/cammill/cammill'>https://gitter.im/cammill/cammill</a>";

static int select_object_flag = 0;
static int select_object_x = 0;
static int select_object_y = 0;
static int main_mode = 0;
static double tooltbl_diameters[100];
static int save_gcode = 0;
static char tool_descr[100][1024];
static int tools_max = 0;
static volatile int loading = 0;

int update_post = 1;
float draw_scale = 1.0;
FILE *fd_out = NULL;
int winw = 1600;
int winh = 1200;
char output_error[1024];
char output_info[1024];
char output_extension[128];
int postcam_plugin = -1;
char postcam_plugins[100][1024];
char *output_buffer = NULL;
int object_last = 0;
double zero_x = 0.0;
double zero_y = 0.0;
float size_x = 0.0;
float size_y = 0.0;
double min_x = 99999.0;
double min_y = 99999.0;
double max_x = 0.0;
double max_y = 0.0;

static int last_mouse_x = 0;
static int last_mouse_y = 0;
static int last_mouse_button = -1;
static int last_mouse_state = 0;
static void ParameterUpdate (void);

#ifdef USE_VNC
static GtkWidget *VncView;
#endif
#ifdef USE_WEBKIT
static GtkWidget *WebKit;
#endif
static GtkWidget *gCodeViewLabelLua;
static GtkWidget *SizeInfoLabel;
static GtkWidget *StatusBar;
static GtkWidget *glCanvas;
static GtkWidget *gCodeView;
static GtkWidget *gCodeViewLua;
static GtkWidget *MenuBar;
static GtkWidget *ToolBar;
static GtkAccelGroup *accel_group;

GtkWidget *notebook;
GtkWidget *gCodeViewLabel;
GtkWidget *OutputErrorLabel;
GtkTreeStore *treestore;
GtkListStore *ListStore[P_LAST];
GtkWidget *hbox;
GtkTreeStore *treestore = NULL;
GtkTreeIter toplevels[NUM_TREE];
GtkWidget *TreeBox = NULL;

int PannedStat;
int ExpanderStat[G_LAST];

int width = 800;
int height = 600;
int need_init = 1;

double mill_distance_xy = 0.0;
double mill_distance_z = 0.0;
double move_distance_xy = 0.0;
double move_distance_z = 0.0;

GtkWidget *window;
GtkWidget *dialog;

#define PATH_OBJECTS '2'
#define GUI_TYPE_CELL_RENDERER_PARAM             (GUI_CELL_RENDERER_PARAM_get_type())
#define GUI_CELL_RENDERER_PARAM(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),  GUI_TYPE_CELL_RENDERER_PARAM, GuiCellRendererParam))
#define GUI_CELL_RENDERER_PARAM_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  GUI_TYPE_CELL_RENDERER_PARAM, GuiCellRendererParamClass))
#define GUI_IS_CELL_RENDERER_PARAM(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GUI_TYPE_CELL_RENDERER_PARAM))
#define GUI_IS_CELL_RENDERER_PARAM_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  GUI_TYPE_CELL_RENDERER_PARAM))
#define GUI_CELL_RENDERER_PARAM_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  GUI_TYPE_CELL_RENDERER_PARAM, GuiCellRendererParamClass))
#define GUI_CELL_RENDERER_PARAM_PATH     "gui-cell-renderer-param-path"
#define GUI_CELL_RENDERER_PARAM_INFO     "gui-cell-renderer-param-info"

struct _GCRParamInfo {
	gulong focus_out_id;
};

struct _GuiCellRendererParam {
	GtkCellRendererText parent;
};

struct _GuiCellRendererParamClass {
	GtkCellRendererTextClass  parent_class;
};

typedef struct _GuiCellRendererParam GuiCellRendererParam;
typedef struct _GuiCellRendererParamClass GuiCellRendererParamClass;
typedef struct _GCRParamInfo GCRParamInfo;

static GType GUI_CELL_RENDERER_PARAM_get_type (void);
static GtkCellRenderer *gui_cell_renderer_param_new (void);
static void gui_cell_renderer_param_init (GuiCellRendererParam *cellspin);
static void gui_cell_renderer_param_class_init (GuiCellRendererParamClass *klass);
static void gui_cell_renderer_param_finalize (GObject *gobject);
static GtkCellEditable *gui_cell_renderer_param_start_editing (GtkCellRenderer *cell, GdkEvent *event, GtkWidget *widget, const gchar *path, GdkRectangle *background_area, GdkRectangle *cell_area, GtkCellRendererState flags);
static gpointer parent_class;

void update_tree_values (void);
void fill_objtree (void);

GType GUI_CELL_RENDERER_PARAM_get_type (void) {
	static GType cell_spin_type = 0;
	if (cell_spin_type) {
		return cell_spin_type;
	}
	static const GTypeInfo cell_spin_info = {sizeof(GuiCellRendererParamClass), NULL, NULL, (GClassInitFunc)gui_cell_renderer_param_class_init, NULL, NULL, sizeof(GuiCellRendererParam), 0, (GInstanceInitFunc) gui_cell_renderer_param_init};
	cell_spin_type = g_type_register_static (GTK_TYPE_CELL_RENDERER_TEXT, "GuiCellRendererParam", &cell_spin_info, 0);
	return cell_spin_type;
}

static void gui_cell_renderer_param_init (GuiCellRendererParam *cellrendererspin) {
	return;
}

static void gui_cell_renderer_param_class_init (GuiCellRendererParamClass *klass) {
	GtkCellRendererClass *cell_class  = GTK_CELL_RENDERER_CLASS(klass);
	GObjectClass         *object_class    = G_OBJECT_CLASS(klass);
	parent_class           = g_type_class_peek_parent (klass);
	object_class->finalize = gui_cell_renderer_param_finalize;
	cell_class->start_editing = gui_cell_renderer_param_start_editing;
}

static void gui_cell_renderer_param_finalize (GObject *object) {
	(* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

GtkCellRenderer *gui_cell_renderer_param_new (void) {
	GtkCellRenderer *cell;
	GuiCellRendererParam *spincell;
	cell = g_object_new(GUI_TYPE_CELL_RENDERER_PARAM, NULL);
	spincell = GUI_CELL_RENDERER_PARAM(cell);
	return cell;
}

char *gui_liststore_get_text (int n, int i) {
	GtkTreeIter iter;
	char value_str[1024];
	char *value_ptr = "---";
	if (ListStore[n] == NULL) {
		return value_ptr;
	}
	if (i < 0) {
		return "-1";
	}
	sprintf(value_str, "%i", i);
	GtkTreePath *path = gtk_tree_path_new_from_string(value_str);
	if (gtk_tree_model_get_iter(GTK_TREE_MODEL(ListStore[n]), &iter, path)) {
		gtk_tree_path_free(path);
		gtk_tree_model_get(GTK_TREE_MODEL(ListStore[n]), &iter, COL_VALUE, &value_ptr, -1);
	}
	return value_ptr;
}

void gui_cell_renderer_param_editing_done (GtkCellEditable *paramwidget, gpointer data) {
	char value_str[1024];
	const gchar *path;
	GCRParamInfo *info = g_object_get_data(G_OBJECT(data), GUI_CELL_RENDERER_PARAM_INFO);
	if (info->focus_out_id > 0) {
		g_signal_handler_disconnect(paramwidget, info->focus_out_id);
		info->focus_out_id = 0;
	}
	path = g_object_get_data(G_OBJECT(paramwidget), GUI_CELL_RENDERER_PARAM_PATH);
	GtkTreeModel *model;
	GtkTreeIter iter;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(TreeBox));
	if (gtk_tree_model_get_iter_from_string(model, &iter, path)) {
		gchar *group = NULL;
		gchar *name = NULL;
		gchar *value = NULL;
		gchar *object = NULL;
		gtk_tree_model_get(model, &iter, COL_GROUP, &group, COL_NAME, &name, COL_VALUE, &value, COL_OBJECT, &object, -1);
		if (object != NULL && group != NULL && name != NULL && value != NULL) {
			if (strcmp(object, "-1") == 0) {
				int n = 0;
				for (n = 0; n < P_LAST; n++) {
					if (strcmp(PARAMETER[n].group, group) == 0 && strcmp(PARAMETER[n].name, name) == 0 && PARAMETER[n].readonly == 0) {
						if (PARAMETER[n].type == T_FLOAT) {
							strcpy(value_str, gtk_entry_get_text(GTK_ENTRY(paramwidget)));
							PARAMETER[n].vfloat = atof(value_str);
						} else if (PARAMETER[n].type == T_DOUBLE) {
							strcpy(value_str, gtk_entry_get_text(GTK_ENTRY(paramwidget)));
							PARAMETER[n].vdouble = atof(value_str);
						} else if (PARAMETER[n].type == T_INT) {
							strcpy(value_str, gtk_entry_get_text(GTK_ENTRY(paramwidget)));
							PARAMETER[n].vint = atoi(value_str);
						} else if (PARAMETER[n].type == T_SELECT) {
							PARAMETER[n].vint = gtk_combo_box_get_active(GTK_COMBO_BOX(paramwidget));
							sprintf(value_str, "%s", gui_liststore_get_text(n, PARAMETER[n].vint));
						} else if (PARAMETER[n].type == T_STRING) {
							strcpy(value_str, gtk_entry_get_text(GTK_ENTRY(paramwidget)));
							strcpy(PARAMETER[n].vstr, value_str);
						} else if (PARAMETER[n].type == T_FILE) {
							strcpy(value_str, gtk_entry_get_text(GTK_ENTRY(paramwidget)));
							strcpy(PARAMETER[n].vstr, value_str);
						}
						update_tree_values();
						update_post = 1;

					}
				}
			} else {
				int object_num = atoi(object);
				int n = 0;
				for (n = 0; n < P_LAST; n++) {
					if (strcmp(myOBJECTS[object_num].PARAMETER[n].group, group) == 0 && strcmp(myOBJECTS[object_num].PARAMETER[n].name, name) == 0 && PARAMETER[n].readonly == 0) {

						myOBJECTS[object_num].PARAMETER[n].overwrite = 1;

						if (myOBJECTS[object_num].PARAMETER[n].type == T_FLOAT) {
							strcpy(value_str, gtk_entry_get_text(GTK_ENTRY(paramwidget)));
							myOBJECTS[object_num].PARAMETER[n].vfloat = atof(value_str);
						} else if (myOBJECTS[object_num].PARAMETER[n].type == T_DOUBLE) {
							strcpy(value_str, gtk_entry_get_text(GTK_ENTRY(paramwidget)));
							myOBJECTS[object_num].PARAMETER[n].vdouble = atof(value_str);
						} else if (myOBJECTS[object_num].PARAMETER[n].type == T_INT) {
							strcpy(value_str, gtk_entry_get_text(GTK_ENTRY(paramwidget)));
							myOBJECTS[object_num].PARAMETER[n].vint = atoi(value_str);
						} else if (myOBJECTS[object_num].PARAMETER[n].type == T_SELECT) {
							myOBJECTS[object_num].PARAMETER[n].vint = gtk_combo_box_get_active(GTK_COMBO_BOX(paramwidget));
							sprintf(value_str, "%s", gui_liststore_get_text(n, myOBJECTS[object_num].PARAMETER[n].vint));
						} else if (myOBJECTS[object_num].PARAMETER[n].type == T_STRING) {
							strcpy(value_str, gtk_entry_get_text(GTK_ENTRY(paramwidget)));
							strcpy(myOBJECTS[object_num].PARAMETER[n].vstr, value_str);
						} else if (myOBJECTS[object_num].PARAMETER[n].type == T_FILE) {
							strcpy(value_str, gtk_entry_get_text(GTK_ENTRY(paramwidget)));
							strcpy(myOBJECTS[object_num].PARAMETER[n].vstr, value_str);
						}
						update_tree_values();
						update_post = 1;
					}
				}
			}
		}
		if (group != NULL) {
			g_free(group);
		}
		if (name != NULL) {
			g_free(name);
		}
		if (value != NULL) {
			g_free(value);
		}
		if (object != NULL) {
			g_free(object);
		}
	}
}

static gboolean gui_cell_renderer_param_focus_out_event (GtkWidget *paramwidget, gpointer data) {
	gui_cell_renderer_param_editing_done (GTK_CELL_EDITABLE (paramwidget), data);
	return FALSE;
}

static gboolean onButtonPress (GtkWidget *paramwidget, GdkEventButton *bevent, gpointer data) {
	if (bevent->button == 1 && (bevent->type == GDK_2BUTTON_PRESS || bevent->type == GDK_3BUTTON_PRESS)) {
		g_print ("double or triple click caught and ignored.\n");
		return TRUE;
	}
	return FALSE;
}

static GtkCellEditable *gui_cell_renderer_param_start_editing (GtkCellRenderer *cell, GdkEvent *event, GtkWidget *widget, const gchar *path, GdkRectangle *background_area, GdkRectangle *cell_area, GtkCellRendererState flags) {
	GtkCellRendererText *celltext;
	GtkAdjustment *adj;
	GtkWidget *paramwidget = NULL;
	GCRParamInfo *info;
	celltext = GTK_CELL_RENDERER_TEXT(cell);
	GtkTreeModel *model;
	GtkTreeIter iter;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(TreeBox));
	if (gtk_tree_model_get_iter_from_string(model, &iter, path)) {
		gchar *group = NULL;
		gchar *name = NULL;
		gchar *value = NULL;
		gchar *object = NULL;
		gtk_tree_model_get(model, &iter, COL_GROUP, &group, COL_NAME, &name, COL_VALUE, &value, COL_OBJECT, &object, -1);
		if (object != NULL && group != NULL && name != NULL && value != NULL) {
			if (strcmp(object, "-1") == 0) {
//				printf("%s/%s = %s\n", group, name, value);
				int n = 0;
				for (n = 0; n < P_LAST; n++) {
					if (strcmp(PARAMETER[n].group, group) == 0 && strcmp(PARAMETER[n].name, name) == 0 && PARAMETER[n].readonly == 0) {
						if (PARAMETER[n].type == T_FLOAT) {
							paramwidget = g_object_new(GTK_TYPE_SPIN_BUTTON, "has_frame", FALSE, "numeric", TRUE, NULL);
							adj = GTK_ADJUSTMENT(gtk_adjustment_new(PARAMETER[n].vfloat, PARAMETER[n].min, PARAMETER[n].max, PARAMETER[n].step, PARAMETER[n].step * 10.0, 0.0));
							gtk_spin_button_configure(GTK_SPIN_BUTTON(paramwidget), adj, 1, 4);
						} else if (PARAMETER[n].type == T_DOUBLE) {
							paramwidget = g_object_new(GTK_TYPE_SPIN_BUTTON, "has_frame", FALSE, "numeric", TRUE, NULL);
							adj = GTK_ADJUSTMENT(gtk_adjustment_new(PARAMETER[n].vdouble, PARAMETER[n].min, PARAMETER[n].max, PARAMETER[n].step, PARAMETER[n].step * 10.0, 0.0));
							gtk_spin_button_configure(GTK_SPIN_BUTTON(paramwidget), adj, 1, 4);
						} else if (PARAMETER[n].type == T_INT) {
							paramwidget = g_object_new(GTK_TYPE_SPIN_BUTTON, "has_frame", FALSE, "numeric", TRUE, NULL);
							adj = GTK_ADJUSTMENT(gtk_adjustment_new(PARAMETER[n].vint, PARAMETER[n].min, PARAMETER[n].max, PARAMETER[n].step, PARAMETER[n].step * 10.0, 0.0));
							gtk_spin_button_configure(GTK_SPIN_BUTTON(paramwidget), adj, 1, 2);
						} else if (PARAMETER[n].type == T_SELECT) {
							paramwidget = g_object_new(GTK_TYPE_COMBO_BOX, "has_frame", FALSE, NULL);
							gtk_combo_box_set_model(GTK_COMBO_BOX(paramwidget), GTK_TREE_MODEL(ListStore[n]));
							GtkCellRenderer *column = gtk_cell_renderer_text_new();
							gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(paramwidget), column, TRUE);
							gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(paramwidget), column, "cell-background", 0, "text", 1, NULL);
							gtk_combo_box_set_active(GTK_COMBO_BOX(paramwidget), PARAMETER[n].vint);
						} else if (PARAMETER[n].type == T_STRING) {
							paramwidget = g_object_new(GTK_TYPE_ENTRY, "has_frame", FALSE, NULL);
							gtk_entry_set_text(GTK_ENTRY(paramwidget), PARAMETER[n].vstr);
						} else if (PARAMETER[n].type == T_FILE) {
							paramwidget = g_object_new(GTK_TYPE_ENTRY, "has_frame", FALSE, NULL);
							gtk_entry_set_text(GTK_ENTRY(paramwidget), PARAMETER[n].vstr);
						} else {
							continue;
						}
						g_object_set_data_full(G_OBJECT(paramwidget), GUI_CELL_RENDERER_PARAM_PATH, g_strdup(path), g_free);
						if (PARAMETER[n].type != T_SELECT) {
							gtk_editable_select_region(GTK_EDITABLE(paramwidget), 0, -1);
						}
						gtk_widget_show(paramwidget);
						g_signal_connect(paramwidget, "editing_done", G_CALLBACK(gui_cell_renderer_param_editing_done), celltext);
						g_signal_connect(paramwidget, "button_press_event", G_CALLBACK(onButtonPress), NULL);
						info = g_new0(GCRParamInfo, 1);
						info->focus_out_id = g_signal_connect(paramwidget, "focus_out_event", G_CALLBACK(gui_cell_renderer_param_focus_out_event), celltext);
						g_object_set_data_full(G_OBJECT(cell), GUI_CELL_RENDERER_PARAM_INFO, info, g_free);
					}
				}
			} else {
				int object_num = atoi(object);
//				printf("%s/%s = %s\n", group, name, value);
				int n = 0;
				for (n = 0; n < P_LAST; n++) {
					if (strcmp(myOBJECTS[object_num].PARAMETER[n].group, group) == 0 && strcmp(myOBJECTS[object_num].PARAMETER[n].name, name) == 0 && PARAMETER[n].readonly == 0) {
						if (myOBJECTS[object_num].PARAMETER[n].type == T_FLOAT) {
							paramwidget = g_object_new(GTK_TYPE_SPIN_BUTTON, "has_frame", FALSE, "numeric", TRUE, NULL);
							adj = GTK_ADJUSTMENT(gtk_adjustment_new(myOBJECTS[object_num].PARAMETER[n].vfloat, myOBJECTS[object_num].PARAMETER[n].min, myOBJECTS[object_num].PARAMETER[n].max, myOBJECTS[object_num].PARAMETER[n].step, myOBJECTS[object_num].PARAMETER[n].step * 10.0, 0.0));
							gtk_spin_button_configure(GTK_SPIN_BUTTON(paramwidget), adj, 1, 2);
						} else if (myOBJECTS[object_num].PARAMETER[n].type == T_DOUBLE) {
							paramwidget = g_object_new(GTK_TYPE_SPIN_BUTTON, "has_frame", FALSE, "numeric", TRUE, NULL);
							adj = GTK_ADJUSTMENT(gtk_adjustment_new(myOBJECTS[object_num].PARAMETER[n].vdouble, myOBJECTS[object_num].PARAMETER[n].min, myOBJECTS[object_num].PARAMETER[n].max, myOBJECTS[object_num].PARAMETER[n].step, myOBJECTS[object_num].PARAMETER[n].step * 10.0, 0.0));
							gtk_spin_button_configure(GTK_SPIN_BUTTON(paramwidget), adj, 1, 2);
						} else if (myOBJECTS[object_num].PARAMETER[n].type == T_INT) {
							paramwidget = g_object_new(GTK_TYPE_SPIN_BUTTON, "has_frame", FALSE, "numeric", TRUE, NULL);
							adj = GTK_ADJUSTMENT(gtk_adjustment_new(myOBJECTS[object_num].PARAMETER[n].vint, myOBJECTS[object_num].PARAMETER[n].min, myOBJECTS[object_num].PARAMETER[n].max, myOBJECTS[object_num].PARAMETER[n].step, myOBJECTS[object_num].PARAMETER[n].step * 10.0, 0.0));
							gtk_spin_button_configure(GTK_SPIN_BUTTON(paramwidget), adj, 1, 2);
						} else if (myOBJECTS[object_num].PARAMETER[n].type == T_SELECT) {
							paramwidget = g_object_new(GTK_TYPE_COMBO_BOX, "has_frame", FALSE, NULL);
							gtk_combo_box_set_model(GTK_COMBO_BOX(paramwidget), GTK_TREE_MODEL(ListStore[n]));
							GtkCellRenderer *column = gtk_cell_renderer_text_new();
							gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(paramwidget), column, TRUE);
							gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(paramwidget), column, "cell-background", 0, "text", 1, NULL);
							gtk_combo_box_set_active(GTK_COMBO_BOX(paramwidget), myOBJECTS[object_num].PARAMETER[n].vint);
						} else if (myOBJECTS[object_num].PARAMETER[n].type == T_STRING) {
							paramwidget = g_object_new(GTK_TYPE_ENTRY, "has_frame", FALSE, NULL);
							gtk_entry_set_text(GTK_ENTRY(paramwidget), myOBJECTS[object_num].PARAMETER[n].vstr);
						} else if (myOBJECTS[object_num].PARAMETER[n].type == T_FILE) {
							paramwidget = g_object_new(GTK_TYPE_ENTRY, "has_frame", FALSE, NULL);
							gtk_entry_set_text(GTK_ENTRY(paramwidget), myOBJECTS[object_num].PARAMETER[n].vstr);
						} else {
							continue;
						}
						g_object_set_data_full(G_OBJECT(paramwidget), GUI_CELL_RENDERER_PARAM_PATH, g_strdup(path), g_free);
						if (myOBJECTS[object_num].PARAMETER[n].type != T_SELECT) {
							gtk_editable_select_region(GTK_EDITABLE(paramwidget), 0, -1);
						}
						gtk_widget_show(paramwidget);
						g_signal_connect(paramwidget, "editing_done", G_CALLBACK(gui_cell_renderer_param_editing_done), celltext);
						g_signal_connect(paramwidget, "button_press_event", G_CALLBACK(onButtonPress), NULL);
						info = g_new0(GCRParamInfo, 1);
						info->focus_out_id = g_signal_connect(paramwidget, "focus_out_event", G_CALLBACK(gui_cell_renderer_param_focus_out_event), celltext);
						g_object_set_data_full(G_OBJECT(cell), GUI_CELL_RENDERER_PARAM_INFO, info, g_free);
					}
				}
			}
		}
		if (group != NULL) {
			g_free(group);
		}
		if (name != NULL) {
			g_free(name);
		}
		if (value != NULL) {
			g_free(value);
		}
		if (object != NULL) {
			g_free(object);
		}
	}
	return GTK_CELL_EDITABLE(paramwidget);
}

void postcam_load_source (char *plugin) {
	char tmp_str[PATH_MAX];
	if (program_path[0] == 0) {
		snprintf(tmp_str, PATH_MAX, "../lib/cammill/posts%s%s.scpost", DIR_SEP, plugin);
	} else {
		snprintf(tmp_str, PATH_MAX, "%s%s../lib/cammill/posts%s%s.scpost", program_path, DIR_SEP, DIR_SEP, plugin);
	}
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
	snprintf(tmp_str, PATH_MAX, "%s%s../lib/cammill/posts%s%s.scpost", path, DIR_SEP, DIR_SEP, plugin);
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
	if (PARAMETER[P_O_BATCHMODE].vint == 1) {
		return;
	}
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
	if (PARAMETER[P_O_BATCHMODE].vint == 1) {
		return;
	}
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
		snprintf(tmp_str, sizeof(tmp_str), "%0.2f%s", lenX, PARAMETER[P_O_UNIT].vstr);
		output_text_gl_center(tmp_str, 0.0, 0.0, 0.0, 0.005 / draw_scale);
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

	/* Zero-Point */
	glLineWidth(5);
	glColor4f(0.0, 0.0, 1.0, 1.0);
	glBegin(GL_LINES);
	glVertex3f(zero_x + PARAMETER[P_M_ZERO_X].vdouble, zero_y + PARAMETER[P_M_ZERO_Y].vdouble, -100.0);
	glVertex3f(zero_x + PARAMETER[P_M_ZERO_X].vdouble, zero_y + PARAMETER[P_M_ZERO_Y].vdouble, 100.0);
	glEnd();
	glLineWidth(1);

	/* Scale-Arrow's */
	float lenY = size_y;
	float lenX = size_x;
	float lenZ = PARAMETER[P_M_DEPTH].vdouble * -1;
	float offXYZ = 10.0 * 0.01 / PARAMETER[P_V_HELP_ARROW].vfloat / draw_scale;
	float arrow_d = 1.0 * 0.01 / PARAMETER[P_V_HELP_ARROW].vfloat / draw_scale;
	float arrow_l = 6.0 * 0.01 / PARAMETER[P_V_HELP_ARROW].vfloat / draw_scale;
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
	glTranslatef(arrow_d * 2.0, lenY / 2.0, 0.0);
	glRotatef(90.0, 0.0, 0.0, 1.0);
	snprintf(tmp_str, sizeof(tmp_str), "%0.2f%s", lenY, PARAMETER[P_O_UNIT].vstr);
	glPushMatrix();
	glTranslatef(0.0, 0.1 / draw_scale, 0.0);
	output_text_gl_center(tmp_str, 0.0, 0.0, 0.0, 0.005 / draw_scale);
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
	glTranslatef(-lenX / 2.0, 0.0, 0.0);
	snprintf(tmp_str, sizeof(tmp_str), "%0.2f%s", lenX, PARAMETER[P_O_UNIT].vstr);
	glPushMatrix();
	glTranslatef(0.0, -0.2 / draw_scale, 0.0);
	output_text_gl_center(tmp_str, 0.0, 0.0, 0.0, 0.005 / draw_scale);
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
	glTranslatef(arrow_d * 2.0, -arrow_d * 2.0, lenZ / 2.0);
	glRotatef(90.0, 0.0, 1.0, 0.0);
	snprintf(tmp_str, sizeof(tmp_str), "%0.2f%s", lenZ, PARAMETER[P_O_UNIT].vstr);
	glPushMatrix();
	glTranslatef(0.0, -0.2 / draw_scale, 0.0);
	output_text_gl_center(tmp_str, 0.0, 0.0, 0.0, 0.005 / PARAMETER[P_V_ZOOM].vfloat / draw_scale);
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

gboolean select_object_foreach_func (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data) {
	gchar *object = NULL;
	gtk_tree_model_get(model, iter, COL_OBJECT, &object, -1);
	if (object != NULL) {
		if (strcmp(object, "-1") != 0) {
			int object_num = atoi(object);
			if (object_num == object_selected) {
				// expand /Objects first
				char tree_path_str[10];
				tree_path_str[0] = PATH_OBJECTS;
				tree_path_str[1] = 0;
				GtkTreePath *path2 = gtk_tree_path_new_from_string(tree_path_str);
				gtk_tree_view_expand_row(GTK_TREE_VIEW(TreeBox), path2, FALSE);
				// than the selected Object
				GtkTreeSelection *selection;
				selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(TreeBox));
				gtk_tree_selection_select_path(selection, path);
//				gtk_tree_view_expand_row(GTK_TREE_VIEW(TreeBox), path, FALSE);
			}
		}
		g_free(object);
	}
    return FALSE;
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
			object_selected = (int)*ptr;
			update_post = 1;
//			printf("%d ", *ptr);
			ptr++;
		}
//		printf("\n");
	}
	if (update_post == 1) {
//		printf("object_selected: %i\n", object_selected);
		gtk_tree_model_foreach(GTK_TREE_MODEL(treestore), select_object_foreach_func, NULL);
	}
}

void update_gui (void) {
	char tmp_str[1024];
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
	snprintf(tmp_str, sizeof(tmp_str), _("Distance: Mill-XY=%0.2f%s/Z=%0.2f%s / Move-XY=%0.2f%s/Z=%0.2f%s / Time>%0.1fmin"), mill_distance_xy, PARAMETER[P_O_UNIT].vstr, mill_distance_z, PARAMETER[P_O_UNIT].vstr, move_distance_xy, PARAMETER[P_O_UNIT].vstr, move_distance_z, PARAMETER[P_O_UNIT].vstr, milltime);

	gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), tmp_str), tmp_str);
	snprintf(tmp_str, sizeof(tmp_str), "Width=%0.1f%s / Height=%0.1f%s", size_x, PARAMETER[P_O_UNIT].vstr, size_y, PARAMETER[P_O_UNIT].vstr);

	if (strstr(output_buffer, "-nan")) {
		gtk_label_set_text(GTK_LABEL(SizeInfoLabel), _("ERROR: found NaN's in gCode"));
		fprintf(stderr, _("ERROR: found NaN's in gCode\n"));
	} else {
		gtk_label_set_text(GTK_LABEL(SizeInfoLabel), tmp_str);
	}

	if (PARAMETER[P_O_BATCHMODE].vint != 1) {
		glEndList();
	}
}


int sortget_order_num = 0;


gboolean sortget_object_foreach_func (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data) {
	gchar *object = NULL;
	gchar *group = NULL;
	gtk_tree_model_get(model, iter, COL_OBJECT, &object, COL_GROUP, &group, -1);
	if (object != NULL && group != NULL) {
		if (strcmp(object, "-1") != 0 && group[0] == 0) {
			int object_num = atoi(object);
			myOBJECTS[object_num].order = sortget_order_num++;
		}
		g_free(object);
		g_free(group);
	}
    return FALSE;
}




void mainloop (void) {
	size_x = (max_x - min_x);
	size_y = (max_y - min_y);
	draw_scale = (4.0 / size_x);
	if (draw_scale > (4.0 / size_y)) {
		draw_scale = (4.0 / size_y);
	}
	// get diameter from tooltable by number
	if (PARAMETER[P_TOOL_SELECT].vint > 0) {
		PARAMETER[P_TOOL_NUM].vint = PARAMETER[P_TOOL_SELECT].vint;
		PARAMETER[P_TOOL_DIAMETER].vdouble = tooltbl_diameters[PARAMETER[P_TOOL_NUM].vint];
	}



	// http://www.precifast.de/schnittgeschwindigkeit-beim-fraesen-berechnen/
	// n  = vc / (d x pi)
	int n = 0;      // Fräserdrehzahl in U/min
	n = (int)(((float)Material[PARAMETER[P_MAT_SELECT].vint].vc * 1000.0) / (PI * (PARAMETER[P_TOOL_DIAMETER].vdouble)));
	if (n > PARAMETER[P_TOOL_SPEED_MAX].vint) {
		n = PARAMETER[P_TOOL_SPEED_MAX].vint;
	}
	PARAMETER[P_TOOL_SPEED_CALC].vint = n;
	float fz = 0.0; // Zahnvorschub in mm/Zahn (Wieviel ein zahn an material pro umdrehung abhebt)
	if ((PARAMETER[P_TOOL_DIAMETER].vdouble) <= 4.0) {
		fz = Material[PARAMETER[P_MAT_SELECT].vint].fz[FZ_FEEDFLUTE4];
	} else if ((PARAMETER[P_TOOL_DIAMETER].vdouble) <= 8.0) {
		fz = Material[PARAMETER[P_MAT_SELECT].vint].fz[FZ_FEEDFLUTE8];
	} else if ((PARAMETER[P_TOOL_DIAMETER].vdouble) <= 12.0) {
		fz = Material[PARAMETER[P_MAT_SELECT].vint].fz[FZ_FEEDFLUTE12];
	}
	// vf = n * z * fz
	PARAMETER[P_M_FEEDRATE_CALC].vint = (int)((float)n * (float)PARAMETER[P_TOOL_W].vint * fz);
	// update rates
	if (PARAMETER[P_M_USE_CALC].vint == 1) {
		PARAMETER[P_TOOL_SPEED].vint = PARAMETER[P_TOOL_SPEED_CALC].vint;
		PARAMETER[P_M_FEEDRATE].vint = PARAMETER[P_M_FEEDRATE_CALC].vint;
	}
	int object_num = 0;
	for (object_num = 0; object_num < object_last; object_num++) {
		// http://www.precifast.de/schnittgeschwindigkeit-beim-fraesen-berechnen/
		// n  = vc / (d x pi)
		int n = 0;      // Fräserdrehzahl in U/min
		n = (int)(((float)Material[myOBJECTS[object_num].PARAMETER[P_MAT_SELECT].vint].vc * 1000.0) / (PI * (myOBJECTS[object_num].PARAMETER[P_TOOL_DIAMETER].vdouble)));
		if (n > myOBJECTS[object_num].PARAMETER[P_TOOL_SPEED_MAX].vint) {
			n = myOBJECTS[object_num].PARAMETER[P_TOOL_SPEED_MAX].vint;
		}
		myOBJECTS[object_num].PARAMETER[P_TOOL_SPEED_CALC].vint = n;
		float fz = 0.0; // Zahnvorschub in mm/Zahn (Wieviel ein zahn an material pro umdrehung abhebt)
		if ((myOBJECTS[object_num].PARAMETER[P_TOOL_DIAMETER].vdouble) <= 4.0) {
			fz = Material[myOBJECTS[object_num].PARAMETER[P_MAT_SELECT].vint].fz[FZ_FEEDFLUTE4];
		} else if ((myOBJECTS[object_num].PARAMETER[P_TOOL_DIAMETER].vdouble) <= 8.0) {
			fz = Material[myOBJECTS[object_num].PARAMETER[P_MAT_SELECT].vint].fz[FZ_FEEDFLUTE8];
		} else if ((myOBJECTS[object_num].PARAMETER[P_TOOL_DIAMETER].vdouble) <= 12.0) {
			fz = Material[myOBJECTS[object_num].PARAMETER[P_MAT_SELECT].vint].fz[FZ_FEEDFLUTE12];
		}
		// vf = n * z * fz
		myOBJECTS[object_num].PARAMETER[P_M_FEEDRATE_CALC].vint = (int)((float)n * (float)myOBJECTS[object_num].PARAMETER[P_TOOL_W].vint * fz);
		// update rates
		if (myOBJECTS[object_num].PARAMETER[P_M_USE_CALC].vint == 1) {
			myOBJECTS[object_num].PARAMETER[P_TOOL_SPEED].vint = myOBJECTS[object_num].PARAMETER[P_TOOL_SPEED_CALC].vint;
			myOBJECTS[object_num].PARAMETER[P_M_FEEDRATE].vint = myOBJECTS[object_num].PARAMETER[P_M_FEEDRATE_CALC].vint;
			myOBJECTS[object_num].PARAMETER[P_M_FEEDRATE].overwrite = 1;
			myOBJECTS[object_num].PARAMETER[P_TOOL_SPEED].overwrite = 1;
		}
		myOBJECTS[object_num].PARAMETER[P_M_FEEDRATE_CALC].overwrite = 1;
		myOBJECTS[object_num].PARAMETER[P_TOOL_SPEED_CALC].overwrite = 1;
	}

	if (update_post == 1) {
		// Zero-Point
		if (PARAMETER[P_M_ZERO].vint == 1) {
			// Original
			zero_x = -min_x;
			zero_y = -min_y;
		} else if (PARAMETER[P_M_ZERO].vint == 0) {
			// Bottom-Left
			zero_x = 0.0;
			zero_y = 0.0;
		} else {
			// Center
			zero_x = (max_x - min_x) / 2.0;
			zero_y = (max_y - min_y) / 2.0;
		}
#ifdef USE_BMPMODE
		if (main_mode == 1) {
			bitmap_pre();
		}
#endif
		if (PARAMETER[P_O_BATCHMODE].vint != 1) {
			glDeleteLists(1, 1);
			glNewList(1, GL_COMPILE);
			draw_grid();
			if (PARAMETER[P_V_HELPLINES].vint == 1) {
				if (PARAMETER[P_M_ROTARYMODE].vint == 0) {
					//draw_helplines();
				}
			}
		}
		mill_begin(program_path);
		if (main_mode == 1) {
#ifdef USE_BMPMODE
			bitmap2cnc();
#endif
		} else {

			sortget_order_num = 0;
			gtk_tree_model_foreach(GTK_TREE_MODEL(treestore), sortget_object_foreach_func, NULL);

			mill_objects();
		}
		mill_end();
		if (PARAMETER[P_O_BATCHMODE].vint != 1) {
			// update GUI
			update_gui();
		}
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
		if (strcmp(PARAMETER[P_MFILE].vstr, "-") != 0) {
			fclose(fd_out);
		}
		if (PARAMETER[P_POST_CMD].vstr[0] != 0 && PARAMETER[P_MFILE].vstr[0] != 0 && strcmp(PARAMETER[P_MFILE].vstr, "-") != 0) {
			char cmd_str[PATH_MAX];
			snprintf(cmd_str, PATH_MAX, "%s %s", PARAMETER[P_POST_CMD].vstr, PARAMETER[P_MFILE].vstr);
			printf("execute command: %s\n", cmd_str);
			int ret;
			if ((ret = system(cmd_str))) {
				if (WIFEXITED(ret)) {
					fprintf(stderr, "exited, status=%d\n", WEXITSTATUS(ret));
				} else if (WIFSIGNALED(ret)) {
					fprintf(stderr, "killed by signal %d\n", WTERMSIG(ret));
				} else {
					fprintf(stderr, "not recognized: %d\n", ret);
				}
			}
		}
		if (PARAMETER[P_O_BATCHMODE].vint != 1) {
			gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "saving g-code...done"), "saving g-code...done");
		}
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
		glScalef(draw_scale, draw_scale, draw_scale);
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

		draw_helplines();
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
			return;
		}
		tooltbl_diameters[0] = 1;
		n = 0;
		if (PARAMETER[P_O_BATCHMODE].vint != 1) {
			gtk_list_store_clear(ListStore[P_TOOL_SELECT]);
			snprintf(tmp_str, sizeof(tmp_str), "FREE");
			gtk_list_store_insert_with_values(ListStore[P_TOOL_SELECT], NULL, -1, 0, NULL, 1, tmp_str, -1);
		}
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
						strncpy(tool_descr[tooln], strstr(line2, ";") + 1, sizeof(tool_descr[tooln]));
					}
					snprintf(tmp_str, sizeof(tmp_str), "#%i D%0.2f%s (%s)", tooln, tooltbl_diameters[tooln], PARAMETER[P_O_UNIT].vstr, tool_descr[tooln]);
					if (PARAMETER[P_O_BATCHMODE].vint != 1) {
						gtk_list_store_insert_with_values(ListStore[P_TOOL_SELECT], NULL, -1, 0, NULL, 1, tmp_str, -1);
					}
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
	strncpy(PARAMETER[P_MFILE].vstr, "-", sizeof(PARAMETER[P_MFILE].vstr));
	for (num = 1; num < argc; num++) {
		if (SetupArgCheck(argv[num], argv[num + 1]) == 1) {
			num++;
		} else if (strcmp(argv[num], "-h") == 0 || strcmp(argv[num], "--help") == 0) {
			SetupShowHelp();
			exit(0);
		} else if (strcmp(argv[num], "-h") == 0 || strcmp(argv[num], "--gcfg") == 0) {
			num++;
			SetupLoadFromGcode(argv[num]);
		} else if (strcmp(argv[num], "-v") == 0 || strcmp(argv[num], "--version") == 0) {
			printf("%s\n", VERSION);
			exit(0);
		} else if (num != argc - 1) {
			fprintf(stderr, "### unknown argument: %s ###\n", argv[num]);
			SetupShowHelp();
			exit(1);
		} else {
			strncpy(PARAMETER[P_V_DXF].vstr, argv[argc - 1], sizeof(PARAMETER[P_V_DXF].vstr));
		}
	}
}

void view_init_gl(void) {
	if (PARAMETER[P_O_BATCHMODE].vint == 1) {
		return;
	}
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
	if (PARAMETER[P_O_BATCHMODE].vint == 1) {
		return;
	}
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
//		PARAMETER[P_W_MAX].vint = gtk_window_is_max(GTK_WINDOW(window));
//		if (PARAMETER[P_W_MAX].vint == 0) {
			gtk_window_get_size(GTK_WINDOW(window), &PARAMETER[P_W_POSW].vint, &PARAMETER[P_W_POSH].vint);
			gtk_window_get_position(GTK_WINDOW(window), &PARAMETER[P_W_POSX].vint, &PARAMETER[P_W_POSY].vint);
//		}
		SetupSave();
	}
	gtk_main_quit();
}

void handler_tool_mm2inch (GtkWidget *widget, gpointer data) {
	int num;
	loading = 1;
	for (num = 0; num < line_last; num++) {
		if (myLINES[num].used == 1) {
			myLINES[num].x1 /= 25.4;
			myLINES[num].y1 /= 25.4;
			myLINES[num].x2 /= 25.4;
			myLINES[num].y2 /= 25.4;
			myLINES[num].cx /= 25.4;
			myLINES[num].cy /= 25.4;
			if (myLINES[num].type == TYPE_ARC || myLINES[num].type == TYPE_CIRCLE) {
				myLINES[num].opt /= 25.4;
			}
		}
	}
	if (PARAMETER[P_O_UNIT].vint == 1) {
		PARAMETER[P_O_UNIT].vint = 0;
	}
	init_objects();
	fill_objtree();
	loading = 0;
}

void handler_tool_inch2mm (GtkWidget *widget, gpointer data) {
	int num;
	loading = 1;
	for (num = 0; num < line_last; num++) {
		if (myLINES[num].used == 1) {
			myLINES[num].x1 *= 25.4;
			myLINES[num].y1 *= 25.4;
			myLINES[num].x2 *= 25.4;
			myLINES[num].y2 *= 25.4;
			myLINES[num].cx *= 25.4;
			myLINES[num].cy *= 25.4;
			if (myLINES[num].type == TYPE_ARC || myLINES[num].type == TYPE_CIRCLE) {
				myLINES[num].opt *= 25.4;
			}
		}
	}
	if (PARAMETER[P_O_UNIT].vint == 0) {
		PARAMETER[P_O_UNIT].vint = 1;
	}
	init_objects();
	fill_objtree();
	loading = 0;
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
	fill_objtree();
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
	fill_objtree();
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
	fill_objtree();
	loading = 0;
}

void handler_reset_view (GtkWidget *widget, gpointer data) {
	PARAMETER[P_V_TRANSX].vint = 0;
	PARAMETER[P_V_TRANSY].vint = 0;
	PARAMETER[P_V_ROTZ].vfloat = 0.0;
	PARAMETER[P_V_ROTY].vfloat = 0.0;
	PARAMETER[P_V_ROTX].vfloat = 0.0;
	PARAMETER[P_V_ZOOM].vfloat = 1.0;
}

#ifdef __linux__
void handler_preview (GtkWidget *widget, gpointer data) {
	char tmp_file[PATH_MAX];
	char cnc_file[PATH_MAX];
	char cmd_str[PATH_MAX + 20];
	sprintf(cnc_file, "/tmp/ngc-preview.%s", output_extension);
	fd_out = fopen(cnc_file, "w");
	if (fd_out == NULL) {
		fprintf(stderr, "Can not open file: %s\n", cnc_file);
	} else {
		fprintf(fd_out, "%s", output_buffer);
		fclose(fd_out);
		strcpy(tmp_file, "/tmp/ngc-preview.xml");
		fd_out = fopen(tmp_file, "w");
		if (fd_out == NULL) {
			fprintf(stderr, "Can not open file: %s\n", tmp_file);
		} else {
			fprintf(fd_out, "<camotics>\n");
			fprintf(fd_out, "  <nc-files>%s</nc-files>\n", cnc_file);
			fprintf(fd_out, "  <resolution v='0.235543'/>\n");
			fprintf(fd_out, "  <resolution-mode v='HIGH'/>\n");
			fprintf(fd_out, "  <automatic-workpiece v='false'/>\n");
			fprintf(fd_out, "  <workpiece-max v='(%f,%f,%f)'/>\n", (float)max_x - min_x + PARAMETER[P_TOOL_DIAMETER].vdouble, (float)max_y - min_y + PARAMETER[P_TOOL_DIAMETER].vdouble, (float)0.0);
			fprintf(fd_out, "  <workpiece-min v='(%f,%f,%f)'/>\n", (float)0.0 - PARAMETER[P_TOOL_DIAMETER].vdouble, (float)0.0 - PARAMETER[P_TOOL_DIAMETER].vdouble, (float)PARAMETER[P_M_DEPTH].vdouble);
			fprintf(fd_out, "  <tool_table>\n");
			fprintf(fd_out, "    <tool length='%f' number='%i' radius='%f' shape='CYLINDRICAL' units='MM'/>\n", (float)-PARAMETER[P_M_DEPTH].vdouble + 1.0, PARAMETER[P_TOOL_NUM].vint, PARAMETER[P_TOOL_DIAMETER].vdouble / 2.0);
			fprintf(fd_out, "  </tool_table>\n");
			fprintf(fd_out, "</camotics>\n");
			fclose(fd_out);
			snprintf(cmd_str, PATH_MAX, "/usr/bin/camotics \"%s\" &", tmp_file);
			if (system(cmd_str) != 0) {
				fprintf(stderr, "Can not open camotics: %s\n", tmp_file);
			}
		}
	}
}
#endif

void handler_reload_dxf (GtkWidget *widget, gpointer data) {
		gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "reloading dxf..."), "reloading dxf...");
		loading = 1;
		main_mode = 0;
		if (strstr(PARAMETER[P_V_DXF].vstr, ".dxf") > 0 || strstr(PARAMETER[P_V_DXF].vstr, ".DXF") > 0) {
			dxf_read(PARAMETER[P_V_DXF].vstr);
		} else if (strstr(PARAMETER[P_V_DXF].vstr, ".plt") > 0 || strstr(PARAMETER[P_V_DXF].vstr, ".PLT") > 0) {
			hpgl_read(PARAMETER[P_V_DXF].vstr);
#ifdef USE_BMPMODE
		} else if (strstr(PARAMETER[P_V_DXF].vstr, ".png") > 0 || strstr(PARAMETER[P_V_DXF].vstr, ".PNG") > 0) {
			main_mode = 1;
			bitmap_load(PARAMETER[P_V_DXF].vstr);
#endif
#ifdef USE_G3D
		} else {
			slice_3d(PARAMETER[P_V_DXF].vstr, 0.0);
#endif
		}
		if (PARAMETER[P_V_DXF].vstr[0] != 0) {
			strncpy(PARAMETER[P_M_LOADPATH].vstr, PARAMETER[P_V_DXF].vstr, PATH_MAX);
			dirname(PARAMETER[P_M_LOADPATH].vstr);
		}
		init_objects();
		fill_objtree();
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

	ffilter = gtk_file_filter_new();
	gtk_file_filter_set_name(ffilter, _("HPGL-Files"));
	gtk_file_filter_add_pattern(ffilter, "*.plt");
	gtk_file_filter_add_pattern(ffilter, "*.PLT");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), ffilter);

	ffilter = gtk_file_filter_new();
	gtk_file_filter_set_name(ffilter, _("gCode-Files"));
	gtk_file_filter_add_pattern(ffilter, "*.ngc");
	gtk_file_filter_add_pattern(ffilter, "*.NGC");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), ffilter);

#ifdef USE_BMPMODE
	ffilter = gtk_file_filter_new();
	gtk_file_filter_set_name(ffilter, _("Bitmap-Files"));
	gtk_file_filter_add_pattern(ffilter, "*.png");
	gtk_file_filter_add_pattern(ffilter, "*.PNG");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), ffilter);
#endif

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
	if (PARAMETER[P_M_LOADPATH].vstr[0] == 0) {
		char loadpath[PATH_MAX];
		if (program_path[0] == 0) {
			snprintf(loadpath, PATH_MAX, "%s", "../share/doc/cammill/examples");
		} else {
			snprintf(loadpath, PATH_MAX, "%s%s%s", program_path, DIR_SEP, "../share/doc/cammill/examples");
		}
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), loadpath);
	} else {
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), PARAMETER[P_M_LOADPATH].vstr);
	}
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		strncpy(PARAMETER[P_V_DXF].vstr, filename, sizeof(PARAMETER[P_V_DXF].vstr));
		gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "reading dxf..."), "reading dxf...");
		loading = 1;
		main_mode = 0;
		if (strstr(filename, ".ngc") > 0 || strstr(filename, ".NGC") > 0) {
			SetupLoadFromGcode(filename);
			if (PARAMETER[P_V_DXF].vstr[0] != 0) {
				dxf_read(PARAMETER[P_V_DXF].vstr);
			}
		} else if (strstr(PARAMETER[P_V_DXF].vstr, ".dxf") > 0 || strstr(PARAMETER[P_V_DXF].vstr, ".DXF") > 0) {
			dxf_read(PARAMETER[P_V_DXF].vstr);
		} else if (strstr(PARAMETER[P_V_DXF].vstr, ".plt") > 0 || strstr(PARAMETER[P_V_DXF].vstr, ".PLT") > 0) {
			hpgl_read(PARAMETER[P_V_DXF].vstr);
#ifdef USE_BMPMODE
		} else if (strstr(PARAMETER[P_V_DXF].vstr, ".png") > 0 || strstr(PARAMETER[P_V_DXF].vstr, ".PNG") > 0) {
			main_mode = 1;
			bitmap_load(PARAMETER[P_V_DXF].vstr);
#endif
#ifdef USE_G3D
		} else {
			slice_3d(PARAMETER[P_V_DXF].vstr, 0.0);
#endif
		}
		if (PARAMETER[P_V_DXF].vstr[0] != 0) {
			strncpy(PARAMETER[P_M_LOADPATH].vstr, PARAMETER[P_V_DXF].vstr, PATH_MAX);
			dirname(PARAMETER[P_M_LOADPATH].vstr);
		}
		init_objects();
		fill_objtree();
		if (strstr(filename, ".ngc") > 0 || strstr(filename, ".NGC") > 0) {
			SetupLoadFromGcodeObjects(PARAMETER[P_V_DXF].vstr);
		}
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
	if (PARAMETER[P_M_PRESETPATH].vstr[0] == 0) {
		char homedir[PATH_MAX];
		get_home_dir(homedir);
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), homedir);
	} else {
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), PARAMETER[P_M_PRESETPATH].vstr);
	}
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "reading preset..."), "reading preset...");
		SetupLoadPreset(filename);
		strncpy(PARAMETER[P_M_PRESETPATH].vstr, filename, PATH_MAX);
		dirname(PARAMETER[P_M_PRESETPATH].vstr);
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
	if (PARAMETER[P_M_PRESETPATH].vstr[0] == 0) {
		char homedir[PATH_MAX];
		get_home_dir(homedir);
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), homedir);
	} else {
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), PARAMETER[P_M_PRESETPATH].vstr);
	}
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "saving Preset..."), "saving Preset...");
		SetupSavePreset(filename);
		strncpy(PARAMETER[P_M_PRESETPATH].vstr, filename, PATH_MAX);
		dirname(PARAMETER[P_M_PRESETPATH].vstr);
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
		char dir[PATH_MAX];
		strncpy(dir, PARAMETER[P_V_DXF].vstr, strlen(dir));
		dirname(dir);
		char file[PATH_MAX];
		strncpy(file, basename(PARAMETER[P_V_DXF].vstr), sizeof(file));
		char *file_nosuffix = suffix_remove(file);
		char *file_nosuffix_new = NULL;
		file_nosuffix_new = realloc(file_nosuffix, strlen(file_nosuffix) + 5);
		if (file_nosuffix_new == NULL) {
				fprintf(stderr, "Not enough memory\n");
				exit(1);
		} else {
			file_nosuffix = file_nosuffix_new;
		}
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
		if (PARAMETER[P_M_SAVEPATH].vstr[0] == 0) {
			char homedir[PATH_MAX];
			get_home_dir(homedir);
			gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), homedir);
		} else {
			gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), PARAMETER[P_M_SAVEPATH].vstr);
		}
	}

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		strncpy(PARAMETER[P_MFILE].vstr, filename, sizeof(PARAMETER[P_MFILE].vstr));
		g_free(filename);
		if (PARAMETER[P_MFILE].vstr[0] != 0 && PARAMETER[P_MFILE].vstr[0] != '-') {
			strncpy(PARAMETER[P_M_SAVEPATH].vstr, PARAMETER[P_MFILE].vstr, PATH_MAX);
			dirname(PARAMETER[P_M_SAVEPATH].vstr);
		}
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
	if (PARAMETER[P_MFILE].vstr[0] != 0 && PARAMETER[P_MFILE].vstr[0] != '-') {
		strncpy(PARAMETER[P_M_SAVEPATH].vstr, PARAMETER[P_MFILE].vstr, PATH_MAX);
		dirname(PARAMETER[P_M_SAVEPATH].vstr);
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
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), "../share/cammill/");
	} else {
		gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), PARAMETER[P_TOOL_TABLE].vstr);
	}
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		strncpy(PARAMETER[P_TOOL_TABLE].vstr, filename, sizeof(PARAMETER[P_TOOL_TABLE].vstr));
		gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "loading tooltable..."), "loading tooltable...");
		ToolLoadTable();
		gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "loading tooltable...done"), "loading tooltable...done");
		g_free(filename);
	}
	gtk_widget_destroy(dialog);
}

void handler_save_setup (GtkWidget *widget, gpointer data) {
	gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "saving setup..."), "saving setup...");
//	PARAMETER[P_W_MAX].vint = gtk_window_is_max(GTK_WINDOW(window));
//	if (PARAMETER[P_W_MAX].vint == 0) {
		gtk_window_get_size(GTK_WINDOW(window), &PARAMETER[P_W_POSW].vint, &PARAMETER[P_W_POSH].vint);
		gtk_window_get_position(GTK_WINDOW(window), &PARAMETER[P_W_POSX].vint, &PARAMETER[P_W_POSY].vint);
//	}
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
	snprintf(tmp_str, sizeof(tmp_str), "%s Version %s/Release %s\n\nCopyright © 2006–2016 %s\n%s\n\n%s",about1, VERSION, VRELEASE, author1, author2, website);
	GtkWidget *label = gtk_label_new(tmp_str);
        gtk_label_set_use_markup (GTK_LABEL(label), TRUE);
        gtk_label_set_markup(GTK_LABEL(label), tmp_str);
	gtk_widget_modify_font(label, pango_font_description_from_string("Tahoma 16"));

	char iconfile[PATH_MAX];
	if (program_path[0] == 0) {
		snprintf(iconfile, PATH_MAX, "%s%s%s", "../share/cammill/icons", DIR_SEP, "logo.png");
	} else {
		snprintf(iconfile, PATH_MAX, "%s%s%s%s%s", program_path, DIR_SEP, "../share/cammill/icons", DIR_SEP, "logo.png");
	}
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
	} else if (e->scroll.direction == GDK_SCROLL_DOWN && PARAMETER[P_V_ZOOM].vfloat > 0.2) {
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
	} else if (button == 5 && state == 0 && PARAMETER[P_V_ZOOM].vfloat > 0.15) {
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
		if ((float)(mouseY - last_mouse_y) / 100 > 0.0) {
			PARAMETER[P_V_ZOOM].vfloat = (float)(mouseY - last_mouse_y) / 100;
		}
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

void ParameterUpdate (void) {
	if (PARAMETER[P_O_BATCHMODE].vint == 1) {
		return;
	}
	if (PARAMETER[P_O_UNIT].vint == 1) {
		strcpy(PARAMETER[P_O_UNIT].vstr, "mm");
	} else {
		strcpy(PARAMETER[P_O_UNIT].vstr, "inch");
	}
	if (PARAMETER[P_O_UNIT_LOAD].vint == 1) {
		strcpy(PARAMETER[P_O_UNIT_LOAD].vstr, "mm");
	} else if (PARAMETER[P_O_UNIT_LOAD].vint == 2) {
		strcpy(PARAMETER[P_O_UNIT_LOAD].vstr, "auto");
	} else {
		strcpy(PARAMETER[P_O_UNIT_LOAD].vstr, "inch");
	}
	update_tree_values();
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

/*
gboolean window_event (GtkWidget *widget, GdkEventWindowState *event, gpointer user_data) {
    if (event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED){
		PARAMETER[P_W_MAX].vint = 1;
	} else {
		PARAMETER[P_W_MAX].vint = 0;
	}
	if (PARAMETER[P_W_MAX].vint == 0) {
		gtk_window_get_size(GTK_WINDOW(window), &PARAMETER[P_W_POSW].vint, &PARAMETER[P_W_POSH].vint);
		gtk_window_get_position(GTK_WINDOW(window), &PARAMETER[P_W_POSX].vint, &PARAMETER[P_W_POSY].vint);
	}
	return TRUE;
}
*/

gboolean DnDmotion (GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *seld, guint ttype, guint time, gpointer *NA) {
	gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "drop..."), "drop...");
	return TRUE;
}

gboolean DnDdrop (GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint time, gpointer *NA) {
	return TRUE;
}

void DnDreceive (GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *sdata, guint ttype, guint time, gpointer *NA) {
	int n = 0;
	char *string = (gchar *)gtk_selection_data_get_data(sdata);
	if (strstr(string, ".dxf\0") > 0 || strstr(string, ".DXF\0") > 0) {
		if (strncmp(string, "file://", 7) == 0) {
			strncpy(PARAMETER[P_V_DXF].vstr, string + 7, PATH_MAX);
			for (n = 0; n < strlen(PARAMETER[P_V_DXF].vstr); n++) {
				if (PARAMETER[P_V_DXF].vstr[n] == '\r' || PARAMETER[P_V_DXF].vstr[n] == '\n') {
					PARAMETER[P_V_DXF].vstr[n] = 0;
					break;
				}
			}
			//fprintf(stderr, "open dxf: ##%s##\n", PARAMETER[P_V_DXF].vstr);
			gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "loading dxf..."), "loading dxf...");
			loading = 1;
			dxf_read(PARAMETER[P_V_DXF].vstr);
			if (PARAMETER[P_V_DXF].vstr[0] != 0) {
				strncpy(PARAMETER[P_M_LOADPATH].vstr, PARAMETER[P_V_DXF].vstr, PATH_MAX);
				dirname(PARAMETER[P_M_LOADPATH].vstr);
			}
			init_objects();
			fill_objtree();
			loading = 0;
			gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), "loading dxf...done"), "loading dxf...done");
		}
	} else {
		fprintf(stderr, "unknown data: %s\n", string);
	}
}

void DnDleave (GtkWidget *widget, GdkDragContext *context, guint time, gpointer *NA) {
	gtk_statusbar_push(GTK_STATUSBAR(StatusBar), gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusBar), ""), "");
}

gboolean update_tree_foreach_func (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data) {
    gchar *tree_path_str = NULL;
//    gchar *tree_path_str2 = NULL;
	gchar *group = NULL;
	gchar *name = NULL;
	gchar *value = NULL;
	gchar *object = NULL;
	gboolean *overwrite = NULL;
	gtk_tree_model_get(model, iter, COL_GROUP, &group, COL_NAME, &name, COL_VALUE, &value, COL_OBJECT, &object, COL_OVERWRITE, &overwrite, -1);
	if (object != NULL && group != NULL && name != NULL && value != NULL) {
		if (strcmp(object, "-1") == 0) {
			tree_path_str = gtk_tree_path_to_string(path);
			int n = 0;
			for (n = 0; n < P_LAST; n++) {
				if (strcmp(PARAMETER[n].group, group) == 0 && strcmp(PARAMETER[n].name, name) == 0) {
					char value_str[1024];
					if (PARAMETER[n].type == T_FLOAT) {
						sprintf(value_str, "%f", PARAMETER[n].vfloat);
					} else if (PARAMETER[n].type == T_DOUBLE) {
						sprintf(value_str, "%f", PARAMETER[n].vdouble);
					} else if (PARAMETER[n].type == T_INT) {
						sprintf(value_str, "%i", PARAMETER[n].vint);
					} else if (PARAMETER[n].type == T_SELECT) {
						sprintf(value_str, "%s", gui_liststore_get_text(n, PARAMETER[n].vint));
					} else if (PARAMETER[n].type == T_BOOL) {
						if (PARAMETER[n].vint == 1) {
							sprintf(value_str, _("On"));
						} else {
							sprintf(value_str, _("Off"));
						}
					} else if (PARAMETER[n].type == T_STRING) {
						sprintf(value_str, "%s", PARAMETER[n].vstr);
					} else if (PARAMETER[n].type == T_FILE) {
						sprintf(value_str, "%s", PARAMETER[n].vstr);
					} else {
						continue;
					}
					if (strcmp(value, value_str) != 0) {
//						g_print("#%s: %s/%s: %s -> %s\n", object, group, name, value, value_str);
						gtk_tree_store_set(treestore, iter, COL_VALUE, value_str, COL_OBJECT, "-1", -1);
					}
				}
			}
			g_free(tree_path_str);
		} else {
			int object_num = atoi(object);
			tree_path_str = gtk_tree_path_to_string(path);

			if (strcmp(group, "") == 0 && strcmp(name, "use") == 0) {
				gtk_tree_store_set(treestore, iter, COL_OVERWRITE, myOBJECTS[object_num].use, -1);
			}

			int n = 0;
			for (n = 0; n < P_LAST; n++) {
				if (strcmp(myOBJECTS[object_num].PARAMETER[n].group, group) == 0 && strcmp(myOBJECTS[object_num].PARAMETER[n].name, name) == 0) {
					char value_str[1024];
					if (myOBJECTS[object_num].PARAMETER[n].type == T_FLOAT) {
						sprintf(value_str, "%f", myOBJECTS[object_num].PARAMETER[n].vfloat);
					} else if (myOBJECTS[object_num].PARAMETER[n].type == T_DOUBLE) {
						sprintf(value_str, "%f", myOBJECTS[object_num].PARAMETER[n].vdouble);
					} else if (myOBJECTS[object_num].PARAMETER[n].type == T_INT) {
						sprintf(value_str, "%i", myOBJECTS[object_num].PARAMETER[n].vint);
					} else if (myOBJECTS[object_num].PARAMETER[n].type == T_SELECT) {
						sprintf(value_str, "%s", gui_liststore_get_text(n, myOBJECTS[object_num].PARAMETER[n].vint));
					} else if (myOBJECTS[object_num].PARAMETER[n].type == T_BOOL) {
						if (myOBJECTS[object_num].PARAMETER[n].vint == 1) {
							sprintf(value_str, _("On"));
						} else {
							sprintf(value_str, _("Off"));
						}
					} else if (myOBJECTS[object_num].PARAMETER[n].type == T_STRING) {
						sprintf(value_str, "%s", myOBJECTS[object_num].PARAMETER[n].vstr);
					} else if (myOBJECTS[object_num].PARAMETER[n].type == T_FILE) {
						sprintf(value_str, "%s", myOBJECTS[object_num].PARAMETER[n].vstr);
					} else {
						continue;
					}

					if (strcmp(value, value_str) != 0) {
//						g_print("#%s: %s/%s: %s -> %s\n", object, group, name, value, value_str);
						gtk_tree_store_set(treestore, iter, COL_VALUE, value_str, -1);
					}

					if ((int)overwrite != (int)myOBJECTS[object_num].PARAMETER[n].overwrite) {

/*
						tree_path_str2 = gtk_tree_path_to_string(path);
						tree_path_str2[5] = 0;
						GtkTreePath *path2 = gtk_tree_path_new_from_string(tree_path_str2);
						GtkTreeIter iter2;
						if (gtk_tree_model_get_iter(model, &iter2, path2)) {
							printf("overwrite: %s %s\n", tree_path_str, tree_path_str2);
							gtk_tree_store_set(treestore, &iter2, COL_OPTION, "overwrites", -1);
						}
*/

						if (myOBJECTS[object_num].PARAMETER[n].overwrite == 1) {
							gtk_tree_store_set(treestore, iter, COL_OVERWRITE, TRUE, -1);
						} else {
							gtk_tree_store_set(treestore, iter, COL_OVERWRITE, FALSE, -1);
						}
					}
				}
			}
			g_free(tree_path_str);
		}
	}
	if (group != NULL) {
		g_free(group);
	}
	if (name != NULL) {
		g_free(name);
	}
	if (value != NULL) {
		g_free(value);
	}
    return FALSE;
}

void update_tree_values (void) {
	int n = 0;
	int object_num = 0;
	for (object_num = 0; object_num < object_last; object_num++) {
		for (n = 0; n < P_LAST; n++) {
			if (myOBJECTS[object_num].PARAMETER[n].overwrite == 0) {
				strcpy(myOBJECTS[object_num].PARAMETER[n].name, PARAMETER[n].name);
				strcpy(myOBJECTS[object_num].PARAMETER[n].group, PARAMETER[n].group);
				myOBJECTS[object_num].PARAMETER[n].type = PARAMETER[n].type;
				myOBJECTS[object_num].PARAMETER[n].vint = PARAMETER[n].vint;
				myOBJECTS[object_num].PARAMETER[n].vfloat = PARAMETER[n].vfloat;
				myOBJECTS[object_num].PARAMETER[n].vdouble = PARAMETER[n].vdouble;
				strcpy(myOBJECTS[object_num].PARAMETER[n].vstr, PARAMETER[n].vstr);
				myOBJECTS[object_num].PARAMETER[n].min = PARAMETER[n].min;
				myOBJECTS[object_num].PARAMETER[n].step = PARAMETER[n].step;
				myOBJECTS[object_num].PARAMETER[n].max = PARAMETER[n].max;
			}
		}
	}
	gtk_tree_model_foreach(GTK_TREE_MODEL(treestore), update_tree_foreach_func, NULL);
}

gboolean delete_object_foreach_func (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data) {
	gchar *object = NULL;
	gchar *group = NULL;
	gtk_tree_model_get(model, iter, COL_OBJECT, &object, COL_GROUP, &group, -1);
	if (object != NULL && group != NULL) {
		if (strcmp(object, "-1") != 0 && group[0] == 0) {
			gtk_tree_store_remove(treestore, iter);
			g_free(object);
			g_free(group);
			return TRUE;
		}
		g_free(object);
		g_free(group);
	}
    return FALSE;
}


void objtree_add_object (int object_num) {
	GtkTreeIter child;
	GtkTreeIter grouplevel[G_LAST];
	int ng = 0;
	char tmp_str[1024];
	char tmp_str2[1024];
	sprintf(tmp_str, "#%i ", object_num);
	if (myOBJECTS[object_num].closed == 1) {
		strcat(tmp_str, "closed ");
	} else {
		strcat(tmp_str, "open ");
	}
	if (myOBJECTS[object_num].inside == 1) {
		strcat(tmp_str, "inside ");
	} else {
		strcat(tmp_str, "outside ");
	}
	strcat(tmp_str, "layer ");
	strcat(tmp_str, myOBJECTS[object_num].layer);
	if (myOBJECTS[object_num].clone != -1) {
		sprintf(tmp_str2, "clone of #%i ", myOBJECTS[object_num].clone);
		strcat(tmp_str, tmp_str2);
	}
	sprintf(tmp_str2, "%i", object_num);
	gtk_tree_store_append(treestore, &myOBJECTS[object_num].level, &toplevels[TREE_OBJECTS]);
	gtk_tree_store_set(treestore, &myOBJECTS[object_num].level,
		COL_OPTION, tmp_str,
		COL_OBJECT, tmp_str2,
		COL_GROUP, "",
		COL_NAME, "use",
		COL_VALUE, "",
		COL_OVERWRITE, TRUE,
	-1);
	for (ng = 0; ng < G_LAST; ng++) {
		if (strcmp(GROUPS[ng].name, "View") == 0) {
		} else if (strcmp(GROUPS[ng].name, "Machine") == 0) {
		} else if (strcmp(GROUPS[ng].name, "Misc") == 0) {
		} else if (strcmp(GROUPS[ng].name, "Rotary") == 0) {
		} else if (strcmp(GROUPS[ng].name, "Tangencial") == 0) {
		} else if (strcmp(GROUPS[ng].name, "Bitmap") == 0) {
		} else {
			gtk_tree_store_append(treestore, &grouplevel[ng], &myOBJECTS[object_num].level);
			gtk_tree_store_set(treestore, &grouplevel[ng],
				COL_OPTION, _(GROUPS[ng].name),
				COL_OBJECT, "-1",
			-1);
			int n = 0;
			for (n = 0; n < P_LAST; n++) {
				if (strcmp(myOBJECTS[object_num].PARAMETER[n].group, GROUPS[ng].name) == 0) {
					gtk_tree_store_append(treestore, &child, &grouplevel[ng]);
					gtk_tree_store_set(treestore, &child,
						COL_OPTION, _(myOBJECTS[object_num].PARAMETER[n].name),
						COL_VALUE, "???",
						COL_GROUP, myOBJECTS[object_num].PARAMETER[n].group,
						COL_NAME, myOBJECTS[object_num].PARAMETER[n].name,
						COL_OBJECT, tmp_str2,
						COL_OVERWRITE, FALSE,
					-1);
				}
			}
		}
	}
}

void fill_objtree (void) {
	int object_num = 0;
	for (object_num = 0; object_num < object_last; object_num++) {
		gtk_tree_model_foreach(GTK_TREE_MODEL(treestore), delete_object_foreach_func, NULL);
	}
	int order_num = 0;
	for (order_num = 0; order_num < object_last; order_num++) {
		for (object_num = 0; object_num < object_last; object_num++) {
			if (order_num == myOBJECTS[object_num].order && myOBJECTS[object_num].line[0] != 0) {
				objtree_add_object(object_num);
			}
		}
	}
}


GtkTreeModel *create_and_fill_model (void) {
	GtkTreeIter child;
	GtkTreeIter grouplevel[G_LAST];
	GtkTreeIter systemtop;
	treestore = gtk_tree_store_new(NUM_COLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN);


	gtk_tree_store_append(treestore, &systemtop, NULL);
	gtk_tree_store_set(treestore, &systemtop, COL_OPTION, _("System"), COL_OBJECT, "-1", -1);


	gtk_tree_store_append(treestore, &toplevels[TREE_VIEW], &systemtop);
	gtk_tree_store_set(treestore, &toplevels[TREE_VIEW], COL_OPTION, _("View"), COL_OBJECT, "-1", -1);
	gtk_tree_store_append(treestore, &toplevels[TREE_MACHINE], &systemtop);
	gtk_tree_store_set(treestore, &toplevels[TREE_MACHINE], COL_OPTION, _("Machine"), COL_OBJECT, "-1", -1);
	gtk_tree_store_append(treestore, &toplevels[TREE_ROTARY], &systemtop);
	gtk_tree_store_set(treestore, &toplevels[TREE_ROTARY], COL_OPTION, _("Rotary"), COL_OBJECT, "-1", -1);
	gtk_tree_store_append(treestore, &toplevels[TREE_TANGENCIAL], &systemtop);
	gtk_tree_store_set(treestore, &toplevels[TREE_TANGENCIAL], COL_OPTION, _("Tangencial"), COL_OBJECT, "-1", -1);
	gtk_tree_store_append(treestore, &toplevels[TREE_BITMAP], &systemtop);
	gtk_tree_store_set(treestore, &toplevels[TREE_BITMAP], COL_OPTION, _("Bitmap"), COL_OBJECT, "-1", -1);
	gtk_tree_store_append(treestore, &toplevels[TREE_MISC], &systemtop);
	gtk_tree_store_set(treestore, &toplevels[TREE_MISC], COL_OPTION, _("Misc"), COL_OBJECT, "-1", -1);

	gtk_tree_store_append(treestore, &toplevels[TREE_GLOBALS], NULL);
	gtk_tree_store_set(treestore, &toplevels[TREE_GLOBALS], COL_OPTION, _("Settings (for all Objects)"), COL_OBJECT, "-1", -1);
	gtk_tree_store_append(treestore, &toplevels[TREE_OBJECTS], NULL);
	gtk_tree_store_set(treestore, &toplevels[TREE_OBJECTS], COL_OPTION, _("Objects"), COL_OBJECT, "-1", -1);
	int ng = 0;
	for (ng = 0; ng < G_LAST; ng++) {
		GtkTreeIter *master;
		if (strcmp(GROUPS[ng].name, "View") == 0) {
			master = &toplevels[TREE_VIEW];
		} else if (strcmp(GROUPS[ng].name, "Machine") == 0) {
			master = &toplevels[TREE_MACHINE];
		} else if (strcmp(GROUPS[ng].name, "Rotary") == 0) {
			master = &toplevels[TREE_ROTARY];
		} else if (strcmp(GROUPS[ng].name, "Tangencial") == 0) {
			master = &toplevels[TREE_TANGENCIAL];
		} else if (strcmp(GROUPS[ng].name, "Bitmap") == 0) {
			master = &toplevels[TREE_BITMAP];
		} else if (strcmp(GROUPS[ng].name, "Misc") == 0) {
			master = &toplevels[TREE_MISC];
		} else {
			gtk_tree_store_append(treestore, &grouplevel[ng], &toplevels[TREE_GLOBALS]);
			gtk_tree_store_set(treestore, &grouplevel[ng],
				COL_OPTION, _(GROUPS[ng].name),
				COL_OBJECT, "-1",
			-1);
			master = &grouplevel[ng];
		}
		int n = 0;
		for (n = 0; n < P_LAST; n++) {
			if (strcmp(PARAMETER[n].group, GROUPS[ng].name) == 0) {
				gtk_tree_store_append(treestore, &child, master);
				gtk_tree_store_set(treestore, &child,
					COL_OPTION, _(PARAMETER[n].name),
					COL_VALUE, "",
					COL_GROUP, PARAMETER[n].group,
					COL_NAME, PARAMETER[n].name,
					COL_OBJECT, "-1",
				-1);
			}
		}
	}
	update_tree_values();
	return GTK_TREE_MODEL(treestore);
}


void view_onRowActivated (GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer userdata) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	model = gtk_tree_view_get_model(treeview);
	if (gtk_tree_model_get_iter(model, &iter, path)) {
		gchar *group = NULL;
		gchar *name = NULL;
		gchar *value = NULL;
		gchar *object = NULL;
		gtk_tree_model_get(model, &iter, COL_GROUP, &group, COL_NAME, &name, COL_VALUE, &value, COL_OBJECT, &object, -1);
		if (object != NULL && group != NULL && name != NULL && value != NULL) {
			if (strcmp(object, "-1") == 0) {
				int n = 0;
				for (n = 0; n < P_LAST; n++) {
					if (strcmp(PARAMETER[n].group, group) == 0 && strcmp(PARAMETER[n].name, name) == 0 && PARAMETER[n].readonly == 0) {
						if (PARAMETER[n].type == T_BOOL) {
							PARAMETER[n].vint = 1 - PARAMETER[n].vint;
							update_tree_values();
							update_post = 1;
						}
					}
				}
			} else {
				int object_num = atoi(object);
				int n = 0;
				for (n = 0; n < P_LAST; n++) {
					if (strcmp(myOBJECTS[object_num].PARAMETER[n].group, group) == 0 && strcmp(myOBJECTS[object_num].PARAMETER[n].name, name) == 0 && PARAMETER[n].readonly == 0) {
						if (myOBJECTS[object_num].PARAMETER[n].type == T_BOOL) {
							myOBJECTS[object_num].PARAMETER[n].vint = 1 - myOBJECTS[object_num].PARAMETER[n].vint;
							myOBJECTS[object_num].PARAMETER[n].overwrite = 1;
							update_tree_values();
							update_post = 1;
						}
					}
				}
			}
		}
		if (group != NULL) {
			g_free(group);
		}
		if (name != NULL) {
			g_free(name);
		}
		if (value != NULL) {
			g_free(value);
		}
		if (object != NULL) {
			g_free(object);
		}
	}
}

void overwrite_cb (GtkCellRendererToggle *cell, gchar *path_str, gpointer data) {
	GtkTreeIter iter;
	GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(TreeBox));
	gtk_tree_model_get_iter(model, &iter, path);
	gchar *group = NULL;
	gchar *name = NULL;
	gchar *object = NULL;
	gtk_tree_model_get(model, &iter, COL_GROUP, &group, COL_NAME, &name, COL_OBJECT, &object, -1);
	if (object != NULL && group != NULL && name != NULL) {
		if (strcmp(object, "-1") != 0) {
			if (strcmp(group, "") == 0 && strcmp(name, "use") == 0) {
				int object_num = atoi(object);
				myOBJECTS[object_num].use = 1 - myOBJECTS[object_num].use;
				update_post = 1;
			}
		}
		int n = 0;
		int object_num = atoi(object);
		for (n = 0; n < P_LAST; n++) {
			if (strcmp(object, "-1") != 0) {
				if (strcmp(myOBJECTS[object_num].PARAMETER[n].group, group) == 0 && strcmp(myOBJECTS[object_num].PARAMETER[n].name, name) == 0 && PARAMETER[n].readonly == 0) {
					myOBJECTS[object_num].PARAMETER[n].overwrite = 1 - myOBJECTS[object_num].PARAMETER[n].overwrite;
					update_post = 1;
				}
			}
		}
	}
	if (group != NULL) {
		g_free(group);
	}
	if (name != NULL) {
		g_free(name);
	}
	if (object != NULL) {
		g_free(object);
	}
	gtk_tree_path_free (path);
}







GtkTreeIter iter_sel;
int object_sel = -1;

gboolean view_selection_func (GtkTreeSelection *selection, GtkTreeModel *model, GtkTreePath *path, gboolean path_currently_selected, gpointer userdata) {
	GtkTreeIter iter;
	if (gtk_tree_model_get_iter(model, &iter, path)) {
		gchar *object = NULL;
		gtk_tree_model_get(model, &iter, COL_OBJECT, &object, -1);
		if (!path_currently_selected) {
			if (gtk_tree_model_get_iter(model, &iter_sel, path)) {
				object_sel = atoi(object);
			}
		}
		if (object != NULL) {
			g_free(object);
		}
	}
	return TRUE;
}




void popup_menu_move_down (GtkWidget *menuitem, gpointer userdata) {
	GtkTreeIter iter = iter_sel;
	gtk_tree_model_iter_next(GTK_TREE_MODEL(treestore), &iter_sel);
	gtk_tree_store_move_after(treestore, &iter, &iter_sel);
	update_post = 1;
}

void popup_menu_clone_object (GtkWidget *menuitem, gpointer userdata) {
	int object_num = object_last;
	object_last++;
	memcpy((void *)&myOBJECTS[object_num], (void *)&myOBJECTS[object_sel], sizeof(_OBJECT));
	myOBJECTS[object_num].clone = object_sel;
	objtree_add_object(object_num);
	update_post = 1;
}


void view_popup_menu (GtkWidget *treeview, GdkEventButton *event, gpointer userdata) {
	GtkWidget *menu, *menuitem;
	menu = gtk_menu_new();

	menuitem = gtk_menu_item_new_with_label(_("Move down..."));
	g_signal_connect(menuitem, "activate", (GCallback)popup_menu_move_down, treeview);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_menu_item_new_with_label(_("Clone this Object..."));
	g_signal_connect(menuitem, "activate", (GCallback)popup_menu_clone_object, treeview);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	gtk_widget_show_all(menu);
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, (event != NULL) ? event->button : 0, gdk_event_get_time((GdkEvent*)event));
}

int count_char_in_string (const char *string, char c) {
	int num = 0;
	int n = 0;
	if (string == NULL) {
		return -1;
	}
	for (n = 0; n < strlen(string); n++) {
		if (string[n] == c) {
			num++;
		}
	}
	return num;
}

gboolean view_onButtonPressed (GtkWidget *treeview, GdkEventButton *event, gpointer userdata) {
	if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
		GtkTreeSelection *selection;
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
		if (gtk_tree_selection_count_selected_rows(selection)  <= 1) {
			GtkTreePath *path;
			if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), (gint)event->x, (gint)event->y, &path, NULL, NULL, NULL)) {
				gtk_tree_selection_unselect_all(selection);
				gtk_tree_selection_select_path(selection, path);
				char *tree_path_str = gtk_tree_path_to_string(path);
				int cnum = count_char_in_string(tree_path_str, ':');
				if (tree_path_str[0] == PATH_OBJECTS && cnum == 1) {
					view_popup_menu(treeview, event, userdata);
				}
				gtk_tree_path_free(path);
			}
		}
		return TRUE;
	}
	return FALSE;
}

gboolean view_onPopupMenu (GtkWidget *treeview, gpointer userdata) {
	view_popup_menu(treeview, NULL, userdata);
	return TRUE;
}




static GtkWidget *create_view_and_model (void) {
	GtkTreeViewColumn *col = NULL;
	GtkCellRenderer *renderer = NULL;
	GtkWidget *view = NULL;
	GtkTreeModel *model = NULL;
	view = gtk_tree_view_new();

	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, _("Option"));
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", COL_OPTION);

	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, _("Value"));
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
	gtk_tree_view_column_set_expand(col, TRUE);
	renderer = gui_cell_renderer_param_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", COL_VALUE);
	g_object_set(renderer, "editable", TRUE, NULL);

	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "");
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
	gtk_tree_view_column_set_resizable(col, FALSE);
	gtk_tree_view_column_set_fixed_width(col, 30);
	gtk_tree_view_column_set_max_width(col, 30);
	gtk_tree_view_column_set_expand(col, FALSE);
	renderer = gtk_cell_renderer_toggle_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "active", COL_OVERWRITE);
//	gtk_tree_view_column_add_attribute(col, renderer, "activatable", COL_OVERWRITE);
	g_signal_connect(G_OBJECT(renderer), "toggled", G_CALLBACK(overwrite_cb), NULL);

	model = create_and_fill_model();
	gtk_tree_view_set_model(GTK_TREE_VIEW(view), model);

//	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(view), TRUE);

    g_signal_connect(view, "button-press-event", (GCallback)view_onButtonPressed, NULL);
    g_signal_connect(view, "popup-menu", (GCallback)view_onPopupMenu, NULL);



	g_object_unref(model);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(view)), GTK_SELECTION_SINGLE);
	gtk_tree_selection_set_select_function(gtk_tree_view_get_selection(GTK_TREE_VIEW(view)), view_selection_func, NULL, NULL);
	g_signal_connect(view, "row-activated", (GCallback)view_onRowActivated, NULL);



	return view;
}

void create_menu (void) {
	MenuBar = gtk_menu_bar_new();
	GtkWidget *MenuItem;
	GtkWidget *FileMenu = gtk_menu_item_new_with_mnemonic(_("_File"));
	GtkWidget *FileMenuList = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(FileMenu), FileMenuList);
	gtk_menu_bar_append(GTK_MENU_BAR(MenuBar), FileMenu);

	accel_group = gtk_accel_group_new();

	MenuItem = gtk_menu_item_new_with_mnemonic(_("_Load DXF"));
	gtk_menu_append(GTK_MENU(FileMenuList), MenuItem);
	gtk_signal_connect(GTK_OBJECT(MenuItem), "activate", GTK_SIGNAL_FUNC(handler_load_dxf), NULL);
	gtk_widget_add_accelerator(MenuItem, "activate", accel_group, GDK_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
			
	MenuItem = gtk_menu_item_new_with_mnemonic(_("_Reload DXF"));
	gtk_menu_append(GTK_MENU(FileMenuList), MenuItem);
	gtk_signal_connect(GTK_OBJECT(MenuItem), "activate", GTK_SIGNAL_FUNC(handler_reload_dxf), NULL);
	gtk_widget_add_accelerator(MenuItem, "activate", accel_group, GDK_r, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
			
	MenuItem = gtk_menu_item_new_with_mnemonic(_("_Save Output As.."));
	gtk_menu_append(GTK_MENU(FileMenuList), MenuItem);
	gtk_signal_connect(GTK_OBJECT(MenuItem), "activate", GTK_SIGNAL_FUNC(handler_save_gcode_as), NULL);
	gtk_widget_add_accelerator(MenuItem, "activate", accel_group, GDK_s, GDK_CONTROL_MASK | GDK_SHIFT_MASK, GTK_ACCEL_VISIBLE);

	MenuItem = gtk_menu_item_new_with_mnemonic(_("_Save Output"));
	gtk_menu_append(GTK_MENU(FileMenuList), MenuItem);
	gtk_signal_connect(GTK_OBJECT(MenuItem), "activate", GTK_SIGNAL_FUNC(handler_save_gcode), NULL);
	gtk_widget_add_accelerator(MenuItem, "activate", accel_group, GDK_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

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
}

void create_toolbar (void) {
	GtkWidget *MenuItem;
	GtkWidget *ToolsMenu = gtk_menu_item_new_with_mnemonic(_("_Tools"));
	GtkWidget *ToolsMenuList = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(ToolsMenu), ToolsMenuList);
	gtk_menu_bar_append(GTK_MENU_BAR(MenuBar), ToolsMenu);

	MenuItem = gtk_menu_item_new_with_label(_("mm->inch"));
	gtk_menu_append(GTK_MENU(ToolsMenuList), MenuItem);
	gtk_signal_connect(GTK_OBJECT(MenuItem), "activate", GTK_SIGNAL_FUNC(handler_tool_mm2inch), NULL);
	MenuItem = gtk_menu_item_new_with_label(_("inch->mm"));
	gtk_menu_append(GTK_MENU(ToolsMenuList), MenuItem);
	gtk_signal_connect(GTK_OBJECT(MenuItem), "activate", GTK_SIGNAL_FUNC(handler_tool_inch2mm), NULL);
	MenuItem = gtk_menu_item_new_with_label(_("rotate"));
	gtk_menu_append(GTK_MENU(ToolsMenuList), MenuItem);
	gtk_signal_connect(GTK_OBJECT(MenuItem), "activate", GTK_SIGNAL_FUNC(handler_rotate_drawing), NULL);
	MenuItem = gtk_menu_item_new_with_label(_("flip-x"));
	gtk_menu_append(GTK_MENU(ToolsMenuList), MenuItem);
	gtk_signal_connect(GTK_OBJECT(MenuItem), "activate", GTK_SIGNAL_FUNC(handler_flip_x_drawing), NULL);
	MenuItem = gtk_menu_item_new_with_label(_("flip-y"));
	gtk_menu_append(GTK_MENU(ToolsMenuList), MenuItem);
	gtk_signal_connect(GTK_OBJECT(MenuItem), "activate", GTK_SIGNAL_FUNC(handler_flip_y_drawing), NULL);

	GtkWidget *HelpMenu = gtk_menu_item_new_with_mnemonic(_("_Help"));
	GtkWidget *HelpMenuList = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(HelpMenu), HelpMenuList);
	gtk_menu_bar_append(GTK_MENU_BAR(MenuBar), HelpMenu);

	MenuItem = gtk_menu_item_new_with_label(_("About"));
	gtk_menu_append(GTK_MENU(HelpMenuList), MenuItem);
	gtk_signal_connect(GTK_OBJECT(MenuItem), "activate", GTK_SIGNAL_FUNC(handler_about), NULL);

	ToolBar = gtk_toolbar_new();
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

	char *iconpath = NULL;
	GtkToolItem *TB_Rotate;
	iconpath = path_real("../share/cammill/icons/object-rotate-right.png");
	TB_Rotate = gtk_tool_button_new(gtk_image_new_from_file(iconpath), "Rotate");
	free(iconpath);
	gtk_tool_item_set_tooltip_text(TB_Rotate, "Rotate 90°");
	gtk_toolbar_insert(GTK_TOOLBAR(ToolBar), TB_Rotate, -1);
	g_signal_connect(G_OBJECT(TB_Rotate), "clicked", GTK_SIGNAL_FUNC(handler_rotate_drawing), NULL);

	GtkToolItem *TB_FlipX;
	iconpath = path_real("../share/cammill/icons/object-flip-horizontal.png");
	TB_FlipX = gtk_tool_button_new(gtk_image_new_from_file(iconpath), "FlipX");
	free(iconpath);
	gtk_tool_item_set_tooltip_text(TB_FlipX, "Flip X");
	gtk_toolbar_insert(GTK_TOOLBAR(ToolBar), TB_FlipX, -1);
	g_signal_connect(G_OBJECT(TB_FlipX), "clicked", GTK_SIGNAL_FUNC(handler_flip_x_drawing), NULL);

	GtkToolItem *TB_FlipY;
	iconpath = path_real("../share/cammill/icons/object-flip-vertical.png");
	TB_FlipY = gtk_tool_button_new(gtk_image_new_from_file(iconpath), "FlipY");
	free(iconpath);
	gtk_tool_item_set_tooltip_text(TB_FlipY, "Flip Y");
	gtk_toolbar_insert(GTK_TOOLBAR(ToolBar), TB_FlipY, -1);
	g_signal_connect(G_OBJECT(TB_FlipY), "clicked", GTK_SIGNAL_FUNC(handler_flip_y_drawing), NULL);


	GtkToolItem *TB_ViewR;
	iconpath = path_real("../share/cammill/icons/view-reset.png");
	TB_ViewR = gtk_tool_button_new(gtk_image_new_from_file(iconpath), "reset View");
	free(iconpath);
	gtk_tool_item_set_tooltip_text(TB_ViewR, "reset View");
	gtk_toolbar_insert(GTK_TOOLBAR(ToolBar), TB_ViewR, -1);
	g_signal_connect(G_OBJECT(TB_ViewR), "clicked", GTK_SIGNAL_FUNC(handler_reset_view), NULL);


#ifdef __linux__
	if (access("/usr/bin/camotics", F_OK) != -1) {
		GtkToolItem *ToolItemSepPV = gtk_separator_tool_item_new();
		gtk_toolbar_insert(GTK_TOOLBAR(ToolBar), ToolItemSepPV, -1); 
		GtkToolItem *TB_Preview;
		iconpath = path_real("../share/cammill/icons/preview.png");
		TB_Preview = gtk_tool_button_new(gtk_image_new_from_file(iconpath), "Preview with Camotics");
		free(iconpath);
		gtk_tool_item_set_tooltip_text(TB_Preview, "Preview");
		gtk_toolbar_insert(GTK_TOOLBAR(ToolBar), TB_Preview, -1);
		g_signal_connect(G_OBJECT(TB_Preview), "clicked", GTK_SIGNAL_FUNC(handler_preview), NULL);
	}
#endif

	GtkToolItem *ToolItemSep2 = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(ToolBar), ToolItemSep2, -1); 

}


void create_gui () {
	GtkWidget *vbox;
	int GroupNum[G_LAST];
	int n = 0;
	for (n = 0; n < P_LAST; n++) {
		ListStore[n] = NULL;
		if (PARAMETER[n].type == T_SELECT) {
			ListStore[n] = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
		}
	}

	glCanvas = create_gl();
	gtk_widget_set_usize(GTK_WIDGET(glCanvas), 800, 600);
	gtk_signal_connect(GTK_OBJECT(glCanvas), "expose_event", GTK_SIGNAL_FUNC(handler_draw), NULL);  
	gtk_signal_connect(GTK_OBJECT(glCanvas), "button_press_event", GTK_SIGNAL_FUNC(handler_button_press), NULL);  
	gtk_signal_connect(GTK_OBJECT(glCanvas), "button_release_event", GTK_SIGNAL_FUNC(handler_button_release), NULL);  
	gtk_signal_connect(GTK_OBJECT(glCanvas), "configure_event", GTK_SIGNAL_FUNC(handler_configure), NULL);  
	gtk_signal_connect(GTK_OBJECT(glCanvas), "motion_notify_event", GTK_SIGNAL_FUNC(handler_motion), NULL);  
	gtk_signal_connect(GTK_OBJECT(glCanvas), "key_press_event", GTK_SIGNAL_FUNC(handler_keypress), NULL);  
	gtk_signal_connect(GTK_OBJECT(glCanvas), "scroll-event", GTK_SIGNAL_FUNC(handler_scrollwheel), NULL);  
//	gtk_signal_connect(GTK_OBJECT(window),   "window-state-event", G_CALLBACK(window_event), NULL);
//	g_signal_connect(G_OBJECT(window), "window-state-event", G_CALLBACK(window_event), NULL);

	// Drag & Drop
	gtk_drag_dest_set(glCanvas, GTK_DEST_DEFAULT_ALL, NULL, 0, GDK_ACTION_COPY);
	gtk_drag_dest_add_text_targets(glCanvas);
	gtk_drag_dest_add_uri_targets(glCanvas);
	g_signal_connect(GTK_OBJECT(glCanvas), "drag-drop", G_CALLBACK(DnDdrop), NULL);
	g_signal_connect(GTK_OBJECT(glCanvas), "drag-motion", G_CALLBACK(DnDmotion), NULL);
	g_signal_connect(GTK_OBJECT(glCanvas), "drag-data-received", G_CALLBACK(DnDreceive), NULL);
	g_signal_connect(GTK_OBJECT(glCanvas), "drag-leave", G_CALLBACK(DnDleave), NULL);

	// top-menu
	create_menu();
	create_toolbar();


	TreeBox = create_view_and_model();
	GtkWidget *NtBox = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(NtBox), TreeBox);


	for (n = 0; n < G_LAST; n++) {
		GroupNum[n] = 0;
	}

	gtk_widget_set_size_request(NtBox, 300, -1);

//	LayerLoadList();
	loading = 0;

	gtk_list_store_insert_with_values(ListStore[P_O_OFFSET], NULL, -1, 0, NULL, 1, _("None"), -1);
	gtk_list_store_insert_with_values(ListStore[P_O_OFFSET], NULL, -1, 0, NULL, 1, _("Inside"), -1);
	gtk_list_store_insert_with_values(ListStore[P_O_OFFSET], NULL, -1, 0, NULL, 1, _("Outside"), -1);

	gtk_list_store_insert_with_values(ListStore[P_M_COOLANT], NULL, -1, 0, NULL, 1, _("Off"), -1);
	gtk_list_store_insert_with_values(ListStore[P_M_COOLANT], NULL, -1, 0, NULL, 1, _("Mist"), -1);
	gtk_list_store_insert_with_values(ListStore[P_M_COOLANT], NULL, -1, 0, NULL, 1, _("Flood"), -1);

	gtk_list_store_insert_with_values(ListStore[P_H_ROTARYAXIS], NULL, -1, 0, NULL, 1, "A", -1);
	gtk_list_store_insert_with_values(ListStore[P_H_ROTARYAXIS], NULL, -1, 0, NULL, 1, "B", -1);
	gtk_list_store_insert_with_values(ListStore[P_H_ROTARYAXIS], NULL, -1, 0, NULL, 1, "C", -1);

	gtk_list_store_insert_with_values(ListStore[P_H_KNIFEAXIS], NULL, -1, 0, NULL, 1, "A", -1);
	gtk_list_store_insert_with_values(ListStore[P_H_KNIFEAXIS], NULL, -1, 0, NULL, 1, "B", -1);
	gtk_list_store_insert_with_values(ListStore[P_H_KNIFEAXIS], NULL, -1, 0, NULL, 1, "C", -1);

	gtk_list_store_insert_with_values(ListStore[P_O_UNIT_LOAD], NULL, -1, 0, NULL, 1, "inch", -1);
	gtk_list_store_insert_with_values(ListStore[P_O_UNIT_LOAD], NULL, -1, 0, NULL, 1, "mm", -1);
	gtk_list_store_insert_with_values(ListStore[P_O_UNIT_LOAD], NULL, -1, 0, NULL, 1, "auto", -1);

	gtk_list_store_insert_with_values(ListStore[P_O_UNIT], NULL, -1, 0, NULL, 1, "inch", -1);
	gtk_list_store_insert_with_values(ListStore[P_O_UNIT], NULL, -1, 0, NULL, 1, "mm", -1);

	gtk_list_store_insert_with_values(ListStore[P_M_ZERO], NULL, -1, 0, NULL, 1, "bottom-left", -1);
	gtk_list_store_insert_with_values(ListStore[P_M_ZERO], NULL, -1, 0, NULL, 1, "original", -1);
	gtk_list_store_insert_with_values(ListStore[P_M_ZERO], NULL, -1, 0, NULL, 1, "center", -1);

	gtk_list_store_insert_with_values(ListStore[P_M_ORDER], NULL, -1, 0, NULL, 1, "inside/open first", -1);
	gtk_list_store_insert_with_values(ListStore[P_M_ORDER], NULL, -1, 0, NULL, 1, "next", -1);
	gtk_list_store_insert_with_values(ListStore[P_M_ORDER], NULL, -1, 0, NULL, 1, "none", -1);


	ParameterUpdate();
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

	GtkWidget *NbBox = gtk_table_new(2, 2, FALSE);
	notebook = gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
	gtk_table_attach_defaults(GTK_TABLE(NbBox), notebook, 0, 1, 0, 1);

	GtkWidget *glCanvasLabel = gtk_label_new(_("3D-View"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), glCanvas, glCanvasLabel);

	gCodeViewLabel = gtk_label_new(_("Output"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), textWidget, gCodeViewLabel);

	gCodeViewLabelLua = gtk_label_new(_("PostProcessor"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), textWidgetLuaBox, gCodeViewLabelLua);


	gtk_label_set_text(GTK_LABEL(OutputErrorLabel), output_info);
	
	


#ifdef USE_VNC
	if (PARAMETER[P_O_VNCSERVER].vstr[0] != 0) {
		char port[128];
		GtkWidget *VncLabel = gtk_label_new(_("VNC"));
		VncView = vnc_display_new();
		GtkWidget *VncWindow = gtk_scrolled_window_new(NULL, NULL);
		gtk_container_add(GTK_CONTAINER(VncWindow), VncView);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(VncWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook), VncWindow, VncLabel);
		snprintf(port, sizeof(port), "%i", PARAMETER[P_O_VNCPORT].vint);
		vnc_display_set_scaling(VNC_DISPLAY(VncView), TRUE);
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
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), WebKitBox, WebKitLabel);
	webkit_web_view_open(WEBKIT_WEB_VIEW(WebKit), "file:///usr/src/cammill/index.html");
#endif

/*
	Embedded Programms (-wid)
	GtkWidget *PlugLabel = gtk_label_new(_("PlugIn"));
	GtkWidget *sck = gtk_socket_new();
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), sck, PlugLabel);
*/

	hbox = gtk_hpaned_new();


	gtk_paned_pack1(GTK_PANED(hbox), NtBox, TRUE, TRUE);
	gtk_paned_pack2(GTK_PANED(hbox), NbBox, TRUE, TRUE);
	gtk_paned_set_position(GTK_PANED(hbox), PannedStat);


	SizeInfoLabel = gtk_label_new("Width=0 / Height=0");
	GtkWidget *SizeInfo = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(SizeInfo), SizeInfoLabel);
	gtk_container_set_border_width(GTK_CONTAINER(SizeInfo), 4);

	char iconfile[PATH_MAX];
	if (program_path[0] == 0) {
		snprintf(iconfile, PATH_MAX, "%s%s%s", "../share/cammill/icons", DIR_SEP, "logo-top.png");
	} else {
		snprintf(iconfile, PATH_MAX, "%s%s%s%s%s", program_path, DIR_SEP, "../share/cammill/icons", DIR_SEP, "logo-top.png");
	}
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

	if (program_path[0] == 0) {
		snprintf(iconfile, PATH_MAX, "%s%s%s%s", program_path, "../share/cammill/icons", DIR_SEP, "icon_128.png");
	} else {
		snprintf(iconfile, PATH_MAX, "%s%s%s%s%s", program_path, DIR_SEP, "../share/cammill/icons", DIR_SEP, "icon_128.png");
	}
	gtk_window_set_icon(GTK_WINDOW(window), create_pixbuf(iconfile));

	gtk_signal_connect(GTK_OBJECT(window), "destroy_event", GTK_SIGNAL_FUNC (handler_destroy), NULL);
	gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC (handler_destroy), NULL);
	gtk_container_add (GTK_CONTAINER(window), vbox);

	gtk_window_move(GTK_WINDOW(window), PARAMETER[P_W_POSX].vint, PARAMETER[P_W_POSY].vint);
	gtk_window_resize(GTK_WINDOW(window), PARAMETER[P_W_POSW].vint, PARAMETER[P_W_POSH].vint);
//	if (PARAMETER[P_W_MAX].vint == 1) {
//		gtk_window_maximize(PARAMETER[P_W_MAX].vint);
//	} else {
//		gtk_window_unmaximize(PARAMETER[P_W_MAX].vint);
//	}

	gtk_widget_show_all(window);

/*
	Embedded Programms (-wid)
	GdkNativeWindow nwnd = gtk_socket_get_id(GTK_SOCKET(sck));
	g_print("%i\n", nwnd);
*/
}



void load_files () {
	DIR *dir;
	int n = 0;
	struct dirent *ent;
	char dir_posts[PATH_MAX];
	if (program_path[0] == 0) {
		snprintf(dir_posts, PATH_MAX, "%s", "../lib/cammill/posts");
	} else {
		snprintf(dir_posts, PATH_MAX, "%s%s%s", program_path, DIR_SEP, "../lib/cammill/posts");
	}
	// fprintf(stderr, "postprocessor directory: '%s'\n", dir_posts);
	if ((dir = opendir(dir_posts)) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			if (ent->d_name[0] != '.') {
				char *pname = suffix_remove(ent->d_name);
				if (PARAMETER[P_O_BATCHMODE].vint != 1) {
					gtk_list_store_insert_with_values(ListStore[P_H_POST], NULL, -1, 0, NULL, 1, pname, -1);
				}
				strncpy(postcam_plugins[n], pname, sizeof(postcam_plugins[n]));
				n++;
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

	char dir_fonts[PATH_MAX];
	if (program_path[0] == 0) {
		snprintf(dir_fonts, PATH_MAX, "%s", "../share/cammill/fonts");
	} else {
		snprintf(dir_fonts, PATH_MAX, "%s%s%s", program_path, DIR_SEP, "../share/cammill/fonts");
	}
	if ((dir = opendir(dir_fonts)) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			if (ent->d_name[0] != '.') {
				char *pname = suffix_remove(ent->d_name);
				if (PARAMETER[P_O_BATCHMODE].vint != 1) {
					gtk_list_store_insert_with_values(ListStore[P_M_TEXT_FONT], NULL, -1, 0, NULL, 1, pname, -1);
				}
				free(pname);
			}
		}
		closedir (dir);
	} else {
		fprintf(stderr, "postprocessor directory not found: %s\n", dir_fonts);
	}

	MaterialLoadList(program_path);
	ToolLoadTable();

	int mat_num = PARAMETER[P_MAT_SELECT].vint;
	PARAMETER[P_MAT_CUTSPEED].vint = Material[mat_num].vc;
	PARAMETER[P_MAT_FEEDFLUTE4].vdouble = Material[mat_num].fz[FZ_FEEDFLUTE4];
	PARAMETER[P_MAT_FEEDFLUTE8].vdouble = Material[mat_num].fz[FZ_FEEDFLUTE8];
	PARAMETER[P_MAT_FEEDFLUTE12].vdouble = Material[mat_num].fz[FZ_FEEDFLUTE12];
	strncpy(PARAMETER[P_MAT_TEXTURE].vstr, Material[mat_num].texture, sizeof(PARAMETER[P_MAT_TEXTURE].vstr));

	/* import DXF */
	loading = 1;
	char filename[PATH_MAX];
	strcpy(filename, PARAMETER[P_V_DXF].vstr);
	main_mode = 0;
	if (PARAMETER[P_V_DXF].vstr[0] != 0) {
		if (strstr(PARAMETER[P_V_DXF].vstr, ".ngc") > 0 || strstr(PARAMETER[P_V_DXF].vstr, ".NGC") > 0) {
			SetupLoadFromGcode(PARAMETER[P_V_DXF].vstr);
			if (PARAMETER[P_V_DXF].vstr[0] != 0) {
				dxf_read(PARAMETER[P_V_DXF].vstr);
			}
		} else if (strstr(PARAMETER[P_V_DXF].vstr, ".dxf") > 0 || strstr(PARAMETER[P_V_DXF].vstr, ".DXF") > 0) {
			dxf_read(PARAMETER[P_V_DXF].vstr);
		} else if (strstr(PARAMETER[P_V_DXF].vstr, ".plt") > 0 || strstr(PARAMETER[P_V_DXF].vstr, ".PLT") > 0) {
			hpgl_read(PARAMETER[P_V_DXF].vstr);
#ifdef USE_BMPMODE
		} else if (strstr(PARAMETER[P_V_DXF].vstr, ".png") > 0 || strstr(PARAMETER[P_V_DXF].vstr, ".PNG") > 0) {
			main_mode = 1;
			bitmap_load(PARAMETER[P_V_DXF].vstr);
#endif
#ifdef USE_G3D
		} else {
			slice_3d(PARAMETER[P_V_DXF].vstr, 0.0);
#endif
		}
	}
	if (PARAMETER[P_V_DXF].vstr[0] != 0) {
		strncpy(PARAMETER[P_M_LOADPATH].vstr, PARAMETER[P_V_DXF].vstr, PATH_MAX);
		dirname(PARAMETER[P_M_LOADPATH].vstr);
	}
	init_objects();
	fill_objtree();
	if (strstr(filename, ".ngc") > 0 || strstr(filename, ".NGC") > 0) {
		SetupLoadFromGcodeObjects(filename);
	}
	loading = 0;
}

int main (int argc, char *argv[]) {
	char tmp_str[1024];

	get_executable_path(argv[0], program_path, sizeof(program_path));

//	bindtextdomain("cammill", "intl");
	textdomain("cammill");

	// force dots in printf/atof
	setlocale(LC_NUMERIC, "POSIX");
	SetupLoad();
	// set allways to 1.0, preset only via command line arguments
	PARAMETER[P_O_SCALE].vdouble = 1.0;
	ArgsRead(argc, argv);
//	SetupShow();

	strncpy(output_extension, "ngc", sizeof(output_extension));
	strncpy(output_info, "", sizeof(output_info));

	if (PARAMETER[P_O_BATCHMODE].vint == 1 && PARAMETER[P_MFILE].vstr[0] != 0) {
		save_gcode = 1;
		load_files();
	} else {
		gtk_init(&argc, &argv);
		if (gtk_gl_init_check(&argc, &argv) == FALSE) {
			fprintf(stderr, "init OpenGL failed\n");
		}
		// force dots in printf/atof after gtk_init :(
		setlocale(LC_NUMERIC, "POSIX");
		create_gui();
		load_files();
		gtk_label_set_text(GTK_LABEL(OutputErrorLabel), output_info);
		snprintf(tmp_str, sizeof(tmp_str), "%s (%s)", _("Output"), output_extension);
		gtk_label_set_text(GTK_LABEL(gCodeViewLabel), tmp_str);
	}
	if (PARAMETER[P_H_POST].vint != -1) {
		postcam_init_lua(program_path, postcam_plugins[PARAMETER[P_H_POST].vint]);
	}
	postcam_plugin = PARAMETER[P_H_POST].vint;	

	update_tree_values();

	if (PARAMETER[P_O_BATCHMODE].vint == 1 && PARAMETER[P_MFILE].vstr[0] != 0) {
		mainloop();
	} else {
		if (PARAMETER[P_H_POST].vint != -1) {
			postcam_load_source(postcam_plugins[PARAMETER[P_H_POST].vint]);
		}
		gtk_timeout_add(1000/25, handler_periodic_action, NULL);
		gtk_main();
	}
	postcam_exit_lua();

	return 0;
}


