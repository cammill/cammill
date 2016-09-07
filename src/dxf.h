
#define ARC_RAST 18.0

#define OPTION_LINE_X1		10
#define OPTION_LINE_Y1		20
#define OPTION_LINE_X2		11
#define OPTION_LINE_Y2		21

#define OPTION_POINT_X		10
#define OPTION_POINT_Y		20

#define OPTION_ARC_X		10
#define OPTION_ARC_Y		20
#define OPTION_ARC_RADIUS	40
#define OPTION_ARC_BEGIN	50
#define OPTION_ARC_END		51

#define OPTION_SPLINE_FLAG	70 // 1 = Closed spline, 2 = Periodic spline, 4 = Rational spline, 8 = Planar, 16 = Linear (planar bit is also set)
#define OPTION_SPLINE_DEG	71 // Degree of the spline curve
#define OPTION_SPLINE_NK	72 // Number of knots
#define OPTION_SPLINE_CP	73 // Number of control points
#define OPTION_SPLINE_FP	74 // Number of fit points (if any)
#define OPTION_SPLINE_KT	42 // Knot tolerance (default = 0.0000001)
#define OPTION_SPLINE_CT	43 // Control-point tolerance (default = 0.0000001)
#define OPTION_SPLINE_FT	44 // Fit tolerance (default = 0.0000000001)
#define OPTION_SPLINE_SX	12 // Start tangent-may be omitted (in WCS) DXF: X value
#define OPTION_SPLINE_SY	22 // Y value
#define OPTION_SPLINE_SZ	32 // Z value
#define OPTION_SPLINE_EX	13 // End tangent-may be omitted (in WCS) DXF: X value
#define OPTION_SPLINE_EY	23 // Y value
#define OPTION_SPLINE_EZ	33 // Z value
#define OPTION_SPLINE_KV	40 // Knot value (one entry per knot)
#define OPTION_SPLINE_W		41 // Weight (if not 1); with multiple group pairs, are present if all are not 1
#define OPTION_SPLINE_CX	10 // Control points (in WCS), one entry per control point DXF: X value; APP: 3D point
#define OPTION_SPLINE_CY	20 // Y
#define OPTION_SPLINE_CZ	30 // Z
#define OPTION_SPLINE_FX	11 // Fit points (in WCS), one entry per fit point DXF: X value; APP: 3D point
#define OPTION_SPLINE_FY	21 // Y
#define OPTION_SPLINE_FZ	31 // Z

#define OPTION_MTEXT_X		10
#define OPTION_MTEXT_Y		20
#define OPTION_MTEXT_SIZE	40
#define OPTION_MTEXT_TEXT	 1
#define OPTION_BLOCKNAME	 3
#define OPTION_LAYERNAME	 8

#define OPTION_MEASUREMENT	70

#define PI 3.14159265358979323846264338327950288419716939937511
#define toDeg(x) (x*180.0)/PI
#define toRad(x) (PI/180.0) * x

#define MAX_LINES		10000
#define MAX_OBJECTS		10000

enum {
	OFFSET_NONE,
	OFFSET_INSIDE,
	OFFSET_OUTSIDE
};

enum {
	TYPE_NONE,
	TYPE_LINE,
	TYPE_ARC,
	TYPE_CIRCLE,
	TYPE_MTEXT,
	TYPE_POINT,
	TYPE_POLYLINE,
	TYPE_VERTEX,
	TYPE_ELLIPSE,
	TYPE_LAST
};

typedef struct{
	int line[MAX_LINES];
	int closed;
	int inside;
	int visited;
	int use;
	int climb;
	int offset;
	int force;
	int overcut;
	int pocket;
	int helix;
	int laser;
	int tabs;
	int order;
	int tool_num;
	int tool_speed;
	int roughfine;
	double roughoff;
	double tool_dia;
	double depth;
	double min_x;
	double min_y;
	double max_x;
	double max_y;
	char layer[256];
} _OBJECT;

typedef struct{
	int used;
	int type;
	char layer[256];
	char block[256];
	double x1;
	double y1;
	double x2;
	double y2;
	double cx;
	double cy;
	double opt;
	int in_object;
	int blockdata;
	int istab;
	int marked;
} _LINE;

extern char LayerNames[MAX_OBJECTS][256];
extern _OBJECT *myOBJECTS;
extern _LINE *myLINES;
extern int mtext_n;
extern char dxf_options[256][256];
extern char dxf_typename[TYPE_LAST][16];
extern int line_last;

void dxf_read (char *file);
size_t trimline (char *out, size_t len, const char *str);
void add_line (int type, char *layer, double x1, double y1, double x2, double y2, double opt, double cx, double cy);

