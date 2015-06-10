//#define LUA_COMPAT_ALL 1
#define LUA_COMPAT_MODULE 1
#define _GNU_SOURCE 1
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <dxf.h>
#include <setup.h>
#include <assert.h>

#include "os-hacks.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <linux/limits.h> // for PATH_MAX
#endif

extern char *output_buffer;
extern char output_extension[128];
extern char output_info[1024];
extern char output_error[1024];

int lua_stat = 0;

lua_State *L;
void append_gcode_new (char *text);
#define A(i)	luaL_checknumber(L,i)

static int Lacosh(lua_State *L) {
	lua_pushnumber(L,acosh(A(1)));
	return 1;
}

static int Lasinh(lua_State *L) {
	lua_pushnumber(L,asinh(A(1)));
	return 1;
}

static int Latanh(lua_State *L) {
	lua_pushnumber(L,atanh(A(1)));
	return 1;
}

static int Lcbrt(lua_State *L) {
	lua_pushnumber(L,cbrt(A(1)));
	return 1;
}

static int Lcopysign(lua_State *L) {
	lua_pushnumber(L,copysign(A(1),A(2)));
	return 1;
}

static int Lerf(lua_State *L) {
	lua_pushnumber(L,erf(A(1)));
	return 1;
}

static int Lerfc(lua_State *L) {
	lua_pushnumber(L,erfc(A(1)));
	return 1;
}

static int Lexp2(lua_State *L) {
	lua_pushnumber(L,exp2(A(1)));
	return 1;
}

static int Lexpm1(lua_State *L) {
	lua_pushnumber(L,expm1(A(1)));
	return 1;
}

static int Lfdim(lua_State *L) {
	lua_pushnumber(L,fdim(A(1),A(2)));
	return 1;
}

static int Lfma(lua_State *L) {
	lua_pushnumber(L,fma(A(1),A(2),A(3)));
	return 1;
}

static int Lfmax(lua_State *L) {
	int i,n = lua_gettop(L);
	lua_Number m=A(1);
	for (i=2; i<=n; i++) m=fmax(m,A(i));
	lua_pushnumber(L,m);
	return 1;
}

static int Lfmin(lua_State *L) {
	int i,n=lua_gettop(L);
	lua_Number m=A(1);
	for (i=2; i<=n; i++) m=fmin(m,A(i));
	lua_pushnumber(L,m);
	return 1;
}

static int Lfpclassify(lua_State *L) {
	switch (fpclassify(A(1))) {
		case FP_INFINITE:	lua_pushliteral(L,"inf");	break;
		case FP_NAN:		lua_pushliteral(L,"nan");	break;
		case FP_NORMAL:	lua_pushliteral(L,"normal");	break;
		case FP_SUBNORMAL:	lua_pushliteral(L,"subnormal");	break;
		case FP_ZERO:		lua_pushliteral(L,"zero");	break;
	}
	return 1;
}

static int Lhypot(lua_State *L) {
	lua_pushnumber(L,hypot(A(1),A(2)));
	return 1;
}

static int Lisfinite(lua_State *L) {
	lua_pushboolean(L,isfinite(A(1)));
	return 1;
}

static int Lisinf(lua_State *L) {
	lua_pushboolean(L,isinf(A(1)));
	return 1;
}

static int Lisnan(lua_State *L) {
	lua_pushboolean(L,isnan(A(1)));
	return 1;
}

static int Lisnormal(lua_State *L) {
	lua_pushboolean(L,isnormal(A(1)));
	return 1;
}

static int Llgamma(lua_State *L) {
	lua_pushnumber(L,lgamma(A(1)));
	return 1;
}

static int Llog1p(lua_State *L) {
	lua_pushnumber(L,log1p(A(1)));
	return 1;
}

static int Llog2(lua_State *L) {
	lua_pushnumber(L,log2(A(1)));
	return 1;
}

static int Llogb(lua_State *L) {
	lua_pushnumber(L,logb(A(1)));
	return 1;
}

static int Lnearbyint(lua_State *L) {
	lua_pushnumber(L,nearbyint(A(1)));
	return 1;
}

