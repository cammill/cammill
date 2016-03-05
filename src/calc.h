
enum {
	FZ_FEEDFLUTE4,
	FZ_FEEDFLUTE8,
	FZ_FEEDFLUTE12,
	FZ_LAST
};

typedef struct{
	int used;
	int vc;
	float fz[FZ_LAST];
	char *texture;
} _MATERIAL;

extern int MaterialMax;
extern _MATERIAL Material[100];


void object2poly (int object_num, double depth, double depth2, int invert);
void DrawLine (float x1, float y1, float z1, float x2, float y2, float z2, float w);
void DrawArrow (float x1, float y1, float x2, float y2, float z, float w);
void draw_line_wrap_conn (float x1, float y1, float depth1, float depth2);
void draw_line_wrap (float x1, float y1, float x2, float y2, float depth);
void draw_oline (float x1, float y1, float x2, float y2, float depth);
void draw_line2 (float x1, float y1, float z1, float x2, float y2, float z2, float width);
void draw_line (float x1, float y1, float z1, float x2, float y2, float z2, float width);
void draw_line3 (float x1, float y1, float z1, float x2, float y2, float z2);

void line_invert (int num);
int point_in_object (int object_num, int object_ex, double testx, double testy);
int point_in_object_old (int object_num, int object_ex, double testx, double testy);
void point_rotate (float y, float depth, float *ny, float *nz);
double _X (double x);
double _Y (double y);
double _Z (double z);
void translateAxisX (double x, char *ret_str);
void translateAxisY (double y, char *ret_str);
void translateAxisZ (double z, char *ret_str);
int object_line_last (int object_num);
double get_len (double x1, double y1, double x2, double y2);
double set_positive (double val);
void resort_object (int object_num, int start);
void redir_object (int object_num);
double line_len (int lnum);
double line_angle (int lnum);
double vector_angle (double x1, double y1, double x2, double y2);
double line_angle2 (int lnum);
void add_angle_offset (double *check_x, double *check_y, double radius, double alpha);
void object_optimize_dir (int object_num);
int intersect_check (double p0_x, double p0_y, double p1_x, double p1_y, double p2_x, double p2_y, double p3_x, double p3_y, double *i_x, double *i_y);
void intersect (double l1x1, double l1y1, double l1x2, double l1y2, double l2x1, double l2y1, double l2x2, double l2y2, double *x, double *y);
void mill_begin (const char* path);
void mill_end (void);
void mill_objects (void);
void mill_z (int gcmd, double z);
void mill_xy (int gcmd, double x, double y, double r, int feed, int object_num, char *comment);
void mill_drill (double x, double y, double depth, double last_depth, int feed, int object_num, char *comment);
void mill_circle (int gcmd, double x, double y, double r, double depth, int feed, int inside, int object_num, char *comment);
void mill_circle_helix (int gcmd, double x, double y, double r, double depth, int feed, int inside, int object_num, char *comment);
void mill_move_in (double x, double y, double depth, int lasermode, int object_num);
void mill_move_out (int lasermode, int object_num);
void object_draw (FILE *fd_out, int object_num);
void object_draw_offset_depth (FILE *fd_out, int object_num, double depth, double total_depth, double last_depth, double *next_x, double *next_y, double tool_offset, int overcut, int lasermode, int offset);
void object_draw_offset (FILE *fd_out, int object_num, double *next_x, double *next_y);
int find_next_line (int object_num, int first, int num, int dir, int depth);
int line_open_check (int num);
void init_objects (void);
void MaterialLoadList (const char* path);
void DrawCheckSize (void);
void DrawSetZero (void);
void remove_double_lines (void);