static int Lnextafter(lua_State *L) {
	lua_pushnumber(L,nextafter(A(1),A(2)));
	return 1;
}

static int Lremainder(lua_State *L) {
	lua_pushnumber(L,remainder(A(1),A(2)));
	return 1;
}

static int Lrint(lua_State *L) {
	lua_pushnumber(L,rint(A(1)));
	return 1;
}

static int Lround(lua_State *L) {
	lua_pushnumber(L,round(A(1)));
	return 1;
}

static int Lscalbn(lua_State *L) {
	lua_pushnumber(L,scalbn(A(1),(int)A(2)));
	return 1;
}

static int Lsignbit(lua_State *L) {
	lua_pushboolean(L,signbit(A(1)));
	return 1;
}

static int Ltgamma(lua_State *L) {
	lua_pushnumber(L,tgamma(A(1)));
	return 1;
}

static int Ltrunc(lua_State *L) {
	lua_pushnumber(L,trunc(A(1)));
	return 1;
}

static const luaL_Reg R[] = {
	{ "acosh",	Lacosh },
	{ "asinh",	Lasinh },
	{ "atanh",	Latanh },
	{ "cbrt",	Lcbrt },
	{ "copysign",	Lcopysign },
	{ "erfc",	Lerfc },
	{ "erf",	Lerf },
	{ "exp2",	Lexp2 },
	{ "expm1",	Lexpm1 },
	{ "fdim",	Lfdim },
	{ "fma",	Lfma },
	{ "fmax",	Lfmax },
	{ "fmin",	Lfmin },
	{ "fpclassify",	Lfpclassify },
	{ "gamma",	Ltgamma },
	{ "hypot",	Lhypot },
	{ "isfinite",	Lisfinite },
	{ "isinf",	Lisinf },
	{ "isnan",	Lisnan },
	{ "isnormal",	Lisnormal },
	{ "lgamma",	Llgamma },
	{ "log1p",	Llog1p },
	{ "log2",	Llog2 },
	{ "logb",	Llogb },
	{ "nearbyint",	Lnearbyint },
	{ "nextafter",	Lnextafter },
	{ "remainder",	Lremainder },
	{ "rint",	Lrint },
	{ "round",	Lround },
	{ "scalbn",	Lscalbn },
	{ "signbit",	Lsignbit },
	{ "trunc",	Ltrunc },
	{ NULL,		NULL }
};


LUALIB_API int luaopen_mathx(lua_State *L) {
#if LUA_VERSION_NUM >= 502
        // TODO fix this for LUA 5.2+
#else
        luaL_register(L,LUA_MATHLIBNAME,R);
	lua_pushnumber(L,INFINITY);
	lua_setfield(L,-2,"infinity");
	lua_pushnumber(L,NAN);
	lua_setfield(L,-2,"nan");
#endif
	return 1;
}


void postcam_var_push_double (char *name, double value) {
	if (lua_stat == 0) {
		return;
	}
	lua_pushnumber(L, value);
	lua_setglobal(L, name);
}

void postcam_var_push_int (char *name, int value) {
	if (lua_stat == 0) {
		return;
	}
	lua_pushinteger(L, value);
	lua_setglobal(L, name);
}

void postcam_var_push_string (char *name, char *value) {
	if (lua_stat == 0) {
		return;
	}
	lua_pushstring(L, value);
	lua_setglobal(L, name);
}

void postcam_call_function (char *name) {
	if (lua_stat == 0) {
		return;
	}
	if (strcmp(name, "OnInit") == 0) {
		postcam_call_function("reset_modal");
	}
	lua_getglobal(L, name);
	int type = lua_type(L, -1);
	if (type != LUA_TNIL && type == LUA_TFUNCTION) {
		if (lua_pcall(L, 0, 0, 0)) {
			sprintf(output_error, "FATAL ERROR (call: %s):\n %s\n\n", name, lua_tostring(L, -1));
			fprintf(stderr, "%s", output_error);
			lua_stat = 0;
		}
	}
}

void postcam_comment (char *value) {
	lua_pushstring(L, value);
	lua_setglobal(L, "commentText");
	postcam_call_function("OnComment");
}

static int append_output (lua_State *L) {
	const char *text = lua_tostring(L, -1);
	if (output_buffer == NULL) {
		int len = strlen(text) + 1;
		output_buffer = (char *)malloc(len);
		output_buffer[0] = 0;
	} else {
		int len = strlen(output_buffer) + strlen(text) + 1;
		output_buffer = (char *)realloc((void *)output_buffer, len);
	}
	strcat(output_buffer, text);
	return 1;
}

static int set_extension (lua_State *L) {
	const char *str = lua_tostring(L, -1);
	strcpy(output_extension, (char *)str);
	return 1;
}

static int set_output_info (lua_State *L) {
	const char *str = lua_tostring(L, -1);
	sprintf(output_info, "\n%s", (char *)str);
	return 1;
}

void postcam_init_lua (const char* path, char *plugin) {
	L = luaL_newstate();

        luaL_openlibs(L);

	luaopen_mathx(L);
	lua_register(L, "output_info", set_output_info);  
	lua_register(L, "append_output", append_output);  
	lua_register(L, "set_extension", set_extension);  
	postcam_var_push_double("endX", 0.0);
	postcam_var_push_double("endY", 0.0);
	postcam_var_push_double("endZ", 0.0);
	postcam_var_push_double("currentX", 0.0);
	postcam_var_push_double("currentY", 0.0);
	postcam_var_push_double("currentZ", 0.0);
	postcam_var_push_double("toolOffset", 0.0);
	postcam_var_push_double("scale", 1.0);
	postcam_var_push_double("arcCentreX", 0.0);
	postcam_var_push_double("arcCentreY", 0.0);
	postcam_var_push_double("arcAngle", 0.0);
	postcam_var_push_int("plungeRate", 200);
	postcam_var_push_int("feedRate", 2000);
	postcam_var_push_int("spindleSpeed", 10000);
	postcam_var_push_int("metric", 1);
	postcam_var_push_int("tool", 1);
	postcam_var_push_int("lastinst", 0);
	postcam_var_push_string("operationName", "");
	postcam_var_push_string("commentText", "");
	postcam_var_push_string("toolName", "");
	postcam_var_push_string("partName", "");

	char filename[PATH_MAX];
	snprintf(filename, PATH_MAX, "%s%s%s", path, DIR_SEP, "postprocessor.lua");
	if (luaL_loadfile(L, filename)) {
		sprintf(output_error, "FATAL ERROR(postprocessor.lua / 1):\n %s\n\n", lua_tostring(L, -1));
		fprintf(stderr, "%s", output_error);
		lua_stat = 0;
	}
	if (lua_pcall(L, 0, 0, 0)) {
		sprintf(output_error, "FATAL ERROR(postprocessor.lua / 2):\n %s\n\n", lua_tostring(L, -1));
		fprintf(stderr, "%s", output_error);
		lua_stat = 0;
	}

	lua_stat = 1;

	char tmp_str[PATH_MAX];
	snprintf(tmp_str, PATH_MAX, "%sposts%s%s.scpost", path, DIR_SEP, plugin);
	postcam_var_push_string("postfile", tmp_str);
	postcam_call_function("load_post");

	if (lua_stat == 0) {
		return;
	}

	lua_getglobal(L, "OnAbout");
	lua_getglobal(L, "event");
	int error = 0;
	int type = lua_type(L, -1);
	if (type != LUA_TNIL && (error = lua_pcall(L, 1, 1, 0))) {
		int ret_stack = lua_gettop(L);
		sprintf(output_error, "FATAL ERROR(lua-init):\nstack: %i\nerror: %i\n %s\n\n", ret_stack, error, lua_tostring(L, -1));
		fprintf(stderr, "%s", output_error);
		lua_stat = 0;
	} else {
		lua_stat = 1;
		output_error[0] = 0;
	}
	lua_pop(L,1);
}


void postcam_exit_lua (void) {
	lua_close(L);
}

